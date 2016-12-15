<?php
/**
 * @file
 * API for the Backdrop menu router system.
 */

/**
 * Process tabs_data and generate $tabs with $level.
 *
 * @param $tabs_data
 *   A multilevel associative array containing tabs information grouped by tab_parent.
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding tabs.
 * @param $depth
 *   The depth of reqursion.
 * @param $map
 *   A path argument array, used to replace integer values in $data; an integer
 *   value N in $data will be replaced by value $map[N]. Typically, the $map
 *   array is generated from a call to the arg() function.
 * @param $root_path
 *   The full current $route_item['path'].
 *
 * @return
 *   A flat array containing tabs information grouped by $level.
 */
function _menu_local_tasks_generate_tabs($tabs_data, $path, $depth = 0, $map, $root_path) {
  static $tabs;
  $items = $tabs_data[$path];
  $tabs_current = array();
  foreach($items as $key => $item) {
    _menu_translate($item, $map, TRUE);

    if ($item['access']) {
      $tab = array(
        '#theme' => 'menu_local_task',
      );
      if (mb_strpos($root_path, $item['path']) === 0) {
        $tab['#active'] = TRUE;
      }
      if (($item['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
        $item['href'] = $item['tab_parent_href'];
        $item['path'] = $item['tab_parent'];
        if ( $item['path'] == $root_path) {
          $tab['#active'] = TRUE;
        }
      }
      $tab['#link'] = $item;

      if (isset($tab['#active'])) {
        if (($item['type'] & MENU_LINKS_TO_PARENT) != MENU_LINKS_TO_PARENT) {
          if (isset($tabs_data[$item['path']])) {
            _menu_local_tasks_generate_tabs($tabs_data, $item['path'], ($depth + 1), $map, $root_path);
          }
        }
      }
      $tabs_current[] = $tab;
    }
  }
  $tabs[$depth]['count'] = count($tabs_current);
  $tabs[$depth]['output'] = $tabs_current;
  return $tabs;
}

/**
 * Builds the router files strucure based on the data from hook_menu().
 */
function _menu_router_files_build($callbacks) {
  $menu = array();
  $masks = array();

  $callbacks_path = array();
  foreach ($callbacks as $path => $item) {
    $fit = 0;
    $move = FALSE;    
    if (!isset($item['_load_functions'])) {
      $item['_load_functions'] = array();
      $item['to_arg_functions'] = array();
    }
    $parts = explode('/', $path);
    $item['number_parts'] = count($parts);
    $slashes = $item['number_parts'] - 1;

    foreach ($parts as $k => $part) {
      $match = FALSE;
      // Look for wildcards in the form allowed to be used in PHP functions,
      // because we are using these to construct the load function names.
      if (preg_match('/^%(|' . BACKDROP_PHP_FUNCTION_PATTERN . ')$/', $part, $matches)) {
        if (empty($matches[1])) {
          $match = TRUE;
          $item['_load_functions'][$k] = NULL;
        }
        else {
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
      else {
        $fit |=  1 << ($slashes - $k);
      }
    }
    if ($fit) {
      $move = TRUE;
    }
    else {
      // If there is no %, it fits maximally.
      $fit = (1 << $item['number_parts']) - 1;
    }
    $masks[$fit] = 1;
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

  $dest = _menu_router_root_path();
  $root = $dest . "." . backdrop_random_key();
  if (!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }

  foreach ($menu_router_tree as $item) {
    _save_menu_router_item($item, $root);
  }
  $tmp_dir_name = $dest . "." .  time();
  if (is_dir($dest)) {
    rename($dest, $tmp_dir_name);
  }
  rename($root, $dest);
  if (is_dir($tmp_dir_name)) {
    // Use background clean up to speed up process.
    popen('rm -rf ' . $tmp_dir_name . ' &', 'r');
  }

// Sort the masks so they are in order of descending fit.
  $masks = array_keys($masks);
  rsort($masks);
  state_set('menu_masks', $masks);
  return array($menu, $masks);
}

/**
 * Save menu router item to json file.
 *
 * @param $item
 *   An array representing a menu router item.
 *
 */
function _save_menu_router_item($item, $root) {

  if (isset($item['_item_'])) {
    $dir = $root . '/' . $item['_item_']['path'];
    if (!is_dir($dir)) {
      file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
      // @todo Set backdrop_chmod on parent dir, if path has multiple level.
    }

    $filename = $dir . '/item.json';
    $item_json = backdrop_json_encode($item['_item_'], TRUE);
//    file_unmanaged_save_data($item_json, $filename, FILE_EXISTS_REPLACE);
    file_put_contents($filename, $item_json);
    backdrop_chmod($filename);

    _save_menu_router_tabs_actions($item, $root);
    _save_menu_router_context($item, $root);
  }
  foreach($item as $name => $content){

    if ($name != '_item_' ) {
      if (isset($content['_item_']['type']) && isset($item['_item_']['path'])) {
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
  if (!is_dir($dir)) {
    file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
    // @todo Set backdrop_chmod on parent dir, if path has multiple level.
  }

  $filename = $dir . '/context.json';

  $content = array();
  if (file_exists($filename)) {
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  if ( isset($item['_item_'])) {
    if ($item['_item_']['context'] == MENU_CONTEXT_NONE) {
      return;
    }

    if ($item['_item_']['context'] == MENU_CONTEXT_PAGE) {
      return;
    }

    $content[$item['_item_']['path']] = $item['_item_'];
    $content_json = backdrop_json_encode($content, TRUE);
    //file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
    file_put_contents($filename, $content_json);
    backdrop_chmod($filename);

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

  if ( isset($item['_item_'])) {

    if ($item['_item_']['context'] == MENU_CONTEXT_INLINE) {
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
  if (!is_dir($dir)) {
    file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
    // @todo Set backdrop_chmod on parent dir, if path has multiple level.
  }

  $filename = $dir . '/actions.json';
  $content = array();
  if (file_exists($filename)) {
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  $content[$item['_item_']['path']] = $item['_item_'];
  $content_json = backdrop_json_encode($content, TRUE);
  //file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
  file_put_contents($filename, $content_json);
  backdrop_chmod($filename);

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
  if (!is_dir($dir)) {
    file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
    // @todo Set backdrop_chmod on parent dir, if path has multiple level.
  }

  $filename = $dir . '/tabs.json';
  $content = array();
  if (file_exists($filename)) {
    $content = backdrop_json_decode(file_get_contents($filename));
  }

  $content[$item['_item_']['tab_parent']][$item['_item_']['path']] = $item['_item_'];
  $content_json = backdrop_json_encode($content, TRUE);
  //file_unmanaged_save_data($content_json, $filename, FILE_EXISTS_REPLACE);
  file_put_contents($filename, $content_json);
  backdrop_chmod($filename);

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
  if (count($parts) > 0) {
    if (!isset($menu_router_tree[$part])) {
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
function _prepare_menu_router_item(&$router_structure, $path, $item) {
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

  if (!isset($item['tab_parent'])) {
    $item['tab_parent'] = '';
  }
  if (!isset($item['tab_root'])) {
    $item['tab_root'] = $path;
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


/**
 * Gets a router item tabs.
 *
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding
 *   node/% item and return that. Defaults to the current path.
 *
 * @return array
 *   The router item tabs array or empty array. A
 *   router item tabs is an associative multilevel array corresponding to router
 *   items path.
 */
function _get_router_item_tabs($path) {
  static $cached_items;

  if (isset($cached_items[$path])) {
    return $cached_items[$path];
  }

  $root = _menu_router_root_path();
  $filename = $root . '/' . $path . '/tabs.json';
  if (is_file($filename)) {
    $content = file_get_contents($filename);
    if ($content_json = backdrop_json_decode($content)) {
      foreach($content_json as $tab_parent => $items){
        usort($content_json[$tab_parent], "_menu_sort");
      }
      $cached_items[$path] = $content_json;
      return $content_json;
    }
  }

  return array();
}

/**
 * Gets a router item actions.
 *
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding
 *   node/% item and return that. Defaults to the current path.
 *
 * @return array
 *   The router item actions array or empty array. A
 *   router item actions is an associative array corresponding to router
 *   items path.
 */
function _get_router_item_actions($path) {
  static $cached_items;

  if (isset($cached_items[$path])) {
    return $cached_items[$path];
  }

  $root = _menu_router_root_path();
  $filename = $root . '/' . $path . '/actions.json';
  if (is_file($filename)) {
    $content = file_get_contents($filename);
    if ($item = backdrop_json_decode($content)) {
      $cached_items[$path] = $item;
      return $item;
    }
  }
  return array();
}

/**
 * Gets a router item context.
 *
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding
 *   node/% item and return that. Defaults to the current path.
 *
 * @return array
 *   The router item context array or empty array. A
 *   router item context is an associative array corresponding to router
 *   items path.
 */
function _get_router_item_context($path) {
  static $cached_items;

  if (isset($cached_items[$path])) {
    return $cached_items[$path];
  }

  $root = _menu_router_root_path();
  $filename = $root . '/' . $path . '/context.json';
  if (is_file($filename)) {
    $content = file_get_contents($filename);
    if ($item = backdrop_json_decode($content)) {
      usort($item, "_menu_sort");
      $cached_items[$path] = $item;
      return $item;
    }
  }
  return array();
}

/**
 * Remove router json file
 *
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding
 *   node/% item and return that. Defaults to the current path.
 *
 * @return bool|FALSE
 *   If router item exists - return TRUE.
 */
function _remove_router_item($path) {
  $root = _menu_router_root_path();
  $filename = $root . '/' . $path . '/item.json';
  if (file_exists($filename)) {
    backdrop_unlink($filename);
    return TRUE;
  }

  return FALSE;
}



/**
 * Get menu_router root directory path.
 *
 * @return string
 */
function _menu_router_root_path() {
  static $root;
    if (!$root) {
    // @todo Use config settings to get root path.
    $root = "files/menu_router";
    if ($test_prefix = backdrop_valid_test_ua()) {
      // See BackdropWebTestBase::setUp().
      $root = conf_path() . '/files/simpletest/' . substr($test_prefix, 10) . '/menu_router';
    }
    if (isset($GLOBALS['backdrop_test_info'])) {
      $test_prefix = $GLOBALS['backdrop_test_info']['test_run_id'];
      // See BackdropWebTestBase::setUp().
      $root = conf_path() . '/files/simpletest/' . substr($test_prefix, 10) . '/menu_router';
    }
  }
  return $root;
}

/**
 * Get empty router item array.
 *
 * @return array
 *   A router item is an associative array. The value corresponding to
 *   the key 'map' holds the loaded objects.
 */
function _get_empty_router_item() {
  return array(
    'type' => NULL,
    'page_callback' => NULL,
    'load_functions' => NULL,
    'to_arg_functions' => NULL,
    'access_callback' => NULL,
    'access_arguments' => NULL,
    'page_callback' => NULL,
    'page_arguments' => NULL,
    'delivery_callback' => NULL,
    'tab_parent' => NULL,
    'tab_root' => NULL,
    'title' => NULL,
    'title_callback' => NULL,
    'title_arguments' => NULL,
    'theme_callback' => NULL,
    'theme_arguments' => NULL,
    'type' => NULL,
    'description' => NULL,
  );
}

/**
 * Read a router item from json file.
 *
 * @param $path
 *   The path; for example, 'node/%'. The function will find NOT the corresponding
 *   node/% item to node/5.
 *
 * @return array|FALSE
 *   The router item or, if an error occurs, FALSE. A router item is
 *   an associative array. The value corresponding to the key 'map' holds the
 *   loaded objects.
 */
function _get_exact_router_item($path) {
  $root = _menu_router_root_path();
  $filename = $root . '/' . $path . '/item.json';
  if (is_file($filename)) {
    $content = file_get_contents($filename);
    if ($item = backdrop_json_decode($content)) {
      return $item;
    }
  }
  return FALSE;
}

/**
 * Search and read a router item from json file.
 *
 * @param $path
 *   The path; for example, 'node/5'. The function will find the corresponding
 *   node/% item and return that. Defaults to the current path.
 *
 * @return array|FALSE
 *   The router item or, if an error occurs, FALSE. A router item is
 *   an associative array. The value corresponding to the key 'map' holds the
 *   loaded objects.
 */
function _get_router_item($path) {
  static $cached_items;
  static $cached_router_items;

  if (isset($cached_items[$path])) {
    return $cached_items[$path];
  }
  $parts = explode("/", $path);
  $ancestors = menu_get_ancestors($parts);
  $root = _menu_router_root_path();
    
  $found = FALSE;
  foreach($ancestors as $ancestor) {
    $filename = $root . '/' . $ancestor . '/item.json';
    if (is_file($filename)) {
      $found = TRUE;
      break;
    }
  }
  // Do not double read the same menu roiter item.
  // Different $path could lead to the same router_item;
  if ($found) {
    if (isset($cached_router_items[$filename])) {
      return $cached_router_items[$filename];
    }
    $content = file_get_contents($filename);
    if ($item = backdrop_json_decode($content)) {
      $cached_items[$path] = $item;
      $cached_router_items[$filename] = $item;
      return $item;
    }
  }
  return FALSE;
}

/**
 * Comparator routine for use in sorting menu items.
 */
function _menu_sort($a, $b) {
  if ($a['weight'] > $b['weight'] ) {
    return 1;
  }
  if ($a['weight'] < $b['weight'] ) {
    return -1;
  }
  if ($a['title'] > $b['title'] ) {
    return 1;
  }
  if ($a['title'] < $b['title'] ) {
    return -1;
  }
  return 0;
}

