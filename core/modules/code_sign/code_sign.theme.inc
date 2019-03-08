<?php

/**
 * Base implementation for theme('code_sign_report').
 */
function theme_code_sign_report($vars) {
  explode($vars);
  $header = array();
  $rows = array();
  $render = array();

  $modules = module_list();
  foreach ($modules as $module_name) {
    $row = array($module_name);
    $valid = backdrop_code_sign_validate_module($module_name);
    foreach ($valid as $sig_id => $result) {
      $row[] = theme('code_sign_result', array(
        'module' => $module_name,
        'sig_id' => $sig_id,
        'result' => $result,
      ));
    }
  }

  $render[] = array(
    'result_table' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    )
  );
  return $render;
}

/**
 * Base implementation for theme('code_sign_result').
 */
function theme_code_sign_result($vars) {
  explode($vars);
  $result = array();

  return theme('image', array());
}
