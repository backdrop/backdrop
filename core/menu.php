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
define('BACKDROP_ROOT', getcwd());

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
//print_r($callbacks);
$router_structure = array();
ksort($callbacks);
print_r($callbacks);
foreach ($callbacks as $path => $item) {
  $parts = explode('/', $path);
  $arg_num = 0;
  _set_router_data($router_structure, $parts, $item, $arg_num, $path);
}

foreach($router_structure as $item){
  _store_item($item);
}

function _store_item($item){
  $root = "files/menu_router";
  if(!is_dir($root)) {
    file_prepare_directory($root, FILE_CREATE_DIRECTORY);
  }
  
  if(isset($item['_item_'])){
    $dir = $root . '/' . $item['_item_']['path'];
    if(!is_dir($dir)) {
      file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
      // TODO: Set backdrop_chmod on parent dir, if path has multiple level.
    }
  }
  
  foreach($item as $name => $content){
    if($name == '_item_') {
      $filename = $dir . '/item.json'; 
      $item_json = backdrop_json_encode($content, TRUE);
      file_unmanaged_save_data($item_json, $filename, FILE_EXISTS_REPLACE);  
      $children_filename = $dir . '/children.json';
      $children = $item;
      unset($children['_item_']);

      // Do not store children.json if there is none.
      if(!empty($children)) {
        $children_json = backdrop_json_encode($children, TRUE);
        file_unmanaged_save_data($children_json, $children_filename, FILE_EXISTS_REPLACE);
      }
    } else {
      _store_item($item[$name]);
    }
  }
  
  if(isset($item['_item_'])){
    _set_tabs_actions($item, $dir);
  }
}

function _set_tabs_actions($parent_item, $dir){
  
  $action_count = 0;
  $tab_count = 0;
  $actions_current = array();
  $tabs_current = array();
  foreach($parent_item as $name => $subitem){
    if($name != '_item_' && isset($subitem['_item_'])){
      $item = $subitem['_item_'];
      if($item['context'] == MENU_CONTEXT_INLINE) {
        continue;
      }
      // Local tasks can be normal items too, so bitmask with
      // MENU_IS_LOCAL_TASK before checking.
      if (!($item['type'] & MENU_IS_LOCAL_TASK)) {
        // This item is not a tab, skip it.
        continue;
      }
      if (($item['type'] & MENU_LINKS_TO_PARENT) == MENU_LINKS_TO_PARENT) {
        $item['href'] = $parent_item['_item_']['path'];
        if ($item['href'] != $_GET['q']) {
            $item['localized_options']['attributes']['class'][] = 'active';
          }
        $tabs_current[] = array(
          '#theme' => 'menu_local_task',
          '#link' => $item,
          '#active' => TRUE,
        );
        $tab_count++;
      } else {
        if (($item['type'] & MENU_IS_LOCAL_ACTION) == MENU_IS_LOCAL_ACTION) {
          // The item is an action, display it as such.
          $actions_current[] = array(
            '#theme' => 'menu_local_action',
            '#link' => $item,
          );
          $action_count++;
        } else {
          // Otherwise, it's a normal tab.
          $tabs_current[] = array(
            '#theme' => 'menu_local_task',
            '#link' => $item,
          );
          $tab_count++;
        }
      }
    }
  }
  if($tab_count > 0 ){
    $tabs_filename = $dir . '/tabs.json';
    $tabs_json = backdrop_json_encode($tabs_current, TRUE);
    file_unmanaged_save_data($tabs_json, $tabs_filename, FILE_EXISTS_REPLACE);
  }
  if($action_count > 0 ){
    $action_filename = $dir . '/actions.json';
    $action_json = backdrop_json_encode($actions_current, TRUE);
    file_unmanaged_save_data($action_json, $action_filename, FILE_EXISTS_REPLACE);
  }
}

function _set_router_data(&$router_structure, $parts, $item, $arg_num, $path){
  $part = array_shift($parts);
  
  if(!isset($item['_load_functions'])){
    $item['_load_functions'] = array();
    $item['to_arg_functions'] = array();
  }
  $match = FALSE;
  // Look for wildcards in the form allowed to be used in PHP functions,
  // because we are using these to construct the load function names.
  if (preg_match('/^%(|' . BACKDROP_PHP_FUNCTION_PATTERN . ')$/', $part, $matches)) {
    if (empty($matches[1])) {
      $match = TRUE;
    } else {
      if (function_exists($matches[1] . '_to_arg')) {
        $item['to_arg_functions'][$arg_num] = $matches[1] . '_to_arg';
        $match = TRUE;
      }
      if (function_exists($matches[1] . '_load')) {
        $function = $matches[1] . '_load';
        $path = str_replace('%' . $matches[1], '%', $path);  
        // Create an array of arguments that will be passed to the _load
        // function when this menu path is checked, if 'load arguments'
        // exists.
        $item['_load_functions'][$arg_num] = isset($item['load arguments']) ? array($function => $item['load arguments']) : $function;
        $match = TRUE;
      }
    }
  }
  if ($match) {
    $part = '%';
  }
  
  if(count($parts) > 0) {
    if(!isset($router_structure[$part])) {
      $router_structure[$part] = array();
    }
    $arg_num++;
    _set_router_data($router_structure[$part], $parts, $item, $arg_num, $path);
  } else {
    
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
    $parent = array();
    
    if(isset($router_structure['_item_'])){
      $parent = $router_structure['_item_'];
    }

    // If an access callback is not found for a default local task we use
    // the callback from the parent, since we expect them to be identical.
    // In all other cases, the access parameters must be specified.
    if (($item['type'] == MENU_DEFAULT_LOCAL_TASK) && !isset($item['access callback']) && isset($parent['access_callback'])) {
      $item['access callback'] = $parent['access_callback'];
      if (!isset($item['access arguments']) && isset($parent['access_arguments'])) {
        $item['access arguments'] = $parent['access_arguments'];
      }
    }
    // Same for page callbacks.
    if (!isset($item['page callback']) && isset($parent['page_callback'])) {
      $item['page callback'] = $parent['page_callback'];
      if (!isset($item['page arguments']) && isset($parent['page_arguments'])) {
        $item['page arguments'] = $parent['page_arguments'];
      }
      if (!isset($item['file path']) && isset($parent['file_path'])) {
        $item['file path'] = $parent['file_path'];
      }
      if (!isset($item['file']) && isset($parent['file'])) {
        $item['file'] = $parent['file'];
        if (empty($item['file path']) && isset($item['module']) && isset($parent['module']) && $item['module'] != $parent['module']) {
          $item['file path'] = backdrop_get_path('module', $parent['module']);
        }
      }
    }
    // Same for delivery callbacks.
    if (!isset($item['delivery callback']) && isset($parent['delivery_callback'])) {
      $item['delivery callback'] = $parent['delivery_callback'];
    }
    // Same for include callbacks.
    if (!isset($item['include file']) && isset($parent['include_file'])) {
      $item['include file'] = $parent['include_file'];
    }
    // Same for theme callbacks.
    if (!isset($item['theme callback']) && isset($parent['theme_callback'])) {
      $item['theme callback'] = $parent['theme_callback'];
      if (!isset($item['theme arguments']) && isset($parent['theme_arguments'])) {
        $item['theme arguments'] = $parent['theme_arguments'];
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
    $item['path']= $path;
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
    );
  
}

/*$config = config('system.router');
$config->set('_config_static', TRUE);
$config->set('router', $router_structure);
$config->save();
*/

print_r($router_structure);
