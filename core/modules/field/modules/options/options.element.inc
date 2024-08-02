<?php
/**
 * @file
 * All logic for options form elements.
 */

/**
 * Logic function for form_options_expand(). Do not call directly.
 *
 * @see form_options_expand()
 */
function _form_options_expand($element) {
  $element['#options'] = isset($element['#options']) ? $element['#options'] : array();
  $element['#multiple'] = isset($element['#multiple']) ? $element['#multiple'] : FALSE;

  $element['#tree'] = TRUE;
  $element['#theme'] = 'options';

  $path = backdrop_get_path('module', 'options');
  $element['#attached']['library'] = array(
      array('options', 'options'),
  );

  // Add the key type toggle checkbox.
  if (!isset($element['custom_keys']) && $element['#key_type'] != 'custom' && !empty($element['#key_type_toggle'])) {
    $element['custom_keys'] = array(
      '#title' => is_string($element['#key_type_toggle']) ? $element['#key_type_toggle'] : t('Customize keys'),
      '#type' => 'checkbox',
      '#default_value' => $element['#key_type_toggled'],
      '#attributes' => array('class' => array('key-type-toggle')),
      '#description' => t('Customizing the keys will allow you to save one value internally while showing a different option to the user.'),
    );
  }

  // Add the multiple value toggle checkbox.
  if (!isset($element['multiple']) && !empty($element['#multiple_toggle'])) {
    $element['multiple'] = array(
      '#title' => is_string($element['#multiple_toggle']) ? $element['#multiple_toggle'] : t('Allow multiple values'),
      '#type' => 'checkbox',
      '#default_value' => !empty($element['#multiple']),
      '#attributes' => array('class' => array('multiple-toggle')),
      '#description' => t('Multiple values will let users select multiple items in this list.'),
    );
  }
  // If the element had a custom interface for toggling whether or not multiple
  // values are accepted, make sure that form_type_options_value() knows to use
  // it.
  if (isset($element['multiple']) && empty($element['#multiple_toggle'])) {
    $element['#multiple_toggle'] = TRUE;
  }

  // Add the main textarea for adding options.
  if (!isset($element['options'])) {
    $element['options_field'] = array(
      '#type' => 'textarea',
      '#resizable' => 'vertical',
      '#cols' => 60,
      '#rows' => 5,
      '#required' => isset($element['#required']) ? $element['#required'] : FALSE,
      '#description' => t('List options one option per line.'),
      '#attributes' => $element['#options_readonly'] ? array('readonly' => 'readonly') : array(),
      '#wysiwyg' => FALSE, // Prevent CKeditor from trying to hijack.
    );

    // If validation fails, reload the user's text even if it's not valid.
    if (isset($element['#value']['options_text'])) {
      $element['options_field']['#value'] = $element['#value']['options_text'];
    }
    // Most of the time, we'll be converting the options array into the text.
    else {
      $element['options_field']['#value'] = isset($element['#options']) ? form_options_to_text($element['#options'], $element['#key_type']) : '';
    }


    if ($element['#key_type'] == 'mixed' || $element['#key_type'] == 'numeric' || $element['#key_type'] == 'custom') {
      $element['options_field']['#description'] .= ' ' . t('Key-label pairs may be specified by separating each option with pipes, such as <em>key|label</em>.');
    }
    elseif ($element['#key_type_toggle']) {
      $element['options_field']['#description'] .= ' ' . t('If the %toggle field is checked, key-label pairs may be specified by separating each option with pipes, such as <em>key|label</em>.', array('%toggle' => $element['custom_keys']['#title']));
    }
    if ($element['#key_type'] == 'numeric') {
      $element['options_field']['#description'] .= ' ' . t('This field requires all specified keys to be integers.');
      $element['options_field']['#description'] .= ' ' . t('The label is optional: if a line contains a single number, it will be used as key and label.');
    }
    $element['options_field']['#description'] .= ' ' . t('Customizing the keys will allow you to save one value internally while showing a different option to the user.');
  }

  // Add the field for storing default values.
  if ($element['#default_value_allowed'] && !isset($element['default_value_field'])) {
    $element['default_value_field'] = array(
      '#title' => t('Default value'),
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 1024,
      '#value' => isset($element['#default_value']) ? ($element['#multiple'] ? implode(', ', (array) $element['#default_value']) : $element['#default_value']) : '',
      '#description' => t('Specify the keys that should be selected by default.'),
    );
    if ($element['#multiple']) {
      $element['default_value_field']['#description'] .= ' ' . t('Multiple default values may be specified by separating keys with commas.');
    }
  }

  // Add the field for storing a default value pattern.
  if ($element['#default_value_pattern']) {
    $element['default_value_pattern'] = array(
      '#type' => 'hidden',
      '#value' => $element['#default_value_pattern'],
      '#attributes' => array('class' => array('default-value-pattern')),
    );
  }

  // Pass allowed HTML tags to JS.
  $element['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('optionsElement' => array('allowedtags' => '<div class="description">' . t('Allowed HTML tags in labels: @tags', array('@tags' => _field_filter_xss_display_allowed_tags())) . '</div>')),
  );

  // Remove properties that will confuse the FAPI.
  unset($element['#options']);

  return $element;
}

/**
 * Logic function for form_options_validate(). Do not call directly.
 *
 * @see form_options_validate()
 */
function _form_options_validate($element, &$form_state) {
  // Even though we already have the converted options in #value['options'], run
  // the conversion again to check for duplicates in the user-defined list.
  $duplicates = array();
  $options = form_options_from_text($element['#value']['options_text'], $element['#key_type'], empty($element['#optgroups']), $duplicates);

  // Check if a key is used multiple times.
  if (count($duplicates) == 1) {
    form_error($element, t('The key %key has been used multiple times. Each key must be unique to display properly.', array('%key' => reset($duplicates))));
  }
  elseif (!empty($duplicates)) {
    array_walk($duplicates, 'check_plain');
    $duplicate_list = theme('item_list', array('items' => $duplicates));
    form_error($element, t('The following keys have been used multiple times. Each key must be unique to display properly.') . $duplicate_list);
  }

  // Add the list of duplicates to the page so that we can highlight the fields.
  if (!empty($duplicates)) {
    backdrop_add_js(array('optionsElement' => array('errors' => backdrop_map_assoc($duplicates))), 'setting');
  }

  // Check if no options are specified.
  if (empty($options) && $element['#required']) {
    form_error($element, t('At least one option must be specified.'));
  }

  // Check for numeric keys if needed.
  if ($element['#key_type'] == 'numeric') {
    foreach ($options as $key => $value) {
      if (!is_int($key)) {
        form_error($element, t('The keys for the %title field must be integers.', array('%title' => $element['#title'])));
        break;
      }
    }
  }

  // Check that the limit of options has not been exceeded.
  if (!empty($element['#limit'])) {
    $count = 0;
    foreach ($options as $value) {
      if (is_array($value)) {
        $count += count($value);
      }
      else {
        $count++;
      }
    }
    if ($count > $element['#limit']) {
      form_error($element, t('The %title field supports a maximum of @count options. Please reduce the number of options.', array('%title' => $element['#title'], '@count' => $element['#limit'])));
    }
  }
}

/**
 * Logic function for form_type_options_value(). Do not call directly.
 *
 * @see form_type_options_value()
 */
function _form_type_options_value(&$element, $edit = FALSE) {
  if ($edit === FALSE) {
     return array(
       'options' => isset($element['#options']) ? $element['#options'] : array(),
       'default_value' => isset($element['#default_value']) ? $element['#default_value'] : '',
     );
  }
  else {
    // Convert text to an array of options.
    $duplicates = array();
    $options = form_options_from_text($edit['options_field'], $element['#key_type'], empty($element['#optgroups']), $duplicates);

    // Convert default value.
    if (isset($edit['default_value_field'])) {
      // If the element supports toggling whether or not it will accept
      // multiple values, use the value that was passed in via $edit (keeping
      // in mind that this value may not be set, if a checkbox was used to
      // configure it). Otherwise, use the current setting stored with the
      // element itself.
      $multiple = !empty($element['#multiple_toggle']) ? !empty($edit['multiple']) : !empty($element['#multiple']);
      if ($multiple) {
        $default_value = array();
        $default_items = explode(',', $edit['default_value_field']);
        foreach ($default_items as $key) {
          $key = trim($key);
          $value = _form_options_search($key, $options, $element['#default_value_pattern']);
          if (!is_null($value)) {
            $default_value[] = $value;
          }
        }
      }
      else {
        $default_value = _form_options_search(trim($edit['default_value_field']), $options, $element['#default_value_pattern']);
      }
    }
    else {
      $default_value = NULL;
    }

    $return = array(
      'options' => $options,
      'default_value' => $default_value,
      'options_text' => $edit['options_field'],
    );

    if (isset($edit['default_value_field'])) {
      $return['default_value_text'] = $edit['default_value_field'];
    }

    return $return;
  }
}

/**
 * Logic function for form_options_to_text(). Do not call directly.
 *
 * @see form_options_to_text()
 */
function _form_options_to_text($options, $key_type) {
  $output = '';
  $previous_key = false;

  foreach ($options as $key => $value) {
    // Convert groups.
    if (is_array($value)) {
      $output .= '<' . $key . '>' . "\n";
      foreach ($value as $subkey => $subvalue) {
        $output .= (($key_type == 'mixed' || $key_type == 'numeric' || $key_type == 'custom') ? $subkey . '|' : '') . $subvalue . "\n";
      }
      $previous_key = $key;
    }
    // Typical key|value pairs.
    else {
      // Exit out of any groups.
      if (isset($options[$previous_key]) && is_array($options[$previous_key])) {
        $output .= "<>\n";
      }
      // Skip empty rows.
      if ($options[$key] !== '') {
        if ($key_type == 'mixed' || $key_type == 'numeric' || $key_type == 'custom') {
          $output .= $key . '|' . $value . "\n";
        }
        else {
          $output .= $value . "\n";
        }
      }
      $previous_key = $key;
    }
  }

  return $output;
}

/**
 * Logic function for form_options_from_text(). Do not call directly.
 *
 * @see form_options_from_text()
 */
function _form_options_from_text($text, $key_type, $flat = FALSE, &$duplicates = array()) {
  $keys = array();
  $items = array();
  $rows = array_filter(explode("\n", trim($text)), 'strlen');
  $group = FALSE;
  foreach ($rows as $row) {
    $row = trim($row);
    $matches = array();

    // Check for a simple empty row.
    if (!strlen($row)) {
      continue;
    }
    // Check if this row is a group.
    elseif (!$flat && preg_match('/^\<((([^>|]*)\|)?([^>]*))\>$/', $row, $matches)) {
      if ($matches[1] === '') {
        $group = FALSE;
      }
      else {
        $group = $matches[4] ? $matches[4] : '';
        $keys[] = $group;
      }
    }
    // Check if this row is a key|value pair.
    elseif (($key_type == 'mixed' || $key_type == 'custom' || $key_type == 'numeric') && preg_match('/^([^|]+)\|(.*)$/', $row, $matches)) {
      $key = $matches[1];
      $value = $matches[2];
      $keys[] = $key;
      $items[] = array(
        'key' => $key,
        'value' => $value,
        'group' => $group,
      );
    }
    // Set this this row as a straight value.
    else {
      $items[] = array(
        'key' => NULL,
        'value' => $row,
        'group' => $group,
      );
    }
  }

  // Expand the list into a nested array, assign keys and check duplicates.
  $options = array();
  $new_key = 1;
  foreach ($items as $item) {
    if (!is_numeric($item['key'])) {
      continue;
    }
    $int_key = (int) $item['key'];
    if (is_int($int_key)) {
      $new_key = max($int_key, $new_key);
    }
  }
  foreach ($items as $item) {
    // Assign a key if needed.
    if ($key_type == 'none') {
      $item['key'] = $new_key++;
    }
    elseif (!isset($item['key'])) {
      while (in_array($new_key, $keys)) {
        $new_key++;
      }
      $keys[] = $new_key;
      $item['key'] = $new_key;
    }

    if ($item['group']) {
      if (isset($options[$item['group']][$item['key']])) {
        $duplicates[] = $item['key'];
      }
      $options[$item['group']][$item['key']] = $item['value'];
    }
    else {
      if (isset($options[$item['key']])) {
        $duplicates[] = $item['key'];
      }
      $options[$item['key']] = $item['value'];
    }
  }

  return $options;
}

/**
 * Recursive function for finding default value keys. Matches on keys or values.
 */
function _form_options_search($needle, $haystack, $include_pattern) {
  if (isset($haystack[$needle])) {
    return $needle;
  }
  elseif ($include_pattern && preg_match('/' . $include_pattern . '/', $needle)) {
    return $needle;
  }
  foreach ($haystack as $key => $value) {
    if (is_array($value)) {
      return _form_options_search($needle, $value, $include_pattern);
    }
    elseif ($value == $needle) {
      return $key;
    }
  }
}
