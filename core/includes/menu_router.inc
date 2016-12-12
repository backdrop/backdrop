<?php
/**
 * @file
 * API for the Backdrop menu router system.
 */

/**
 * Builds the router files strucure based on the data from hook_menu().
 */
function _menu_router_files_build($callbacks) {
  $menu = array();
  $masks = array();
    
  $callbacks_path = array();
  foreach ($callbacks as $path => $item) {
    if(!isset($item['_load_functions'])){
      $item['_load_functions'] = array();
      $item['to_arg_functions'] = array();
    }
    $parts = explode('/', $path);
    $item['number_parts'] = count($parts);
    foreach ($parts as $k => $part) {
      $match = FALSE;
      // Look for wildcards in the form allowed to be used in PHP functions,
      // because we are using these to construct the load function names.
      if (preg_match('/^%(|' . BACKDROP_PHP_FUNCTION_PATTERN . ')$/', $part, $matches)) {
        if (empty($matches[1])) {
          $match = TRUE;
          $item['_load_functions'][$k] = NULL;
        } else {
          if (function_exists($matches[1] . '_to_arg')) {
            $item['to_arg_functions'][$k] = $matches[1] . '_to_arg';
            $item['_load_functions'][$k] = NULL;
            $match = TRUE;
          }
          if (function_exists($matches[1] . '_load')) {
            $function = $matches[1] . '_load';
            // Create an array of arguments that will be passed to the _load
            // function when this menu path is checked, if 'load arguments'
            // exists.
            $item['_load_functions'][$k] = isset($item['load arguments']) ? array($function => $item['load arguments']) : $function;
            $match = TRUE;
          }
        }
      }
      if ($match) {
        $parts[$k] = '%';
      }
    }
    $path = implode("/", $parts);
    $callbacks_path[$path] = $item;
  }
  // Sort items based on path. This way we are going to process all items
  // from parent to children.
  ksort($callbacks_path);
  $router_structure = array();
  foreach ($callbacks_path as $path => $item) {
    _prepare_menu_router_item($router_structure, $path, $item);
  }
  
  $menu = $router_structure;

  $menu_router_tree = array();
  foreach ($router_structure as $path => $item) {
    $parts = explode('/', $path);
    _generate_menu_router_tree($menu_router_tree, $parts, $item);
  }

  // @todo Use config settings to get root path.
  $dest = "files/menu_router";
  // backdrop_valid_test_ua() is not returning info inside testrun.
  if (isset($GLOBALS['backdrop_test_info'])) {
    $test_prefix = $GLOBALS['backdrop_test_info']['test_run_id'];
    // See BackdropWebTestBase::setUp().
    $dest = conf_path() . '/files/simpletest/' . substr($test_prefix, 10) . '/menu_router';
  }

  $root = $dest . "." . backdrop_random_key();
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }

  foreach ($menu_router_tree as $item) {
    _save_menu_router_item($item, $root);
  }
  $tmp_dir_name = $dest . "." .  time();
  if(is_dir($dest)){
    rename($dest, $tmp_dir_name);
  }
  rename($root, $dest);
  if(is_dir($tmp_dir_name)){
    file_unmanaged_delete_recursive($tmp_dir_name);
  }
  return array($menu, $masks);
}

/**
 * Save menu router item to json file.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_item($item, $root){

  if(isset($item['_item_'])) {
    $dir = $root . '/' . $item['_item_']['path'];
    if(!is_dir($dir)) {
      file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
      // @todo Set backdrop_chmod on parent dir, if path has multiple level.
    }

    $filename = $dir . '/item.json';
    $item_json = backdrop_json_encode($item['_item_'], TRUE);
    file_unmanaged_save_data($item_json, $filename, FILE_EXISTS_REPLACE);

    _save_menu_router_tabs_actions($item, $root);
    _save_menu_router_context($item, $root);
  }
  foreach($item as $name => $content){

    if($name != '_item_' ) {
      if(isset($content['_item_']['type']) && isset($item['_item_']['path'])) {
        // The default task href is looking to parent URL.
        if (($content['_item_']['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
          $item[$name]['_item_']['href'] = $item['_item_']['path'];
        }
      }
      _save_menu_router_item($item[$name], $root);
    }
  }
}
/**
 * Determinate if item is context and save it.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_context($item, $root) {
  $dir = $root . '/' . $item['_item_']['tab_root'];

  $filename = $dir . '/context.json';

  $content = array();
  if(file_exists($filename)){
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  if( isset($item['_item_'])){
    if($item['_item_']['context'] == MENU_CONTEXT_NONE) {
      return;
    }

    if($item['_item_']['context'] == MENU_CONTEXT_PAGE) {
      return;
    }

    $content[$item['_item_']['path']] = $item['_item_'];
    $content_json = backdrop_json_encode($content, TRUE);
    file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
  }
}

/**
 * Determinate if item is tab or action and save it.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_tabs_actions($item, $root) {

  if( isset($item['_item_'])){

    if($item['_item_']['context'] == MENU_CONTEXT_INLINE) {
      return;
    }

    // Local tasks can be normal items too, so bitmask with
    // MENU_IS_LOCAL_TASK before checking.
    if (!($item['_item_']['type'] & MENU_IS_LOCAL_TASK)) {
      // This item is not a tab, skip it.
      return;
    }
    if (($item['_item_']['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
      _save_menu_router_tab_item($item, $root);
    } else {
      if (($item['_item_']['type'] & MENU_IS_LOCAL_ACTION) == MENU_IS_LOCAL_ACTION) {
        // The item is an action, display it as such.
        _save_menu_router_action_item($item, $root);
      } else {
        _save_menu_router_tab_item($item, $root);
      }
    }
  }
}

/**
 * Save action item.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_action_item($item, $root) {
  $dir = $root . '/' . $item['_item_']['tab_root'];

  $filename = $dir . '/actions.json';
  $content = array();
  if(file_exists($filename)){
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  $content[$item['_item_']['path']] = $item['_item_'];
  $content_json = backdrop_json_encode($content, TRUE);
  file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
}

/**
 * Save tab item.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_tab_item($item, $root) {
  $dir = $root . '/' . $item['_item_']['tab_root'];

  $filename = $dir . '/tabs.json';
  $content = array();
  if(file_exists($filename)){
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  $content[$item['_item_']['tab_parent']][$item['_item_']['path']] = $item['_item_'];
  $content_json = backdrop_json_encode($content, TRUE);
  file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
}

/**
 * Generate menu router tree.
 *
 * @param $menu_router_tree
 *   A multilevel (tree like) array of menu router items.
 * @param $parts
 *   The menu item path parts.
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _generate_menu_router_tree(&$menu_router_tree, $parts, $item) {
  $part = array_shift($parts);
  if(count($parts) > 0) {
    if(!isset($menu_router_tree[$part])) {
      $menu_router_tree[$part] = array();
    }
    _generate_menu_router_tree($menu_router_tree[$part], $parts, $item);
  } else {
    $menu_router_tree[$part]['_item_'] = array(
      'path' => $item['path'],
      'load_functions' => $item['load_functions'],
      'to_arg_functions' => $item['to_arg_functions'],
      'access_callback' => $item['access callback'],
      'access_arguments' => $item['access arguments'],
      'page_callback' => $item['page callback'],
      'page_arguments' => $item['page arguments'],
      'delivery_callback' => $item['delivery callback'],
      'context' => $item['context'],
      'title' => $item['title'],
      'title_callback' => $item['title callback'],
      'title_arguments' => ($item['title arguments'] ? $item['title arguments'] : ''),
      'theme_callback' => $item['theme callback'],
      'theme_arguments' => $item['theme arguments'],
      'type' => $item['type'],
      'description' => $item['description'],
      'position' => $item['position'],
      'weight' => $item['weight'],
      'include_file' => $item['include file'],
      'tab_parent' => $item['tab_parent'],
      'tab_root' => $item['tab_root'],
      'number_parts' => $item['number_parts'],
    );
  }
}

/**
 * Process each item and set all nesessary settings.
 *
 * @param $router_structure
 *   An array of processed items.
 * @param $path
 *   The router_item path.
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _prepare_menu_router_item(&$router_structure, $path, $item){
  $parts = explode('/', $path);

  $item += array(
    'title' => '',
    'weight' => 0,
    'type' => MENU_NORMAL_ITEM,
    'module' => '',
  );
  $item += array(
    '_visible' => (bool) ($item['type'] & MENU_VISIBLE_IN_BREADCRUMB),
    '_tab' => (bool) ($item['type'] & MENU_IS_LOCAL_TASK),
  );
  if (!isset($item['context'])) {
    $item['context'] = MENU_CONTEXT_PAGE;
  }

  if (!$item['_tab']) {
    // Non-tab items.
    $item['tab_parent'] = '';
    $item['tab_root'] = $path;
  }
  $parent = array();

  for ($i = count($parts) - 1; $i; $i--) {
    $parent_path = implode('/', array_slice($parts, 0, $i));
    if (isset($router_structure[$parent_path])) {

      $parent = &$router_structure[$parent_path];
      // If we have no menu name, try to inherit it from parent items.
      if (!isset($item['menu_name'])) {
        // If the parent item of this item does not define a menu name (and no
        // previous iteration assigned one already), try to find the menu name
        // of the parent item in the currently stored menu links.
        if (!isset($parent['menu_name'])) {
          $menu_name = db_query("SELECT menu_name FROM {menu_links} WHERE router_path = :router_path AND module = 'system'", array(':router_path' => $parent_path))->fetchField();
          if ($menu_name) {
            $parent['menu_name'] = $menu_name;
          }
        }
        // If the parent item defines a menu name, inherit it.
        if (!empty($parent['menu_name'])) {
          $item['menu_name'] = $parent['menu_name'];
        }
      }
      if (!isset($item['tab_parent'])) {
        // Parent stores the parent of the path.
        $item['tab_parent'] = $parent_path;
      }
      if (!isset($item['tab_root']) && !$parent['_tab']) {
        $item['tab_root'] = $parent_path;
      }

      // If an access callback is not found for a default local task we use
      // the callback from the parent, since we expect them to be identical.
      // In all other cases, the access parameters must be specified.
      if (($item['type'] == MENU_DEFAULT_LOCAL_TASK) && !isset($item['access callback']) && isset($parent['access callback'])) {
        $item['access callback'] = $parent['access callback'];
        if (!isset($item['access arguments']) && isset($parent['access arguments'])) {
          $item['access arguments'] = $parent['access arguments'];
        }
      }

      // Same for page callbacks.
      if (!isset($item['page callback']) && isset($parent['page callback'])) {
        $item['page callback'] = $parent['page callback'];
        if (!isset($item['page arguments']) && isset($parent['page arguments'])) {
          $item['page arguments'] = $parent['page arguments'];
        }
        if (!isset($item['file path']) && isset($parent['file path'])) {
          $item['file path'] = $parent['file path'];
        }
        if (!isset($item['file']) && isset($parent['file'])) {
          $item['file'] = $parent['file'];
          if (empty($item['file path']) && isset($item['module']) && isset($parent['module']) && $item['module'] != $parent['module']) {
            $item['file path'] = backdrop_get_path('module', $parent['module']);
          }
        }
      }

      // Same for delivery callbacks.
      if (!isset($item['delivery callback']) && isset($parent['delivery callback'])) {
        $item['delivery callback'] = $parent['delivery callback'];
      }

      // Same for theme callbacks.
      if (!isset($item['theme callback']) && isset($parent['theme callback'])) {
        $item['theme callback'] = $parent['theme callback'];
        if (!isset($item['theme arguments']) && isset($parent['theme arguments'])) {
          $item['theme arguments'] = $parent['theme arguments'];
        }
      }

      // Same for load arguments: if a loader doesn't have any explict
      // arguments, try to find arguments in the parent.
      if (!isset($item['load arguments'])) {
        foreach ($item['_load_functions'] as $k => $function) {
          // This loader doesn't have any explict arguments...
          if (!is_array($function)) {
            // ... check the parent for a loader at the same position
            // using the same function name and defining arguments...
            if (isset($parent['_load_functions'][$k]) && is_array($parent['_load_functions'][$k]) && key($parent['_load_functions'][$k]) === $function) {
              // ... and inherit the arguments on the child.
              $item['_load_functions'][$k] = $parent['_load_functions'][$k];
            }
          }
        }
      }
    }
  }
  if (!isset($item['access callback']) && isset($item['access arguments'])) {
    // Default callback.
    $item['access callback'] = 'user_access';
  }
  if (!isset($item['access callback']) || empty($item['page callback'])) {
    $item['access callback'] = 0;
  }
  if (is_bool($item['access callback'])) {
    $item['access callback'] = intval($item['access callback']);
  }

  $item['load_functions'] = $item['_load_functions'];
  unset($item['_load_functions']);

  $item += array(
    'access arguments' => array(),
    'access callback' => '',
    'page arguments' => array(),
    'page callback' => '',
    'delivery callback' => '',
    'title arguments' => array(),
    'title callback' => 't',
    'theme arguments' => array(),
    'theme callback' => '',
    'description' => '',
    'position' => '',
    'context' => 0,
    'path' => $path,
    'file' => '',
    'file path' => '',
    'include file' => '',
  );

  // Calculate out the file to be included for each callback, if any.
  if ($item['file']) {
    $file_path = $item['file path'] ? $item['file path'] : backdrop_get_path('module', $item['module']);
    $item['include file'] = $file_path . '/' . $item['file'];
  }
  $router_structure[$path] = $item;
}
