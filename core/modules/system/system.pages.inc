<?php
/**
 * @file
 * System module non-administrative page callbacks.
 */

/**
 * Page callback to output a token tree as an empty page.
 */
function system_token_output() {
  $options = isset($_GET['options']) ? backdrop_json_decode($_GET['options']) : array();

  // Check the token against the serialized options to prevent random access to
  // the token browser page.
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'token-tree:' . serialize($options))) {
    return MENU_ACCESS_DENIED;
  }

  $tree = '<div>' . theme('token_tree', $options) . '</div>';

  if (backdrop_is_ajax()) {
    // Return as a renderable so the Backdrop dialog system can use the title.
    $dialog_options = array(
      'dialogClass' => 'token-dialog',
      'modal' => FALSE,
      'draggable' => TRUE,
      'resizeable' => TRUE,
      'autoResize' => FALSE,
      'width' => '50%',
      'height' => '600',
    );
    $commands = array(ajax_command_open_dialog('#token-dialog', t('Available tokens'), $tree, $dialog_options));
    $return = array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
    return $return;
  }
  else {
    return $tree;
  }
}

/**
 * 
 */
function system_autocomplete_token($token_type) {
  $args = func_get_args();
  array_shift($args);
  $string = trim(implode('/', $args));
  $string = substr($string, strrpos($string, '['));

  $token_type = $token_type['type'];
  $matches = array();

  if (!backdrop_strlen($string)) {
    $matches["[{$token_type}:"] = 0;
  }
  else {
    $depth = max(1, substr_count($string, ':'));
    $tree = token_build_tree($token_type, array('flat' => TRUE, 'depth' => $depth));
    foreach (array_keys($tree) as $token) {
      if (strpos($token, $string) === 0) {
        $matches[$token] = levenshtein($token, $string);
        if (isset($tree[$token]['children'])) {
          $token = rtrim($token, ':]') . ':';
          $matches[$token] = levenshtein($token, $string);
        }
      }
    }
  }

  asort($matches);
  $matches = backdrop_map_assoc(array_keys($matches));
  backdrop_json_output($matches);
}

/**
 * 
 */
function token_autocomplete() {
  $args = func_get_args();
  $string = implode('/', $args);

  $token_info = token_info();

  preg_match_all('/\[([^\s\]:]*):?([^\s\]]*)?\]?/', $string, $matches);
  $types = $matches[1];
  $tokens = $matches[2];

  foreach ($types as $index => $type) {
    if (!empty($tokens[$index]) || isset($token_info['types'][$type])) {
      system_autocomplete_token($type, $tokens[$index]);
    }
    else {
      token_autocomplete_type($type);
    }
  }
}

/**
 * 
 */
function token_autocomplete_type($string = '') {
  $token_info = token_info();
  $types = $token_info['types'];
  $matches = array();

  foreach ($types as $type => $info) {
    if (!$string || strpos($type, $string) === 0) {
      $type_key = "[{$type}:";
      $matches[$type_key] = levenshtein($type, $string);
    }
  }

  if ($string) {
    asort($matches);
  }
  else {
    ksort($matches);
  }

  $matches = backdrop_map_assoc(array_keys($matches));
  backdrop_json_output($matches);
}
