<?php

/**
 * @file
 * The PHP page that serves all page requests on a Backdrop installation.
 *
 * The routines here dispatch control to the appropriate handler, which then
 * prints the appropriate page.
 *
 * All Backdrop code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt files in the "core" directory.
 */

/**
 * Root directory of Backdrop installation.
 */
define('BACKDROP_ROOT', dirname(dirname($_SERVER['SCRIPT_FILENAME'])));
// Change the directory to the Backdrop root.
chdir(BACKDROP_ROOT);

require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);

$callbacks = array();
foreach (module_implements('menu') as $module) {
  $router_items = call_user_func($module . '_menu');
  if (isset($router_items) && is_array($router_items)) {
    foreach (array_keys($router_items) as $path) {
      $router_items[$path]['module'] = $module;
    }
    $callbacks = array_merge($callbacks, $router_items);
  }
}

backdrop_alter('menu', $callbacks);
// Update Path and load
$callbacks_path = array();
foreach ($callbacks as $path => $item) {
  $parts = explode('/', $path);
  if(!isset($item['_load_functions'])){
    $item['_load_functions'] = array();
    $item['to_arg_functions'] = array();
  }
  
  $item['number_parts'] = count($parts);
  
  foreach ($parts as $k => $part) {
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
          $path = str_replace('%' . $matches[1], '%', $path);  
          // Create an array of arguments that will be passed to the _load
          // function when this menu path is checked, if 'load arguments'
          // exists.
          $item['_load_functions'][$k] = isset($item['load arguments']) ? array($function => $item['load arguments']) : $function;
        }
      }
    }
  }
  
  //$path = implode("/", $parts);
  $callbacks_path[$path] = $item;
}
//print_r($callbacks);
$router_structure = array();
ksort($callbacks_path);
//print_r($callbacks_path);
foreach ($callbacks_path as $path => $item) {
  _set_router_data($router_structure, $path, $item);
}

//print_r($router_structure);
$menu_router_tree = array();
foreach($router_structure as $path => $item){
  $parts = explode('/', $path);
  _generate_menu_tree($menu_router_tree, $parts, $item);
}
//print_r($menu_router_tree);
foreach($menu_router_tree as $item){
  _store_item($item);
}

function _generate_menu_tree(&$menu_router_tree, $parts, $item) {
  $part = array_shift($parts);
  if(count($parts) > 0) {
    if(!isset($menu_router_tree[$part])) {
      $menu_router_tree[$part] = array();
    }
    _generate_menu_tree($menu_router_tree[$part], $parts, $item);
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

function _store_item($item){
  
  $root = "files/menu_router";
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }
  
  if(isset($item['_item_'])) {
    echo "store: " . $item['_item_']['path'] . "\n";
    $dir = $root . '/' . $item['_item_']['path'];
    if(!is_dir($dir)) {
      file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
      // TODO: Set backdrop_chmod on parent dir, if path has multiple level.
    }
  
    $filename = $dir . '/item.json'; 
    $item_json = backdrop_json_encode($item['_item_'], TRUE);
    file_unmanaged_save_data($item_json, $filename, FILE_EXISTS_REPLACE);  
    
    if(!empty($children)) {
      $children_filename = $dir . '/children.json';
      $children_json = backdrop_json_encode($children, TRUE);
      file_unmanaged_save_data($children_json, $children_filename, FILE_EXISTS_REPLACE);
    }
    _set_tabs_actions($item);
    _store_context($item);
  }
  foreach($item as $name => $content){
  
    if($name != '_item_' ) {
      if(isset($content['_item_']['type']) && isset($item['_item_']['path'])) {
        // The default task href is looking to parent URL.
        if (($content['_item_']['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
          $item[$name]['_item_']['href'] = $item['_item_']['path'];
        }
      }
      _store_item($item[$name]);
    }
  }
}

function _store_tab_item($item) {
  $root = "files/menu_router";
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }
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

function _store_action_item($item) {
  $root = "files/menu_router";
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }
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

function _store_context($item) {
  $root = "files/menu_router";
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }
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

function _set_tabs_actions($item){
  
  if( isset($item['_item_'])){
//    $item = $subitem['_item_'];
    
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
//      $item['_item_']['href'] = $parent_item['_item_']['path'];
      // We need to change path as well in this case.
      // $item['path'] = $item['href'];
      _store_tab_item($item);
    } else {
      if (($item['_item_']['type'] & MENU_IS_LOCAL_ACTION) == MENU_IS_LOCAL_ACTION) {
        // The item is an action, display it as such.
        _store_action_item($item);
      } else {
        _store_tab_item($item);
      }
    }
  }

/*  if($tab_count > 0 ){
    $tabs_filename = $dir . '/tabs.json';
    $tabs_json = backdrop_json_encode($tabs_current, TRUE);
    file_unmanaged_save_data($tabs_json, $tabs_filename, FILE_EXISTS_REPLACE);
  }
  if($action_count > 0 ){
    $action_filename = $dir . '/actions.json';
    $action_json = backdrop_json_encode($actions_current, TRUE);
    file_unmanaged_save_data($action_json, $action_filename, FILE_EXISTS_REPLACE);
  }*/
}

function _set_router_data(&$router_structure, $path, $item){
  $parts = explode('/', $path);
  
 /* if(!isset($item['_load_functions'])){
    $item['_load_functions'] = array();
    $item['to_arg_functions'] = array();
  }
  $match = FALSE;
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
          $path = str_replace('%' . $matches[1], '%', $path);  
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
  
  $path = implode("/", $parts);*/
    
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
      if(!isset($parent['_tab'])){
        print_r($parent);
        print_r($item);        
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
/*      
  // Same for include callbacks.
  if (!isset($item['include file']) && isset($parent['include_file'])) {
    $item['include file'] = $parent['include_file'];
  }*/
  
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
  
  /*if (($item['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
    // We need to change path as well in this case.
    $item['path'] = $item['href'];
  }        */
  $router_structure[$path] = $item;
  /*
  $router_structure[$part]['_item_'] = array(
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
  );*/
  
}

/*$config = config('system.router');
$config->set('_config_static', TRUE);
$config->set('router', $router_structure);
$config->save();
*/

print_r($router_structure);
