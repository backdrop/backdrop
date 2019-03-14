<?php

/**
 * @file
 * Theming functions for code_sign.
 */

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
 *
 * A signing module may overide this with a more complex item including links
 * to the signing certificate or displaying the identity of the signer.
 */
function theme_code_sign_result($vars) {
  $result_class = empty($vars['result']) ? 'code-sign-result-bad' : 'code-sign-result-good';
  $result_text = t('@signer:@profile - @result', array(
    '@signer' => $vars['signer'],
    '@profile' => $vars['profile'],
    '@result' => empty($vars['result']) ? t('Pass') : t('Fail'),
  ));

  return '<div class="code-sign-result ' . $result_class .'>' . $result_text . '</div>';
}
