<?php

// Menu callback that shows menu item.
function devel_menu_item() {
  $item = menu_get_item($_GET['path']);
  return kdevel_print_object($item);
}

/**
 * Returns a list of all currently defined user functions in the current
 * request lifecycle, with links their documentation.
 */
function devel_function_reference() {
  $functions = get_defined_functions();
  $version = devel_get_core_version(BACKDROP_VERSION);
  $ufunctions = $functions['user'];
  sort($ufunctions);
  $api = config_get('devel.settings', 'api_url');
  foreach ($ufunctions as $function) {
    $links[] = l($function, "http://$api/api/function/$function/$version");
  }
  return theme('item_list', array('items' => $links));
}

// A menu callback. Called by the AJAX link in query log.
function devel_querylog_explain($request_id, $qid) {
  if (!is_numeric($request_id)) {
    return MENU_ACCESS_DENIED;
  }

  $path = "temporary://devel_querylog/$request_id.txt";
  $path = file_stream_wrapper_uri_normalize($path);
  $queries = json_decode(file_get_contents($path));
  $query = $queries[$qid];
  $result = db_query('EXPLAIN ' . $query->query, (array)$query->args)->fetchAllAssoc('table');
  $i = 1;
  foreach ($result as $row) {
    $row = (array)$row;
    if ($i == 1) {
      $header = array_keys($row);
    }
    $rows[] = array_values($row);
    $i++;
  }
  // Print and return nothing thus avoiding page wrapper.
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  print $output;
  $GLOBALS['devel_shutdown'] = FALSE;
}

// A menu callback. Called by the AJAX link in query log.
function devel_querylog_arguments($request_id, $qid) {
  if (!is_numeric($request_id)) {
    return MENU_ACCESS_DENIED;
  }

  $path = "temporary://devel_querylog/$request_id.txt";
  $path = file_stream_wrapper_uri_normalize($path);
  $queries = json_decode(file_get_contents($path));
  $query = $queries[$qid];
  $conn = Database::getConnection();
  $quoted = array();
  foreach ((array)$query->args as $key => $val) {
    $quoted[$key] = $conn->quote($val);
  }
  $output = strtr($query->query, $quoted);

  // print and return nothing thus avoiding page wrapper.
  print $output;
  $GLOBALS['devel_shutdown'] = FALSE;
}

/**
 * Display a dropdown of installed modules with the option to reinstall them.
 */
function devel_reinstall($form, &$form_state) {
  $output = '';
  $modules = module_list();
  sort($modules);
  $options = backdrop_map_assoc($modules);
  $form['list'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('Uninstall and then install the selected modules. <code>hook_uninstall()</code> and <code>hook_install()</code> will be executed and the schema version number will be set to the most recent update number. You may have to manually clear out any existing tables first if the module doesn\'t implement <code>hook_uninstall()</code>.'),
  );
  $form['submit'] = array(
    '#value' => t('Reinstall'),
    '#type' => 'submit',
  );
  return $form;
}

/**
 * Process reinstall menu form submissions.
 */
function devel_reinstall_submit($form, &$form_state) {
  // require_once './includes/install.inc';
  $modules = array_filter($form_state['values']['list']);
  module_disable($modules, FALSE);
  backdrop_uninstall_modules($modules, FALSE);
  module_enable($modules, FALSE);
  backdrop_set_message(t('Uninstalled and installed: %names.', array('%names' => implode(', ', $modules))));
}

// Menu callback.
function devel_theme_registry() {
  backdrop_theme_initialize();
  $hooks = theme_get_registry();
  ksort($hooks);
  return kprint_r($hooks, TRUE);
}

// Menu callback. $entity_type argument not currently used in the UI.
function devel_entity_info_page($entity_type = NULL) {
  $info = entity_get_info($entity_type);
  ksort($info);
  return kprint_r($info, TRUE);
}

// Menu callback.
function devel_field_info_page() {
  $info = field_info_fields();
  $output = kprint_r($info, TRUE, t('Fields'));
  $info = field_info_instances();
  $output .= kprint_r($info, TRUE, t('Instances'));
  $info = field_info_bundles();
  $output .= kprint_r($info, TRUE, t('Bundles'));
  return $output;
}

/**
 * Menu callback: display the session.
 */
function devel_session() {
  global $user;
  $output = kprint_r($_SESSION, TRUE);
  $headers = array(t('Session name'), t('Session ID'));
  $output .= theme('table', array('headers' => $headers, 'rows' => array(array(session_name(), session_id()))));
  return $output;
}

/**
 * Menu callback; prints the loaded structure of the current node/user.
 */
function devel_load_object($type, $object, $name = NULL) {
  $name = isset($name) ? $name : $type;
  return kdevel_print_object($object, '$'. $name .'->');
}

/**
 * Menu callback; prints the render structure of the current object (currently node or user).
 */
function devel_render_object($type, $object, $name = NULL) {
  $name = isset($name) ? $name : $type;
  $function = $type . '_view';
  $build = $function($object);
  return kdevel_print_object($build, '$'. $name .'->');
}

function devel_elements_page() {
  return kdevel_print_object(module_invoke_all('element_info'));
}

function devel_phpinfo() {
  print phpinfo();
  backdrop_exit();
}
