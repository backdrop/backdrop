<?php

/**
 * @file
 * Token integration for the Pathauto module.
 */

/**
 * Implements hook_token_info().
 */
function path_token_info() {
  $info = array();

  $info['tokens']['array']['join-path'] = array(
    'name' => t('Joined path'),
    'description' => t('The array values each cleaned by Pathauto and then joined with the slash into a string that resembles an URL.'),
  );

  return $info;
}

/**
 * Implements hook_tokens().
 */
function path_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  if ($type == 'array' && !empty($data['array'])) {
    $array = $data['array'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'join-path':
          module_load_include('inc', 'path');
          $values = array();
          foreach ($array as $key => $value) {
            $value = path_cleanstring((string) $array[$key], $options);
            $values[] = $value;
          }
          $replacements[$original] = implode('/', $values);
          break;
      }
    }
  }

  return $replacements;
}
