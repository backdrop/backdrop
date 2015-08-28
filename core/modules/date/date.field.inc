<?php

/**
 * @file
 * Field hooks to implement a date field.
 */

/**
 * Implements hook_field_formatter_info().
 */
function date_field_formatter_info() {
  $formatters = array(
    'date_default' => array(
      'label' => t('Date and time'),
      'field types' => array('date', 'datestamp', 'datetime'),
      'settings' => array(
        'format_type' => 'long',
        'multiple_number' => '',
        'multiple_from' => '',
        'multiple_to' => '',
        'fromto' => 'both',
        'show_remaining_days' => FALSE,
      ),
    ),
    'format_interval' => array(
      'label' => t('Time ago'),
      'field types' => array('date', 'datestamp', 'datetime'),
      'settings' => array(
        'interval' => 2,
        'interval_display' => 'time ago',
      ),
    ),
    'date_plain' => array(
      'label' => t('Plain'),
      'field types' => array('date', 'datestamp', 'datetime'),
    ),
  );
  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function date_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $formatter = $display['type'];
  module_load_include('inc', 'date', 'date_admin');
  switch ($formatter) {
    case 'format_interval':
      $form = date_interval_formatter_settings_form($field, $instance, $view_mode, $form, $form_state);
      break;

    default:
      $form = date_default_formatter_settings_form($field, $instance, $view_mode, $form, $form_state);
      break;
  }
  $context = array(
    'field' => $field,
    'instance' => $instance,
    'view_mode' => $view_mode,
  );
  backdrop_alter('date_field_formatter_settings_form', $form, $form_state, $context);
  return $form;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function date_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $formatter = $display['type'];
  module_load_include('inc', 'date', 'date_admin');
  switch ($formatter) {
    case 'format_interval':
      $summary = date_interval_formatter_settings_summary($field, $instance, $view_mode);
      break;

    default:
      $summary = date_default_formatter_settings_summary($field, $instance, $view_mode);
      break;
  }
  $context = array(
    'field' => $field,
    'instance' => $instance,
    'view_mode' => $view_mode,
  );
  backdrop_alter('date_field_formatter_settings_summary', $summary, $context);

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 *
 * Useful values:
 *
 *   $entity->date_id
 *     If set, this will show only an individual date on a field with
 *     multiple dates. The value should be a string that contains
 *     the following values, separated with periods:
 *     - module name of the module adding the item
 *     - node nid
 *     - field name
 *     - delta value of the field to be displayed
 *     - other information the module's custom theme might need
 *
 *     Used by the calendar module and available for other uses.
 *     example: 'date:217:field_date:3:test'
 *
 *   $entity->date_repeat_show
 *     If true, tells the theme to show all the computed values
 *     of a repeating date. If not true or not set, only the
 *     start date and the repeat rule will be displayed.
 */
function date_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];
  $formatter = $display['type'];
  $variables = array(
    'entity' => $entity,
    'entity_type' => $entity_type,
    'field' => $field,
    'instance' => $instance,
    'langcode' => $langcode,
    'items' => $items,
    'display' => $display,
    'dates' => array(),
    'attributes' => array(),
    'rdf_mapping' => array(),
    'add_rdf' => module_exists('rdf'),
    'microdata' => array(),
    'add_microdata' => module_exists('microdata'),
  );

  // If the microdata module is enabled, the microdata mapping will have been
  // passed in via the entity.
  if ($variables['add_microdata'] && isset($entity->microdata[$field['field_name']])) {
    $variables['microdata'] = $entity->microdata[$field['field_name']];
  }

  // If there is an RDf mapping for this date field, pass it down to the theme.
  $rdf_mapping = array();
  if (!empty($entity->rdf_mapping) && function_exists('rdf_rdfa_attributes')) {
    if (!empty($entity->rdf_mapping[$field['field_name']])) {
      $variables['rdf_mapping'] = $rdf_mapping = $entity->rdf_mapping[$field['field_name']];
    }
  }

  // Give other modules a chance to prepare the entity before formatting it.
  backdrop_alter('date_formatter_pre_view', $entity, $variables);

  // See if we are only supposed to display a selected
  // item from multiple value date fields.
  $selected_deltas = array();
  if (!empty($entity->date_id)) {
    foreach ((array) $entity->date_id as $key => $id) {
      list($module, $nid, $field_name, $selected_delta, $other) = explode('.', $id . '.');
      if ($field_name == $field['field_name']) {
        $selected_deltas[] = $selected_delta;
      }
    }
  }

  switch ($display['type']) {
    case 'date_plain':
      foreach ($items as $delta => $item) {
        if (!empty($entity->date_id) && !in_array($delta, $selected_deltas)) {
          continue;
        }
        else {
          if (empty($item['value2']) || $item['value'] == $item['value2']) {
            $element[$delta] = array('#markup' => $item['value']);
          }
          else {
            $element[$delta] = array(
              '#markup' => t('!start-date to !end-date', array(
                '!start-date' => $item['value'],
                '!end-date' => $item['value2']
              )));
          }
        }
      }
      break;

    case 'format_interval':
      foreach ($items as $delta => $item) {
        if (!empty($entity->date_id) && !in_array($delta, $selected_deltas)) {
          continue;
        }
        else {
          $variables['delta'] = $delta;
          $variables['item'] = $item;
          $variables['dates'] = date_formatter_process($formatter, $entity_type, $entity, $field, $instance, $langcode, $item, $display);
          $variables['attributes'] = !empty($rdf_mapping) ? rdf_rdfa_attributes($rdf_mapping, $item['value']) : array();
          $element[$delta] = array('#markup' => theme('date_display_interval', $variables));
        }
      }
      break;

    default:
      foreach ($items as $delta => $item) {
        if (!empty($entity->date_id) && !in_array($delta, $selected_deltas)) {
          continue;
        }
        else {
          $variables['delta'] = $delta;
          $variables['item'] = $item;
          $variables['dates'] = date_formatter_process($formatter, $entity_type, $entity, $field, $instance, $langcode, $item, $display);
          $variables['attributes'] = !empty($rdf_mapping) ? rdf_rdfa_attributes($rdf_mapping, $item['value']) : array();
          $variables['show_remaining_days'] = $display['settings']['show_remaining_days'];
          $output = theme('date_display_combination', $variables);
          if (!empty($output)) {
            $element[$delta] = array('#markup' => $output);
          }
        }
      }
      break;
  }
  return $element;
}

/**
 * Implements hook_field_is_empty().
 */
function date_field_is_empty($item, $field) {
  // Sometimes a $item is a date object.
  // Coming from repeating dates. Why??
  if (!is_array($item)) {
    return FALSE;
  }
  if (empty($item['value'])) {
    return TRUE;
  }
  elseif ($field['settings']['todate'] == 'required' && empty($item['value2'])) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Implements hook_field_info().
 */
function date_field_info() {
  $granularity = array('year', 'month', 'day', 'hour', 'minute');
  $settings = array(
    'settings' => array(
      'todate' => '',
      'granularity' => backdrop_map_assoc($granularity),
      'tz_handling' => 'site',
      'timezone_db' => 'UTC',
    ),
    'instance_settings' => array(
      'default_value' => 'now',
      'default_value_code' => '',
      'default_value2' => 'same',
      'default_value_code2' => '',
    ),
    // Integrate with the Entity Metadata module.
    'property_type' => 'date',
    'property_callbacks' => array('date_entity_metadata_property_info_alter'),
  );
  return array(
    'datetime' => array(
      'label' => 'Date',
      'description' => t('Store a date in the database as a datetime field, recommended for complete dates and times that may need timezone conversion.'),
      'default_widget' => 'date_select',
      'default_formatter' => 'date_default',
      'default_token_formatter' => 'date_plain',
    ) + $settings,
    'date' => array(
      'label' => 'Date (ISO format)',
      'description' => t('Store a date in the database as an ISO date, recommended for historical or partial dates.'),
      'default_widget' => 'date_select',
      'default_formatter' => 'date_default',
      'default_token_formatter' => 'date_plain',
    ) + $settings,
    'datestamp' => array(
      'label' => 'Date (Unix timestamp)',
      'description' => t('Store a date in the database as a timestamp, deprecated format to support legacy data.'),
      'default_widget' => 'date_select',
      'default_formatter' => 'date_default',
      'default_token_formatter' => 'date_plain',
    ) + $settings,
  );
}

/**
 * Implements hook_field_widget_info().
 */
function date_field_widget_info() {
  $settings = array(
    'settings' => array(
      'input_format' => date_default_format('date_select'),
      'input_format_custom' => '',
      'increment' => 15,
      'text_parts' => array(),
      'year_range' => '-3:+3',
      'label_position' => 'above',
    ),
    'behaviors' => array(
      'multiple values' => FIELD_BEHAVIOR_DEFAULT,
      'default value' => FIELD_BEHAVIOR_NONE,
    ),
  );

  $info = array(
    'date_select' => array(
      'label' => t('Select list'),
      'field types' => array('date', 'datestamp', 'datetime'),
    ) + $settings,
    'date_text' => array(
      'label' => t('Text field'),
      'field types' => array('date', 'datestamp', 'datetime'),
    ) + $settings,
  );

  if (module_exists('date_popup')) {
    $info['date_popup'] = array(
      'label' => t('Pop-up calendar'),
      'field types' => array('date', 'datestamp', 'datetime'),
    ) + $settings;
  }

  // The date text widget should use an increment of 1.
  $info['date_text']['increment'] = 1;

  return $info;
}

/**
 * Implements hook_field_load().
 */
function date_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  $timezone_db = date_get_timezone_db($field['settings']['tz_handling']);
  $db_format = date_type_format($field['type']);
  $process = date_process_values($field);
  foreach ($entities as $id => $entity) {
    foreach ($items[$id] as $delta => &$item) {
      // If the file does not exist, mark the entire item as empty.
      if (is_array($item)) {
        $timezone = isset($item['timezone']) ? $item['timezone'] : '';
        $item['timezone'] = date_get_timezone($field['settings']['tz_handling'], $timezone);
        $item['timezone_db'] = $timezone_db;
        $item['date_type'] = $field['type'];
        if (!empty($field['settings']['cache_enabled']) && ($delta < $field['settings']['cache_count'] || $field['settings']['cache_count'] == 0)) {
          foreach ($process as $processed) {
            if (!empty($item[$processed])) {
              $date = new DateObject($item[$processed], $item['timezone_db'], $db_format);
              $date->limitGranularity($field['settings']['granularity']);
              $item['db'][$processed] = $date;
            }
          }
          if (!empty($item['db']['value']) && empty($item['db']['value2'])) {
            $item['db']['value2'] = $item['db']['value'];
          }
        }
      }
    }
  }
}

/**
 * Implements hook_field_validate().
 */
function date_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  $field_name = $field['field_name'];
  $flexible = 0;

  // Don't try to validate if there were any errors before this point
  // since the element won't have been munged back into a date.
  if (!form_get_errors()) {
    foreach ($items as $delta => $item) {
      if (is_array($item) && isset($item['value'])) {
        $process = date_process_values($field, $instance);
        $date1 = new DateObject($item['value'], $item['timezone'], date_type_format($field['type']));
        if (count($process) == 1 || (empty($item['value2']) && $item['value2'] !== 0)) {
          $date2 = clone($date1);
        }
        else {
          $date2 = new DateObject($item['value2'], $item['timezone'], date_type_format($field['type']));
        }
        $valid1 = $date1->validGranularity($field['settings']['granularity'], $flexible);
        $valid2 = $date2->validGranularity($field['settings']['granularity'], $flexible);

        foreach ($process as $processed) {
          if ($processed == 'value' && $field['settings']['todate'] && !$valid1 && $valid2) {
            $errors[$field['field_name']][$langcode][$delta][] = array(
              'error' => 'value',
              'message' => t("A 'Start date' date is required for field %field #%delta.", array('%delta' => $field['cardinality'] ? intval($delta + 1) : '', '%field' => $instance['label'])),
            );
          }
          if ($processed == 'value2' && $field['settings']['todate'] == 'required' && ($instance['required'] && $valid1 && !$valid2)) {
            $errors[$field['field_name']][$langcode][$delta][] = array(
              'error' => 'value2',
              'message' => t("An 'End date' is required for field %field #%delta.", array('%delta' => $field['cardinality'] ? intval($delta + 1) : '', '%field' => $instance['label'])),
            );
          }
        }
      }
    }
  }
}

/**
 * Implements hook_field_insert().
 */
function date_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
  $field_name = $field['field_name'];

  if (empty($items)) {
    return;
  }
  // Add some information needed to interpret token values.
  $values = $items;
  foreach ($values as $delta => $item) {
    $timezone = isset($item['timezone']) ? $item['timezone'] : '';
    if (is_array($item)) {
      $items[$delta]['timezone'] = date_get_timezone($field['settings']['tz_handling'], $timezone);
      $items[$delta]['timezone_db'] = date_get_timezone_db($field['settings']['tz_handling']);
      $items[$delta]['date_type'] = $field['type'];
    }
  }
  $entity->{$field['field_name']}[$langcode] = $items;
}

/**
 * Implements hook_field_insert().
 */
function date_field_insert($entity_type, $entity, $field, $instance, $langcode, &$items) {
  $context = array(
    'entity_type' => $entity_type,
    'entity' => $entity,
    'field' => $field,
    'instance' => $instance,
    'langcode' => $langcode,
  );
  backdrop_alter('date_field_insert', $items, $context);
}

/**
 * Implements hook_field_update().
 */
function date_field_update($entity_type, $entity, $field, $instance, $langcode, &$items) {
  $context = array(
    'entity_type' => $entity_type,
    'entity' => $entity,
    'field' => $field,
    'instance' => $instance,
    'langcode' => $langcode,
  );
  backdrop_alter('date_field_update', $items, $context);
}

/**
 * Implements hook_field_instance_settings_form().
 *
 * Wrapper functions for date administration, included only when processing
 * field settings.
 */
function date_field_instance_settings_form($field, $instance) {
  module_load_include('inc', 'date', 'date_admin');
  return _date_field_instance_settings_form($field, $instance);
}

/**
 * Implements hook_field_widget_settings_form().
 */
function date_field_widget_settings_form($field, $instance) {
  module_load_include('inc', 'date', 'date_admin');
  return _date_field_widget_settings_form($field, $instance);
}

/**
 * Implements hook_field_settings_form().
 */
function date_field_settings_form($field, $instance, $has_data) {
  module_load_include('inc', 'date', 'date_admin');
  return _date_field_settings_form($field, $instance, $has_data);
}

/**
 * Implements hook_content_migrate_field_alter().
 *
 * Use this to tweak the conversion of field settings from the D6 style to the
 * D7 style for specific situations not handled by basic conversion, as when
 * field types or settings are changed.
 *
 * $field_value['widget_type'] is available to see what widget type was
 * originally used.
 */
function date_content_migrate_field_alter(&$field_value, $instance_value) {
  switch ($field_value['module']) {
    case 'date':
      // Those settings do not exist anymore, or have been moved to the instance
      // level.
      unset($field_value['settings']['default_format']);
      unset($field_value['settings']['repeat_collapsed']);
      break;
  }
}

/**
 * Implements hook_content_migrate_instance_alter().
 *
 * Use this to tweak the conversion of instance or widget settings from the D6
 * style to the D7 style for specific situations not handled by basic
 * conversion, as when formatter or widget names or settings are changed.
 */
function date_content_migrate_instance_alter(&$instance_value, $field_value) {
  switch ($instance_value['widget']['module']) {
    case 'date':

      // Some settings have been moved from field to instance.
      $instance_value['widget']['settings']['repeat_collapsed'] = $field_value['settings']['repeat_collapsed'];

      // Some settings were moved from widget settings to instance settings.
      $instance_value['settings']['default_value'] = $instance_value['default_value'];
      unset($instance_value['default_value']);
      $instance_value['settings']['default_value_code'] = $instance_value['widget']['settings']['default_value_code'];
      unset($instance_value['widget']['settings']['default_value_code']);
      $instance_value['settings']['default_value2'] = $instance_value['widget']['settings']['default_value2'];
      unset($instance_value['widget']['settings']['default_value2']);
      $instance_value['settings']['default_value_code2'] = $instance_value['widget']['settings']['default_value_code2'];
      unset($instance_value['widget']['settings']['default_value_code2']);

      // We need to retrieve formatter settings from the variables and store
      // them in the instance.
      foreach ($instance_value['display'] as $context => &$display) {
        if ($display['type'] != 'format_interval') {
          $old_settings = date_old_formatter_get_settings($instance_value['field_name'], $instance_value['bundle'], $context);
          $display['settings'] = array_merge($display['settings'], $old_settings);
          // If the formatter was the 'default', then use the old
          // 'default_format' field property.
          $format = ($display['type'] == 'default') ? $field_value['settings']['default_format'] : $display['type'];
          $display['settings']['format_type'] = $format;
          $display['type'] = 'date_default';
        }
      }

      break;
  }
}

/**
 * Constructs an array of old formatter settings.
 */
function date_old_formatter_get_settings($field_name, $type_name, $context) {
  $options = array();
  $value = 'date:' . $type_name . ':' . $context . ':' . $field_name;
  $options['show_repeat_rule'] = config_get('date.settings','show_repeat_rule');
  $options['multiple_number'] = config_get('date.settings','multiple_number');
  $options['multiple_from'] = config_get('date.settings','multiple_from');
  $options['multiple_to'] = config_get('date.settings','multiple_to');
  $options['fromto'] = config_get('date.settings','fromto');
  return $options;
}
