<?php
/**
 * @file
 * Token module integration.
 */

/**
 * Implements hook_token_info().
 */
function date_token_info() {
  // All date types can share the same date value type.
  $info['types']['date-field-value'] = array(
    'name' => t('Date field values'),
    'description' => t('Tokens related to date field values.'),
    'needs-data' => 'date-field-value',
    'field-value' => TRUE,
  );

  // Provide two tokens: 'date' (the date or start-date), and 'end-date'.
  $info['tokens']['date-field-value']['date'] = array(
    'name' => t('Date'),
    'description' => t('The date value.'),
    'type' => 'date',
  );
  $info['tokens']['date-field-value']['to-date'] = array(
    'name' => t('End Date'),
    'description' => t('The End date value.'),
    'type' => 'date',
  );

  return $info;
}

/**
 * Implements hook_token_info_alter().
 */
function date_token_info_alter(&$info) {
  // Attach date field tokens to their respective entity tokens.
  $field_info = field_info_fields();
  $fields = _field_token_info();

  foreach ($fields as $field_name => $field) {
    if (in_array($field_info[$field_name]['type'], array('date', 'datetime', 'datestamp'))) {
      foreach (array_keys($field['bundles']) as $token_type) {
        // If a token already exists for this field, then don't add it.
        if (isset($info['tokens'][$token_type][$field_name])) {
          continue;
        }
  
        // Ensure token type for this entity already exists. 
        if (!isset($info['types'][$token_type]) || !isset($info['tokens'][$token_type])) {
          continue;
        }
  
        $info['tokens'][$token_type][$field_name] = array(
          // Note that label and description have already been
          // sanitized by _field_token_info().
          'name' => $field['label'],
          'description' => $field['description'],
          'module' => 'date',
          'type' => 'date-field-value',
        );

        if ($field_info[$field_name]['cardinality'] != 1) {
          $info['tokens'][$token_type][$field_name]['dynamic'] = TRUE;
          $info['tokens'][$token_type][$field_name]['description'] .= ' ' . t('Replace the ? with the delta for multi-value fields.');
        }

        if (empty($field_info[$field_name]['settings']['todate'])) {
          $info['tokens'][$token_type][$field_name]['type'] = 'date';
        }
      }
    }
  }
}


/**
 * Implements hook_tokens().
 */
function date_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $language_code = isset($options['language']) ? $options['language']->langcode : NULL;

  // Process date fields attached to entities.
  if (!empty($data[$type]) && is_a($data[$type], 'Entity')) {
    $entity = $data[$type];
    $entity_type = $entity->entityType();
    $entity_info = entity_get_info($entity_type); 

    if (isset($entity_info['fieldable']) && $entity_info['fieldable']) {
      $fields = field_info_field_map();

      // Check to see which fields are supported.
      foreach ($fields as $field_name => $field) {
        if (in_array($field['type'], array('date', 'datetime', 'datestamp'))) {
          $date_field_tokens = token_find_with_prefix($tokens, $field_name);

          $field_items = field_get_items($entity_type, $entity, $field_name);
          if ($date_field_tokens && $field_items) {
            $replacements += token_generate('date-field', $date_field_tokens, array('date-field' => $field_items), $options);
          }
        }
      }
    }
  } 
  
  // Process the chain after the field name.
  elseif (($type == 'date-field') && !empty($data[$type])) {
    $field_items = $data[$type];
    foreach ($tokens as $token => $original) {
      $explode = explode(':', $token);

      if (is_numeric($explode[0]) && !empty($field_items[$explode[0]])) {
        $delta = $explode[0];
        $chained_token = token_find_with_prefix(array($token => $original), $delta);

        // If there is no arguments after delta, send date:medium as default.
        if (empty($chained_token)) {
          $chained_token = array('date:medium' => $original);
        } 
        $replacements += token_generate('date-field-value', $chained_token,array('date-field-value' => $field_items[$delta]), $options);

      } else {
        // If no delta is specified, send the first item of the field.
        $replacements += token_generate('date-field-value', array($token => $original), array('date-field-value' => $field_items[0]), $options);
      }
    }
  }
    
  // Process date field item. The token at this point is in the format 
  // "[date||end-date]:[format]", where format is 'long', 'medium', etc. 
  // or 'custom'.
  elseif (($type == 'date-field-value') && !empty($data['date-field-value'])) {
    $item = $data['date-field-value'];

    // Create tokens for the field's "Date" or "Start date".
    if (($date_tokens = token_find_with_prefix($tokens, 'date')) && !empty($item['value'])) {
      // Load the Start date and convert to a unix timestamp.
      $date = new BackdropDateTime($item['value'], $item['timezone_db'], date_type_format($item['date_type']));
      if (!empty($date) && $item['timezone_db'] != $item['timezone']) {
        date_timezone_set($date, timezone_open($item['timezone']));
      }
      $timestamp = !empty($date) ? date_format_date($date, 'custom', 'U') : '';
      // Generate the token replacements, using the date token type provided
      // by system.module.
      $replacements += token_generate('date', $date_tokens, array('date' => $timestamp), $options);
    }

    // Create tokens for the field's "End date".
    if (($date_tokens = token_find_with_prefix($tokens, 'end-date')) && !empty($item['value2'])) {
      // Load the to date and convert to a unix timestamp.
      $date = new BackdropDateTime($item['value2'], $item['timezone_db'], date_type_format($item['date_type']));
      if (!empty($date) && $item['timezone_db'] != $item['timezone']) {
        date_timezone_set($date, timezone_open($item['timezone']));
      }
      $timestamp = !empty($date) ? date_format_date($date, 'custom', 'U') : '';
      // Generate the token replacements, using the date token type provided
      // by system.module.
      $replacements += token_generate('date', $date_tokens, array('date' => $timestamp), $options);
    }

    // Handle token chains that do not contain 'date' or 'end-date'. 
    // Defaults to "starting date".
    $short_tokens = array();
    foreach ($tokens as $token => $original) {
      if (strpos($token, 'date') === FALSE && strpos($token, 'end-date') === FALSE && !empty($item['value'])) {
        $short_tokens = array_merge($short_tokens, array($token => $original));
      }
    }
    if (!empty($short_tokens)) {
      $date = new BackdropDateTime($item['value'], $item['timezone_db'], date_type_format($item['date_type']));
      if (!empty($date) && $item['timezone_db'] != $item['timezone']) {
        date_timezone_set($date, timezone_open($item['timezone']));
      }
      $timestamp = !empty($date) ? date_format_date($date, 'custom', 'U') : '';
      $replacements += token_generate('date', $short_tokens, array('date' => $timestamp), $options);
    }
  }

  return $replacements;
}
