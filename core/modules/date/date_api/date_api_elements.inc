<?php

/**
 * @file
 * Date API elements themes and validation.
 * This file is only included during the edit process to reduce memory usage.
 */

/**
 * Implements hook_element_info().
 *
 * Parameters for date form elements, designed to have sane defaults so any
 * or all can be omitted.
 *
 * Fill the element #default_value with a date in datetime format,
 * (YYYY-MM-DD HH:MM:SS), adjusted to the proper local timezone.
 *
 * NOTE - Converting a date stored in the database from UTC to the local zone
 * and converting it back to UTC before storing it is not handled by this
 * element and must be done in pre-form and post-form processing!!
 *
 * The date_select element will create a collection of form elements, with a
 * separate select or textfield for each date part. The whole collection will
 * get reformatted back to a date value of the requested type during validation.
 *
 * The date_text element will create a textfield that can contain a whole
 * date or any part of a date as text. The user input value will be re-formatted
 * back into a date value of the requested type during validation.
 *
 * The date_timezone element will create a drop-down selector to pick a
 * timezone name.
 *
 * The date_year_range element will create two textfields (for users with
 * JavaScript enabled they will appear as drop-down selectors with an option
 * for custom text entry) to pick a range of years that will be passed to form
 * submit handlers as a single string (e.g., -3:+3).
 *
 * #date_timezone
 *   The local timezone to be used to create this date.
 *
 * #date_format
 *   A format string that describes the format and order of date parts to
 *   display in the edit form for this element. This makes it possible
 *   to show date parts in a custom order, or to leave some of them out.
 *   Be sure to add 'A' or 'a' to get an am/pm selector. Defaults to the
 *   short site default format.
 *
 * #date_label_position
 *   Handling option for date part labels, like 'Year', 'Month', and 'Day',
 *   can be 'above' the date part, 'within' it, or 'none', default is 'above' .
 *   The 'within' option shows the label as the first option in a select list
 *   or the default value for an empty textfield, taking up less screen space.
 *
 * #date_increment
 *   Increment minutes and seconds by this amount, default is 1.
 *
 * #date_year_range
 *   The number of years to go back and forward in a year selector,
 *   default is -3:+3 (3 back and 3 forward).
 *
 * #date_text_parts
 *   Array of date parts that should use textfields instead of selects
 *   i.e. array('year') will format the year as a textfield and other
 *   date parts as drop-down selects.
 */
function _date_api_element_info() {
  $date_base = array(
    '#input' => TRUE, '#tree' => TRUE,
    '#date_timezone' => date_default_timezone(),
    '#date_flexible' => 0,
    '#date_format' => system_date_format_load('short'),
    '#date_text_parts' => array(),
    '#date_increment' => 1,
    '#date_year_range' => '-3:+3',
    '#date_label_position' => 'above',
  );
  if (module_exists('ctools')) {
    $date_base['#pre_render'] = array('ctools_dependent_pre_render');
  }
  $type['date_select'] = array_merge($date_base, array(
    '#process' => array('date_select_element_process'),
    '#theme_wrappers' => array('date_select'),
    '#value_callback' => 'date_select_element_value_callback',
  ));
  $type['date_text'] = array_merge($date_base, array(
    '#process' => array('date_text_element_process'),
    '#theme_wrappers' => array('date_text'),
    '#value_callback' => 'date_text_element_value_callback',
  ));
  $type['date_timezone'] = array(
    '#input' => TRUE, '#tree' => TRUE,
    '#process' => array('date_timezone_element_process'),
    '#theme_wrappers' => array('date_text'),
    '#value_callback' => 'date_timezone_element_value_callback',
  );
  $type['date_year_range'] = array(
    '#input' => TRUE,
    '#process' => array('date_year_range_element_process'),
    '#value_callback' => 'date_year_range_element_value_callback',
    '#element_validate' => array('date_year_range_validate'),
  );
  return $type;
}

/**
 * Create a date object from a datetime string value.
 */
function date_default_date($element) {
  $granularity = date_format_order($element['#date_format']);
  $default_value = $element['#default_value'];
  $format = DATE_FORMAT_DATETIME;

  // The text and popup widgets might return less than a full datetime string.
  if (strlen($element['#default_value']) < 19) {
    switch (strlen($element['#default_value'])) {
      case 16:
        $format = 'Y-m-d H:i';
        break;

      case 13:
        $format = 'Y-m-d H';
        break;

      case 10:
        $format = 'Y-m-d';
        break;

      case 7:
        $format = 'Y-m';
        break;

      case 4:
        $format = 'Y';
        break;
    }
  }
  $date = new DateObject($default_value, $element['#date_timezone'], $format);
  if (is_object($date)) {
    $date->limitGranularity($granularity);
    if ($date->validGranularity($granularity, $element['#date_flexible'])) {
      date_increment_round($date, $element['#date_increment']);
    }
    return $date;
  }
  return NULL;
}

/**
 * Process callback which creates a date_year_range form element.
 */
function date_year_range_element_process($element, &$form_state, $form) {
  // Year range is stored in the -3:+3 format, but collected as two separate
  // textfields.
  $element['years_back'] = array(
    '#type' => 'textfield',
    '#title' => t('Starting year'),
    '#default_value' => $element['#value']['years_back'],
    '#size' => 10,
    '#maxsize' => 10,
    '#attributes' => array('class' => array('select-list-with-custom-option', 'back')),
    '#description' => t('Enter a relative value (-9, +9) or an absolute year such as 2015.'),
  );
  $element['years_forward'] = array(
    '#type' => 'textfield',
    '#title' => t('Ending year'),
    '#default_value' => $element['#value']['years_forward'],
    '#size' => 10,
    '#maxsize' => 10,
    '#attributes' => array('class' => array('select-list-with-custom-option', 'forward')),
    '#description' => t('Enter a relative value (-9, +9) or an absolute year such as 2015.'),
  );

  $element['#tree'] = TRUE;
  $element['#attached']['js'][] = backdrop_get_path('module', 'date_api') . '/date_year_range.js';

  $context = array(
    'form' => $form,
  );
  backdrop_alter('date_year_range_process', $element, $form_state, $context);

  return $element;
}

/**
 * Element value callback for the date_year_range form element.
 */
function date_year_range_element_value_callback($element, $input = FALSE, &$form_state = array()) {
  // Convert the element's default value from a string to an array (to match
  // what we will get from the two textfields when the form is submitted).
  if ($input === FALSE) {
    list($years_back, $years_forward) = explode(':', $element['#default_value']);
    return array(
      'years_back' => $years_back,
      'years_forward' => $years_forward,
    );
  }
}

/**
 * Element validation function for the date_year_range form element.
 */
function date_year_range_validate(&$element, &$form_state) {
  // Recombine the two submitted form values into the -3:+3 format we will
  // validate and save.
  $year_range_submitted = backdrop_array_get_nested_value($form_state['values'], $element['#parents']);
  $year_range = $year_range_submitted['years_back'] . ':' . $year_range_submitted['years_forward'];
  backdrop_array_set_nested_value($form_state['values'], $element['#parents'], $year_range);
  if (!date_range_valid($year_range)) {
    form_error($element['years_back'], t('Starting year must be in the format -9, or an absolute year such as 1980.'));
    form_error($element['years_forward'], t('Ending year must be in the format +9, or an absolute year such as 2030.'));
  }
}

/**
 * Element value callback for date_timezone element.
 */
function date_timezone_element_value_callback($element, $input = FALSE, &$form_state = array()) {
  $return = '';
  if ($input !== FALSE) {
    $return = $input;
  }
  elseif (!empty($element['#default_value'])) {
    $return = array('timezone' => $element['#default_value']);
  }
  return $return;
}

/**
 * Creates a timezone form element.
 *
 * @param array $element
 *   The timezone form element.
 *
 * @return array
 *   the timezone form element
 */
function date_timezone_element_process($element, &$form_state, $form) {
  if (date_hidden_element($element)) {
    return $element;
  }

  $element['#tree'] = TRUE;
  $label = theme('date_part_label_timezone', array('part_type' => 'select', 'element' => $element));
  $element['timezone'] = array(
    '#type' => 'select',
    '#title' => $label,
    '#title_display' => $element['#date_label_position'] == 'above' ? 'before' : 'invisible',
    '#options' => date_timezone_names($element['#required']),
    '#value' => $element['#value'],
    '#weight' => $element['#weight'],
    '#required' => $element['#required'],
    '#theme' => 'date_select_element',
    '#theme_wrappers' => array('form_element'),
  );
  if (isset($element['#element_validate'])) {
    array_push($element['#element_validate'], 'date_timezone_validate');
  }
  else {
    $element['#element_validate'] = array('date_timezone_validate');
  }

  $context = array(
    'form' => $form,
  );
  backdrop_alter('date_timezone_process', $element, $form_state, $context);

  return $element;
}

/**
 * Validation for timezone input.
 *
 *  Move the timezone value from the nested field back to the original field.
 */
function date_timezone_validate($element, &$form_state) {
  if (date_hidden_element($element)) {
    return;
  }

  form_set_value($element, $element['#value']['timezone'], $form_state);
}

/**
 * Element value callback for date_text element.
 */
function date_text_element_value_callback($element, $input = FALSE, &$form_state = array()) {
  $return = array('date' => '');
  $date = NULL;

  // Normal input from submitting the form element.
  // Check is_array() to skip the string input values created by Views pagers.
  // Those string values, if present, should be interpreted as empty input.
  if ($input != FALSE && is_array($input)) {
    $return = $input;
    $date = date_text_input_date($element, $input);
  }
  // No input? Try the default value.
  elseif (!empty($element['#default_value'])) {
    $date = date_default_date($element);
  }
  if (date_is_date($date)) {
    $return['date'] = date_format_date($date, 'custom', $element['#date_format']);
  }
  return $return;
}

/**
 * Text date input form.
 *
 * Display all or part of a date in a single textfield.
 *
 * The exact parts displayed in the field are those in #date_granularity.
 * The display of each part comes from #date_format.
 */
function date_text_element_process($element, &$form_state, $form) {
  if (date_hidden_element($element)) {
    return $element;
  }

  $element['#tree'] = TRUE;
  $element['#theme_wrappers'] = array('date_text');
  $element['date']['#value'] = $element['#value']['date'];
  $element['date']['#type'] = 'textfield';
  $element['date']['#weight'] = !empty($element['date']['#weight']) ? $element['date']['#weight'] : $element['#weight'];
  $element['date']['#attributes'] = array('class' => isset($element['#attributes']['class']) ? $element['#attributes']['class'] += array('date-date') : array('date-date'));
  $now = date_example_date();
  $element['date']['#title'] = t('Date');
  $element['date']['#title_display'] = 'invisible';
  $element['date']['#description'] = ' ' . t('Format: @date', array(
    '@date' => date_format_date(date_example_date(), 'custom', $element['#date_format']
  )));
  $element['date']['#ajax'] = !empty($element['#ajax']) ? $element['#ajax'] : FALSE;

  // Make changes if instance is set to be rendered as a regular field.
  if (!empty($element['#instance']['widget']['settings']['no_fieldset']) && $element['#field']['cardinality'] == 1) {
    $element['date']['#title'] = check_plain($element['#instance']['label']);
    $element['date']['#title_display'] = $element['#title_display'];
    $element['date']['#required'] = $element['#required'];
  }

  // Keep the system from creating an error message for the sub-element.
  // We'll set our own message on the parent element.
  // $element['date']['#required'] = $element['#required'];
  $element['date']['#theme'] = 'date_textfield_element';
  if (isset($element['#element_validate'])) {
    array_push($element['#element_validate'], 'date_text_validate');
  }
  else {
    $element['#element_validate'] = array('date_text_validate');
  }
  if (!empty($element['#force_value'])) {
    $element['date']['#value'] = $element['date']['#default_value'];
  }

  $context = array(
    'form' => $form,
  );
  backdrop_alter('date_text_process', $element, $form_state, $context);

  return $element;
}

/**
 * Validation for text input.
 *
 * When used as a Views widget, the validation step always gets triggered,
 * even with no form submission. Before form submission $element['#value']
 * contains a string, after submission it contains an array.
 */
function date_text_validate($element, &$form_state) {
  if (date_hidden_element($element)) {
    return;
  }

  if (is_string($element['#value'])) {
    return;
  }
  $input_exists = NULL;
  $input = backdrop_array_get_nested_value($form_state['values'], $element['#parents'], $input_exists);

  // Trim extra spacing off user input of text fields.
  if (isset($input['date'])) {
    $input['date'] = trim($input['date']);
  }

  backdrop_alter('date_text_pre_validate', $element, $form_state, $input);

  $label = !empty($element['#date_title']) ? $element['#date_title'] : (!empty($element['#title']) ? $element['#title'] : '');
  $date = date_text_input_date($element, $input);

  // If the field has errors, display them.
  // If something was input but there is no date, the date is invalid.
  // If the field is empty and required, set error message and return.
  $error_field = implode('][', $element['#parents']);
  if (empty($date) || !empty($date->errors)) {
    if (is_object($date) && !empty($date->errors)) {
      $message = t('The value input for field %field is invalid:', array('%field' => $label));
      $message .= '<br />' . implode('<br />', $date->errors);
      form_set_error($error_field, $message);
      return;
    }
    if (!empty($element['#required'])) {
      $message = t('A valid date is required for %title.', array('%title' => $label));
      form_set_error($error_field, $message);
      return;
    }
    // Fall through, some other error.
    if (!empty($input['date'])) {
      form_error($element, t('%title is invalid.', array('%title' => $label)));
      return;
    }
  }
  $value = !empty($date) ? $date->format(DATE_FORMAT_DATETIME) : NULL;
  form_set_value($element, $value, $form_state);
}

/**
 * Helper function for creating a date object out of user input.
 */
function date_text_input_date($element, $input) {
  if (empty($input) || !is_array($input) || !array_key_exists('date', $input) || empty($input['date'])) {
    return NULL;
  }
  $granularity = date_format_order($element['#date_format']);
  $date = new DateObject($input['date'], $element['#date_timezone'], $element['#date_format']);
  if (is_object($date)) {
    $date->limitGranularity($granularity);
    if ($date->validGranularity($granularity, $element['#date_flexible'])) {
      date_increment_round($date, $element['#date_increment']);
    }
    return $date;
  }
  return NULL;
}

/**
 * Element value callback for date_select element.
 */
function date_select_element_value_callback($element, $input = FALSE, &$form_state = array()) {
  $return = array(
    'year' => '',
    'month' => '',
    'day' => '',
    'hour' => '',
    'minute' => '',
    'second' => '',
  );
  $date = NULL;
  if ($input !== FALSE) {
    $return = $input;
    $date = date_select_input_date($element, $input);
  }
  elseif (!empty($element['#default_value'])) {
    $date = date_default_date($element);
  }
  $granularity = date_format_order($element['#date_format']);
  $formats = array(
    'year' => 'Y',
    'month' => 'n',
    'day' => 'j',
    'hour' => 'H',
    'minute' => 'i',
    'second' => 's',
  );
  foreach ($granularity as $field) {
    if ($field != 'timezone') {
      $return[$field] = date_is_date($date) ? $date->format($formats[$field]) : '';
    }
  }
  return $return;
}

/**
 * Flexible date/time drop-down selector.
 *
 * Splits date into a collection of date and time sub-elements, one
 * for each date part. Each sub-element can be either a textfield or a
 * select, based on the value of ['#date_settings']['text_fields'].
 *
 * The exact parts displayed in the field are those in #date_granularity.
 * The display of each part comes from ['#date_settings']['format'].
 */
function date_select_element_process($element, &$form_state, $form) {
  if (date_hidden_element($element)) {
    return $element;
  }

  $date = NULL;
  $granularity = date_format_order($element['#date_format']);

  if (is_array($element['#default_value'])) {
    $date = date_select_input_date($element, $element['#default_value']);
  }
  elseif (!empty($element['#default_value'])) {
    $date = date_default_date($element);
  }

  $element['#tree'] = TRUE;
  $element['#theme_wrappers'] = array('date_select');

  $element += (array) date_parts_element($element, $date, $element['#date_format']);

  // Store a hidden value for all date parts not in the current display.
  $granularity = date_format_order($element['#date_format']);
  $formats = array(
    'year' => 'Y',
    'month' => 'n',
    'day' => 'j',
    'hour' => 'H',
    'minute' => 'i',
    'second' => 's',
  );
  foreach (date_nongranularity($granularity) as $field) {
    if ($field != 'timezone') {
      $element[$field] = array(
        '#type' => 'value',
        '#value' => 0,
      );
    }
  }
  if (isset($element['#element_validate'])) {
    array_push($element['#element_validate'], 'date_select_validate');
  }
  else {
    $element['#element_validate'] = array('date_select_validate');
  }

  $context = array(
    'form' => $form,
  );
  backdrop_alter('date_select_process', $element, $form_state, $context);

  return $element;
}

/**
 * Creates form elements for one or more date parts.
 *
 * Get the order of date elements from the provided format.
 * If the format order omits any date parts in the granularity, alter the
 * granularity array to match the format, then flip the $order array
 * to get the position for each element. Then iterate through the
 * elements and create a sub-form for each part.
 *
 * @param array $element
 *   The date element.
 * @param object $date
 *   The date object.
 * @param string $format
 *   A date format string.
 *
 * @return array
 *   The form array for the submitted date parts.
 */
function date_parts_element($element, $date, $format) {
  $granularity = date_format_order($format);
  $sub_element = array('#granularity' => $granularity);
  $order = array_flip($granularity);

  $hours_format  = strpos(strtolower($element['#date_format']), 'a') ? 'g' : 'G';
  $month_function  = strpos($element['#date_format'], 'F') !== FALSE ? 'date_month_names' : 'date_month_names_abbr';
  $count = 0;
  $increment = min(intval($element['#date_increment']), 1);

  // Allow empty value as option if date is not required or there is no date.
  $part_required = (bool) $element['#required'] && is_object($date);
  foreach ($granularity as $field) {
    $part_type = in_array($field, $element['#date_text_parts']) ? 'textfield' : 'select';
    $sub_element[$field] = array(
      '#weight' => $order[$field],
      '#required' => $part_required,
      '#attributes' => array('class' => isset($element['#attributes']['class']) ? $element['#attributes']['class'] += array('date-' . $field) : array('date-' . $field)),
      '#ajax' => !empty($element['#ajax']) ? $element['#ajax'] : FALSE,
    );
    switch ($field) {
      case 'year':
        $range = date_range_years($element['#date_year_range'], $date);
        $start_year = $range[0];
        $end_year = $range[1];

        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format('Y') : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = backdrop_map_assoc(date_years($start_year, $end_year, $part_required));
        }
        break;

      case 'month':
        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format('n') : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = $month_function($part_required);
        }
        break;

      case 'day':
        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format('j') : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = backdrop_map_assoc(date_days($part_required));
        }
        break;

      case 'hour':
        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format($hours_format) : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = backdrop_map_assoc(date_hours($hours_format, $part_required));
        }
        $sub_element[$field]['#prefix'] = theme('date_part_hour_prefix', $element);
        break;

      case 'minute':
        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format('i') : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = backdrop_map_assoc(date_minutes('i', $part_required, $element['#date_increment']));
        }
        $sub_element[$field]['#prefix'] = theme('date_part_minsec_prefix', $element);
        break;

      case 'second':
        $sub_element[$field]['#default_value'] = is_object($date) ? $date->format('s') : '';
        if ($part_type == 'select') {
          $sub_element[$field]['#options'] = backdrop_map_assoc(date_seconds('s', $part_required, $element['#date_increment']));
        }
        $sub_element[$field]['#prefix'] = theme('date_part_minsec_prefix', $element);
        break;
    }

    // Add handling for the date part label.
    $label = theme('date_part_label_' . $field, array('part_type' => $part_type, 'element' => $element));
    if (in_array($field, $element['#date_text_parts'])) {
      $sub_element[$field]['#type'] = 'textfield';
      $sub_element[$field]['#theme'] = 'date_textfield_element';
      $sub_element[$field]['#size'] = 7;
      $sub_element[$field]['#title'] = $label;
      $sub_element[$field]['#title_display'] = in_array($element['#date_label_position'], array('within', 'none')) ? 'invisible' : 'before';
      if ($element['#date_label_position'] == 'within') {
        if (!empty($sub_element[$field]['#options']) && is_array($sub_element[$field]['#options'])) {
          $sub_element[$field]['#options'] = array(
            '-' . $label => '-' . $label) + $sub_element[$field]['#options'];
        }
        if (empty($sub_element[$field]['#default_value'])) {
          $sub_element[$field]['#default_value'] = '-' . $label;
        }
      }
    }
    else {
      $sub_element[$field]['#type'] = 'select';
      $sub_element[$field]['#theme'] = 'date_select_element';
      $sub_element[$field]['#title'] = $label;
      $sub_element[$field]['#title_display'] = in_array($element['#date_label_position'], array('within', 'none')) ? 'invisible' : 'before';
      if ($element['#date_label_position'] == 'within') {
        $sub_element[$field]['#options'] = array(
          '' => '-' . $label) + $sub_element[$field]['#options'];
      }
    }
  }

  // Views exposed filters are treated as submitted even if not,
  // so force the #default value in that case. Make sure we set
  // a default that is in the option list.
  if (!empty($element['#force_value'])) {
    $options = $sub_element[$field]['#options'];
    $default = !empty($sub_element[$field]['#default_value']) ? $sub_element[$field]['#default_value'] : array_shift($options);
    $sub_element[$field]['#value'] = $default;
  }

  if (($hours_format == 'g' || $hours_format == 'h') && date_has_time($granularity)) {
    $label = theme('date_part_label_ampm', array('part_type' => 'ampm', 'element' => $element));
    $sub_element['ampm'] = array(
      '#type' => 'select',
      '#theme' => 'date_select_element',
      '#title' => $label,
      '#title_display' => in_array($element['#date_label_position'], array('within', 'none')) ? 'invisible' : 'before',
      '#default_value' => is_object($date) ? (date_format($date, 'G') >= 12 ? 'pm' : 'am') : '',
      '#options' => backdrop_map_assoc(date_ampm($part_required)),
      '#required' => $part_required,
      '#weight' => 8,
      '#attributes' => array('class' => array('date-ampm')),
    );
    if ($element['#date_label_position'] == 'within') {
      $sub_element['ampm']['#options'] = array('' => '-' . $label) + $sub_element['ampm']['#options'];
    }
  }

  return $sub_element;
}

/**
 * Validation function for date selector.
 *
 * When used as a Views widget, the validation step always gets triggered,
 * even with no form submission. Before form submission $element['#value']
 * contains a string, after submission it contains an array.
 */
function date_select_validate($element, &$form_state) {
  if (date_hidden_element($element)) {
    return;
  }

  if (is_string($element['#value'])) {
    return;
  }

  $input_exists = NULL;
  $input = backdrop_array_get_nested_value($form_state['values'], $element['#parents'], $input_exists);

  // Strip field labels out of the results.
  foreach ($element['#value'] as $field => $field_value) {
    if (substr($field_value, 0, 1) == '-') {
      $input[$field] = '';
    }
  }

  backdrop_alter('date_select_pre_validate', $element, $form_state, $input);

  $label = !empty($element['#date_title']) ? $element['#date_title'] : (!empty($element['#title']) ? $element['#title'] : '');
  if (isset($input['ampm'])) {
    if ($input['ampm'] == 'pm' && $input['hour'] < 12) {
      $input['hour'] += 12;
    }
    elseif ($input['ampm'] == 'am' && $input['hour'] == 12) {
      $input['hour'] -= 12;
    }
  }
  unset($input['ampm']);
  $date = date_select_input_date($element, $input);

  // If the field has errors, display them.
  $error_field = implode('][', $element['#parents']);
  $entered = array_values(array_filter($input));
  if (empty($date) || !empty($date->errors)) {
    // The input created a date but it has errors.
    if (is_object($date) && !empty($date->errors)) {
      $message = t('The value input for field %field is invalid:', array('%field' => $label));
      $message .= '<br />' . implode('<br />', $date->errors);
      form_set_error($error_field, $message);
      return;
    }
    // Nothing was entered but the date is required.
    elseif (empty($entered) && $element['#required']) {
      $message = t('A valid date is required for %title.', array('%title' => $label));
      form_set_error($error_field, $message);
      return;
    }
    // Something was input but it wasn't enough to create a valid date.
    elseif (!empty($entered)) {
      $message = t('The value input for field %field is invalid.', array('%field' => $label));
      form_set_error($error_field, $message);
      return;
    }
  }
  $value = !empty($date) ? $date->format(DATE_FORMAT_DATETIME) : NULL;
  form_set_value($element, $value, $form_state);
}

/**
 * Helper function for creating a date object out of user input.
 */
function date_select_input_date($element, $input) {

  // Was anything entered? If not, we have no date.
  if (!is_array($input)) {
    return NULL;
  }
  else {
    $entered = array_values(array_filter($input));
    if (empty($entered)) {
      return NULL;
    }
  }
  $granularity = date_format_order($element['#date_format']);
  if (isset($input['ampm'])) {
    if ($input['ampm'] == 'pm' && $input['hour'] < 12) {
      $input['hour'] += 12;
    }
    elseif ($input['ampm'] == 'am' && $input['hour'] == 12) {
      $input['hour'] -= 12;
    }
  }
  unset($input['ampm']);

  // Make the input match the granularity.
  foreach (date_nongranularity($granularity) as $part) {
    unset($input[$part]);
  }

  $date = new DateObject($input, $element['#date_timezone']);
  if (is_object($date)) {
    $date->limitGranularity($granularity);
    if ($date->validGranularity($granularity, $element['#date_flexible'])) {
      date_increment_round($date, $element['#date_increment']);
    }
    return $date;
  }
  return NULL;
}
