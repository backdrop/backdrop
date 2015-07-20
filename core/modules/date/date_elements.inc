<?php
/**
 * @file
 * Date forms and form themes and validation.
 *
 * All code used in form editing and processing is in this file,
 * included only during form editing.
 */

/**
 * Private implementation of hook_widget().
 *
 * The widget builds out a complex date element in the following way:
 *
 * - A field is pulled out of the database which is comprised of one or
 *   more collections of start/end dates.
 *
 * - The dates in this field are all converted from the UTC values stored
 *   in the database back to the local time. This is done in #process
 *   to avoid making this change to dates that are not being processed,
 *   like those hidden with #access.
 *
 * - If values are empty, the field settings rules are used to determine
 *   if the default_values should be empty, now, the same, or use strtotime.
 *
 * - Each start/end combination is created using the date_combo element type
 *   defined by the date module. If the timezone is date-specific, a
 *   timezone selector is added to the first combo element.
 *
 * - The date combo element creates two individual date elements, one each
 *   for the start and end field, using the appropriate individual Date API
 *   date elements, like selects, textfields, or popups.
 *
 * - In the individual element validation, the data supplied by the user is
 *   used to update the individual date values.
 *
 * - In the combo date validation, the timezone is updated, if necessary,
 *   then the user input date values are used with that timezone to create
 *   date objects, which are used update combo date timezone and offset values.
 *
 * - In the field's submission processing, the new date values, which are in
 *   the local timezone, are converted back to their UTC values and stored.
 */
function date_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $base) {

  $element = $base;
  $field_name = $field['field_name'];
  $entity_type = $instance['entity_type'];

  // If this is a new entity, populate the field with the right default values.
  // This happens early so even fields later hidden with #access get those values.
  // We should only add default values to new entities, to avoid over-writing
  // a value that has already been set. This means we can't just check to see
  // if $items is empty, because it might have been set that way on purpose.
  // @see date_field_widget_properties_alter() where we flagged if this is a new entity.

  // We check !isset($items[$delta]['value']) because entity translation may create
  // a new translation entity for an existing entity and we don't want to clobber
  // values that were already set in that case.
  // @see http://drupal.org/node/1478848.

  $is_default = FALSE;
  if (!empty($instance['widget']['is_new']) && !isset($items[$delta]['value'])) {
    $items = date_default_value($field, $instance, $langcode);
    $is_default = TRUE;
  }

  // @TODO Repeating dates should probably be made into their own field type and completely separated out.
  // That will have to wait for a new branch since it may break other things, including other modules
  // that have an expectation of what the date field types are.

  // Since repeating dates cannot use the default Add more button, we have to handle our own behaviors here.
  // Return only the first multiple value for repeating dates, then clean up the 'Add more' bits in #after_build.
  // The repeating values will be re-generated when the repeat widget form is validated.
  // At this point we can't tell if this form element is going to be hidden by #access, and we're going to
  // lose all but the first value by doing this, so store the original values in case we need to replace them later.
  if (!empty($field['settings']['repeat'])) {
    if ($delta == 0) {
      $form['#after_build'][] = 'date_repeat_after_build';
      $form_state['storage']['repeat_fields'][$field_name] = array_merge($form['#parents'], array($field_name));
      $form_state['storage']['date_items'][$field_name][$langcode] = $items;
    }
    else {
      return;
    }
  }

  module_load_include('inc', 'date_api', 'date_api_elements');
  $timezone = date_get_timezone($field['settings']['tz_handling'], isset($items[$delta]['timezone']) ? $items[$delta]['timezone'] : date_default_timezone());

  // TODO see if there's a way to keep the timezone element from ever being
  // nested as array('timezone' => 'timezone' => value)). After struggling
  // with this a while, I can find no way to get it displayed in the form
  // correctly and get it to use the timezone element without ending up
  // with nesting.
  if (is_array($timezone)) {
    $timezone = $timezone['timezone'];
  }

  $element += array(
    '#type' => 'date_combo',
    '#theme_wrappers' => array('date_combo'),
    '#weight' => $delta,
    '#default_value' => isset($items[$delta]) ? $items[$delta] : '',
    '#date_timezone' => $timezone,
    '#element_validate' => array('date_combo_validate'),
    '#date_is_default' => $is_default,

    // Store the original values, for use with disabled and hidden fields.
    '#date_items' => isset($items[$delta]) ? $items[$delta] : '',
  );

  $element['#title'] = $instance['label'];

  if ($field['settings']['tz_handling'] == 'date') {
    $element['timezone'] = array(
      '#type' => 'date_timezone',
      '#theme_wrappers' => array('date_timezone'),
      '#delta' => $delta,
      '#default_value' => $timezone,
      '#weight' => $instance['widget']['weight'] + 1,
      '#attributes' => array('class' => array('date-no-float')),
      '#date_label_position' => $instance['widget']['settings']['label_position'],
    );
  }

  // Make changes if instance is set to be rendered as a regular field.
  if (!empty($instance['widget']['settings']['no_fieldset'])) {
   $element['#title'] = check_plain($instance['label']);
   $element['#theme_wrappers'] = ($field['cardinality'] == 1) ? array('date_form_element') : array();
  }

  return $element;
}

/**
 * Create local date object.
 *
 * Create a date object set to local time from the field and
 * widget settings and item values. Default values for new entities
 * are set by the default value callback, so don't need to be accounted for here.
 */
function date_local_date($item, $timezone, $field, $instance, $part = 'value') {

  $value = $item[$part];

  // If the value is empty, don't try to create a date object because it will
  // end up being the current day.
  if (empty($value)) {
    return NULL;
  }

  // @TODO Figure out how to replace date_fuzzy_datetime() function.
  // Special case for ISO dates to create a valid date object for formatting.
  // Is this still needed?
  // @codingStandardsIgnoreStart
  /*
  if ($field['type'] == DATE_ISO) {
    $value = date_fuzzy_datetime($value);
  }
  else {
    $db_timezone = date_get_timezone_db($field['settings']['tz_handling']);
    $value = date_convert($value, $field['type'], DATE_DATETIME, $db_timezone);
  }
  */
  // @codingStandardsIgnoreEnd

  $date = new DateObject($value, date_get_timezone_db($field['settings']['tz_handling']));
  $date->limitGranularity($field['settings']['granularity']);
  if (empty($date)) {
    return NULL;
  }
  date_timezone_set($date, timezone_open($timezone));

  return $date;
}

/**
 * The callback for setting a default value for an empty date field.
 */
function date_default_value($field, $instance, $langcode) {
  $item = array();
  $db_format = date_type_format($field['type']);
  $date = date_default_value_part($item, $field, $instance, $langcode, 'value');
  $item[0]['value'] = is_object($date) ? date_format($date, $db_format) : '';
  if (!empty($field['settings']['todate'])) {
    $date = date_default_value_part($item, $field, $instance, $langcode, 'value2');
    $item[0]['value2'] = is_object($date) ? date_format($date, $db_format) : '';
  }

  // Make sure the default value has the same construct as a loaded field value
  // to avoid errors if the default value is used on a hidden element.
  $item[0]['timezone'] = date_get_timezone($field['settings']['tz_handling']);
  $item[0]['timezone_db'] = date_get_timezone_db($field['settings']['tz_handling']);
  $item[0]['date_type'] = $field['type'];
  if (!isset($item[0]['value2'])) {
    $item[0]['value2'] = $item[0]['value'];
  }
  return $item;
}

/**
 * Helper function for the date default value callback to set either 'value' or 'value2' to its default value.
 */
function date_default_value_part($item, $field, $instance, $langcode, $part = 'value') {
  $timezone = date_get_timezone($field['settings']['tz_handling']);
  $timezone_db = date_get_timezone_db($field['settings']['tz_handling']);
  $date = NULL;
  if ($part == 'value') {
    $default_value = $instance['settings']['default_value'];
    $default_value_code = $instance['settings']['default_value_code'];
  }
  else {
    $default_value = $instance['settings']['default_value2'];
    $default_value_code = $instance['settings']['default_value_code2'];
  }
  if (empty($default_value) || $default_value == 'blank') {
    return NULL;
  }
  elseif ($default_value == 'strtotime' && !empty($default_value_code)) {
    $date = new DateObject($default_value_code, date_default_timezone());
  }
  elseif ($part == 'value2' && $default_value == 'same') {
    if ($instance['settings']['default_value'] == 'blank' || empty($item[0]['value'])) {
      return NULL;
    }
    else {
      // The date stored in 'value' has already been switched to the db timezone.
      $date = new DateObject($item[0]['value'], $timezone_db, DATE_FORMAT_DATETIME);
    }
  }
  // Special case for 'now' when using dates with no timezone,
  // make sure 'now' isn't adjusted to UTC value of 'now' .
  elseif ($field['settings']['tz_handling'] == 'none') {
    $date = date_now();
  }
  else {
    $date = date_now($timezone);
  }
  // The default value needs to be in the database timezone.
  date_timezone_set($date, timezone_open($timezone_db));
  $date->limitGranularity($field['settings']['granularity']);
  return $date;
}

/**
 * Process an individual date element.
 */
function date_combo_element_process($element, &$form_state, $form) {
  if (date_hidden_element($element)) {
    // A hidden value for a new entity that had its end date set to blank
    // will not get processed later to populate the end date, so set it here.
    if (isset($element['#value']['value2']) && empty($element['#value']['value2'])) {
      $element['#value']['value2'] = $element['#value']['value'];
    }
    return $element;
  }

  $field_name = $element['#field_name'];
  $delta = $element['#delta'];
  $bundle = $element['#bundle'];
  $entity_type = $element['#entity_type'];
  $langcode = $element['#language'];
  $date_is_default = $element['#date_is_default'];

  $field = field_widget_field($element, $form_state);
  $instance = field_widget_instance($element, $form_state);

  // Figure out how many items are in the form, including new ones added by ajax.
  $field_state = field_form_get_state($element['#field_parents'], $field_name, $element['#language'], $form_state);
  $items_count = $field_state['items_count'];

  $columns = $element['#columns'];
  if (isset($columns['rrule'])) {
    unset($columns['rrule']);
  }
  $from_field = 'value';
  $to_field = 'value2';
  $tz_field = 'timezone';
  $offset_field = 'offset';
  $offset_field2 = 'offset2';

  // Convert UTC dates to their local values in DATETIME format,
  // and adjust the default values as specified in the field settings.

  // It would seem to make sense to do this conversion when the data
  // is loaded instead of when the form is created, but the loaded
  // field data is cached and we can't cache dates that have been converted
  // to the timezone of an individual user, so we cache the UTC values
  // instead and do our conversion to local dates in the form and
  // in the formatters.
  $process = date_process_values($field, $instance);
  foreach ($process as $processed) {
    if (!isset($element['#default_value'][$processed])) {
      $element['#default_value'][$processed] = '';
    }
    $date = date_local_date($element['#default_value'], $element['#date_timezone'], $field, $instance, $processed);
    $element['#default_value'][$processed] = is_object($date) ? date_format($date, DATE_FORMAT_DATETIME) : '';
  }

  // Blank out the end date for optional end dates that match the start date,
  // except when this is a new node that has default values that should be honored.
  if (!$date_is_default && $field['settings']['todate'] != 'required'
  && !empty($element['#default_value'][$to_field])
  && $element['#default_value'][$to_field] == $element['#default_value'][$from_field]) {
    unset($element['#default_value'][$to_field]);
  }

  $show_todate = !empty($form_state['values']['show_todate']) || !empty($element['#default_value'][$to_field]) || $field['settings']['todate'] == 'required';
  $element['show_todate'] = array(
    '#title' => t('Show End Date'),
    '#type' => 'checkbox',
    '#default_value' => $show_todate,
    '#weight' => -20,
    '#access' => $field['settings']['todate'] == 'optional',
    '#prefix' => '<div class="date-float">',
    '#suffix' => '</div>',
  );

  $parents = $element['#parents'];
  $first_parent = array_shift($parents);
  $show_id = $first_parent . '[' . implode('][', $parents) . '][show_todate]';

  $element[$from_field] = array(
    '#field'         => $field,
    '#instance'      => $instance,
    '#weight'        => $instance['widget']['weight'],
    '#required'      => ($element['#required'] && $delta == 0) ? 1 : 0,
    '#default_value' => isset($element['#default_value'][$from_field]) ? $element['#default_value'][$from_field] : '',
    '#delta'         => $delta,
    '#date_timezone' => $element['#date_timezone'],
    '#date_format'      => date_limit_format(date_input_format($element, $field, $instance), $field['settings']['granularity']),
    '#date_text_parts'  => (array) $instance['widget']['settings']['text_parts'],
    '#date_increment'   => $instance['widget']['settings']['increment'],
    '#date_year_range'  => $instance['widget']['settings']['year_range'],
    '#date_label_position' => $instance['widget']['settings']['label_position'],
  );

  $description = !empty($element['#description']) ? t($element['#description']) : '';
  unset($element['#description']);

  // Give this element the right type, using a Date API
  // or a Date Popup element type.
  $element[$from_field]['#attributes'] = array('class' => array('date-clear'));
  $element[$from_field]['#wrapper_attributes']['class'] = array();
  $element[$from_field]['#wrapper_attributes']['class'][] = 'date-no-float';

  switch ($instance['widget']['type']) {
    case 'date_select':
      $element[$from_field]['#type'] = 'date_select';
      $element[$from_field]['#theme_wrappers'] = array('date_select');
      $element['#attached']['js'][] = backdrop_get_path('module', 'date') . '/date.js';
      $element[$from_field]['#ajax'] = !empty($element['#ajax']) ? $element['#ajax'] : FALSE;
      break;

    case 'date_popup':
      $element[$from_field]['#type'] = 'date_popup';
      $element[$from_field]['#theme_wrappers'] = array('date_popup');
      $element[$from_field]['#ajax'] = !empty($element['#ajax']) ? $element['#ajax'] : FALSE;
      break;

    default:
      $element[$from_field]['#type'] = 'date_text';
      $element[$from_field]['#theme_wrappers'] = array('date_text');
      $element[$from_field]['#ajax'] = !empty($element['#ajax']) ? $element['#ajax'] : FALSE;
      break;
  }

  // If this field uses the 'End', add matching element
  // for the 'End' date, and adapt titles to make it clear which
  // is the 'Start' and which is the 'End' .

  if (!empty($field['settings']['todate'])) {
    $element[$to_field] = $element[$from_field];
    $element[$from_field]['#title_display'] = 'none';
    $element[$to_field]['#title'] = t('to:');
    $element[$from_field]['#wrapper_attributes']['class'][] = 'start-date-wrapper';
    $element[$to_field]['#wrapper_attributes']['class'][] = 'end-date-wrapper';
    $element[$to_field]['#default_value'] = isset($element['#default_value'][$to_field]) ? $element['#default_value'][$to_field] : '';
    $element[$to_field]['#required'] = ($element[$from_field]['#required'] && $field['settings']['todate'] == 'required');
    $element[$to_field]['#weight'] += .2;
    $element[$to_field]['#prefix'] = '';
    // Users with JS enabled will never see initially blank values for the end
    // date (see Backdrop.date.EndDateHandler()), so hide the message for them.
    $description .= '<span class="js-hide"> ' . t("Empty 'End date' values will use the 'Start date' values.") . '</span>';
    $element['#fieldset_description'] = $description;
    if ($field['settings']['todate'] == 'optional') {
      $element[$to_field]['#states'] = array(
        'visible' => array(
          'input[name="' . $show_id . '"]' => array(
            'checked' => TRUE,
          ),
        ),
      );
    }
  }
  else {
    $element[$from_field]['#description'] = $description;
  }

  // Create label for error messages that make sense in multiple values
  // and when the title field is left blank.
  if ($field['cardinality'] <> 1 && empty($field['settings']['repeat'])) {
    $element[$from_field]['#date_title'] = t('@field_name Start date value #@delta', array('@field_name' => $instance['label'], '@delta' => $delta + 1));
    if (!empty($field['settings']['todate'])) {
      $element[$to_field]['#date_title'] = t('@field_name End date value #@delta', array('@field_name' => $instance['label'], '@delta' => $delta + 1));
    }
  }
  elseif (!empty($field['settings']['todate'])) {
    $element[$from_field]['#date_title'] = t('@field_name Start date', array('@field_name' => $instance['label']));
    $element[$to_field]['#date_title'] = t('@field_name End date', array('@field_name' => $instance['label']));
  }
  else {
    $element[$from_field]['#date_title'] = t('@field_name', array('@field_name' => $instance['label']));
  }

  // Make changes if instance is set to be rendered as a regular field.
  if (!empty($instance['widget']['settings']['no_fieldset'])) {
    unset($element[$from_field]['#description']);
    if (!empty($field['settings']['todate']) && isset($element['#description'])) {
      $element['#description'] .= '<span class="js-hide"> ' . t("Empty 'End date' values will use the 'Start date' values.") . '</span>';
    }
  }

  $context = array(
    'field'     => $field,
    'instance'  => $instance,
    'form'      => $form,
  );
  backdrop_alter('date_combo_process', $element, $form_state, $context);

  return $element;
}

/**
 * Empty a date element.
 */
function date_element_empty($element, &$form_state) {
  $item = array();
  $item['value'] = NULL;
  $item['value2']   = NULL;
  $item['timezone']   = NULL;
  $item['offset'] = NULL;
  $item['offset2'] = NULL;
  $item['rrule'] = NULL;
  form_set_value($element, $item, $form_state);
  return $item;
}

/**
 * Validate and update a combo element.
 *
 * Don't try this if there were errors before reaching this point.
 */
function date_combo_validate($element, &$form_state) {

  // Disabled and hidden elements won't have any input and don't need validation,
  // we just need to re-save the original values, from before they were processed into
  // widget arrays and timezone-adjusted.
  if (date_hidden_element($element) || !empty($element['#disabled'])) {
    form_set_value($element, $element['#date_items'], $form_state);
    return;
  }

  $field_name = $element['#field_name'];
  $delta = $element['#delta'];
  $langcode = $element['#language'];

  // Related issue: https://drupal.org/node/2279831.
  if (!is_array($element['#field_parents'])) {
    $element['#field_parents'] = array();
  }
  $form_values = backdrop_array_get_nested_value($form_state['values'], $element['#field_parents']);
  $form_input = backdrop_array_get_nested_value($form_state['input'], $element['#field_parents']);

  // If the whole field is empty and that's OK, stop now.
  if (empty($form_input[$field_name]) && !$element['#required']) {
    return;
  }

  $item = backdrop_array_get_nested_value($form_state['values'], $element['#parents']);
  $posted = backdrop_array_get_nested_value($form_state['input'], $element['#parents']);

  $field = field_widget_field($element, $form_state);
  $instance = field_widget_instance($element, $form_state);

  $context = array(
    'field' => $field,
    'instance' => $instance,
    'item' => $item,
  );

  backdrop_alter('date_combo_pre_validate', $element, $form_state, $context);

  $from_field = 'value';
  $to_field = 'value2';
  $tz_field = 'timezone';
  $offset_field = 'offset';
  $offset_field2 = 'offset2';

  // Check for empty 'Start date', which could either be an empty
  // value or an array of empty values, depending on the widget.
  $empty = TRUE;
  if (!empty($item[$from_field])) {
    if (!is_array($item[$from_field])) {
      $empty = FALSE;
    }
    else {
      foreach ($item[$from_field] as $key => $value) {
        if (!empty($value)) {
          $empty = FALSE;
          break;
        }
      }
    }
  }

  // An 'End' date without a 'Start' date is a validation error.
  if ($empty && !empty($item[$to_field])) {
    if (!is_array($item[$to_field])) {
      form_error($element, t("A 'Start date' date is required if an 'end date' is supplied for field %field #%delta.", array('%delta' => $field['cardinality'] ? intval($delta + 1) : '', '%field' => $instance['label'])));
      $empty = FALSE;
    }
    else {
      foreach ($item[$to_field] as $key => $value) {
        if (!empty($value)) {
          form_error($element, t("A 'Start date' date is required if an 'End date' is supplied for field %field #%delta.", array('%delta' => $field['cardinality'] ? intval($delta + 1) : '', '%field' => $instance['label'])));
          $empty = FALSE;
          break;
        }
      }
    }
  }

  // If the user chose the option to not show the end date, just swap in the
  // start date as that value so the start and end dates are the same.
  if ($field['settings']['todate'] == 'optional' && empty($item['show_todate'])) {
    $item[$to_field] = $item[$from_field];
    $posted[$to_field] = $posted[$from_field];
  }

  if ($empty) {
    $item = date_element_empty($element, $form_state);
    if (!$element['#required']) {
      return;
    }
  }
  // Don't look for further errors if errors are already flagged
  // because otherwise we'll show errors on the nested elements
  // more than once.
  elseif (!form_get_errors()) {

    $timezone = !empty($item[$tz_field]) ? $item[$tz_field] : $element['#date_timezone'];
    $timezone_db = date_get_timezone_db($field['settings']['tz_handling']);
    $element[$from_field]['#date_timezone'] = $timezone;
    $from_date = date_input_date($field, $instance, $element[$from_field], $posted[$from_field]);

    if (!empty($field['settings']['todate'])) {
      $element[$to_field]['#date_timezone'] = $timezone;
      $to_date = date_input_date($field, $instance, $element[$to_field], $posted[$to_field]);
    }
    else {
      $to_date = $from_date;
    }

    // Neither the start date nor the end date should be empty at this point
    // unless they held values that couldn't be evaluated.

    if (!$instance['required'] && (!date_is_date($from_date) || !date_is_date($to_date))) {
      $item = date_element_empty($element, $form_state);
      $errors[] = t('The dates are invalid.');
    }
    elseif (!empty($field['settings']['todate']) && $from_date > $to_date) {
      form_set_value($element[$to_field], $to_date, $form_state);
      $errors[] = t('The End date must be greater than the Start date.');
    }
    else {
      // Convert input dates back to their UTC values and re-format to ISO
      // or UNIX instead of the DATETIME format used in element processing.
      $item[$tz_field] = $timezone;

      // Update the context for changes in the $item, and allow other modules to
      // alter the computed local dates.
      $context['item'] = $item;
      // We can only pass two additional values to backdrop_alter, so $element
      // needs to be included in $context.
      $context['element'] = $element;
      backdrop_alter('date_combo_validate_date_start', $from_date, $form_state, $context);
      backdrop_alter('date_combo_validate_date_end', $to_date, $form_state, $context);

      $item[$offset_field] = date_offset_get($from_date);

      $test_from = date_format($from_date, 'r');
      $test_to = date_format($to_date, 'r');

      $item[$offset_field2] = date_offset_get($to_date);
      date_timezone_set($from_date, timezone_open($timezone_db));
      date_timezone_set($to_date, timezone_open($timezone_db));
      $item[$from_field] = date_format($from_date, date_type_format($field['type']));
      $item[$to_field] = date_format($to_date, date_type_format($field['type']));
      if (isset($form_values[$field_name]['rrule'])) {
        $item['rrule'] = $form_values[$field['field_name']]['rrule'];
      }

      // If the db timezone is not the same as the display timezone
      // and we are using a date with time granularity,
      // test a roundtrip back to the original timezone to catch
      // invalid dates, like 2AM on the day that spring daylight savings
      // time begins in the US.
      $granularity = date_format_order($element[$from_field]['#date_format']);
      if ($timezone != $timezone_db && date_has_time($granularity)) {
        date_timezone_set($from_date, timezone_open($timezone));
        date_timezone_set($to_date, timezone_open($timezone));

        if ($test_from != date_format($from_date, 'r')) {
          $errors[] = t('The Start date is invalid.');
        }
        if ($test_to != date_format($to_date, 'r')) {
          $errors[] = t('The End date is invalid.');
        }
      }
      if (empty($errors)) {
        form_set_value($element, $item, $form_state);
      }
    }
  }
  if (!empty($errors)) {
    if ($field['cardinality']) {
      form_error($element, t('There are errors in @field_name value #@delta:', array('@field_name' => $instance['label'], '@delta' => $delta + 1)) . theme('item_list', array('items' => $errors)));
    }
    else {
      form_error($element, t('There are errors in @field_name:', array('@field_name' => $instance['label'])) . theme('item_list', array('items' => $errors)));
    }
  }
}

/**
 * Determine the input format for this element.
 */
function date_input_format($element, $field, $instance) {
  if (!empty($instance['widget']['settings']['input_format_custom'])) {
    return $instance['widget']['settings']['input_format_custom'];
  }
  elseif (!empty($instance['widget']['settings']['input_format']) && $instance['widget']['settings']['input_format'] != 'site-wide') {
    return $instance['widget']['settings']['input_format'];
  }
  return system_date_format_load('short')['pattern'];
}


/**
 * Implements hook_date_select_pre_validate_alter().
 */
function date_date_select_pre_validate_alter(&$element, &$form_state, &$input) {
  date_empty_end_date($element, $form_state, $input);
}

/**
 * Implements hook_date_text_pre_validate_alter().
 */
function date_date_text_pre_validate_alter(&$element, &$form_state, &$input) {
  date_empty_end_date($element, $form_state, $input);
}

/**
 * Implements hook_date_popup_pre_validate_alter().
 */
function date_date_popup_pre_validate_alter(&$element, &$form_state, &$input) {
  date_empty_end_date($element, $form_state, $input);
}

/**
 * Helper function to clear out end date when not being used.
 */
function date_empty_end_date(&$element, &$form_state, &$input) {
  // If this is the end date and the option to show an end date has not been selected,
  // empty the end date to surpress validation errors and stop further processing.
  $parents = $element['#parents'];
  $parent = array_pop($parents);
  if ($parent == 'value2') {
    $parent_values = backdrop_array_get_nested_value($form_state['values'], $parents);
    if (isset($parent_values['show_todate']) && $parent_values['show_todate'] != 1) {
      $input = array();
      form_set_value($element, NULL, $form_state);
    }
  }
}
