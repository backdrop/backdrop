<?php

/**
 * @file
 * The Date Wizard code.
 */

/**
 * Implements hook_form().
 *
 * @todo.
 */
function date_tools_wizard_form() {
  $form = array();
  $form['type'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content type'),
  );
  $form['type']['bundle'] = array(
    '#type' => 'textfield',
    '#default_value' => 'date',
    '#title' => t('Content type name'),
    '#description' => t('Machine-readable name. Allowed values: (a-z, 0-9, _). If this is not an existing content type, the content type will be created.'),
  );
  $form['type']['name'] = array(
    '#type' => 'textfield',
    '#default_value' => t('Date'),
    '#title' => t('Content type label'),
    '#description' => t('The human-readable name for this content type. Only needed when creating a new content type.'),
  );
  $form['type']['type_description'] = array(
    '#type' => 'textarea',
    '#default_value' => t('A date content type that is linked to a Views calendar.'),
    '#title' => t('Content type description'),
    '#description' => t('A description for the content type. Only needed when creating a new content type.'),
  );
  $form['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('Date field'),
  );
  $form['field']['field_name'] = array(
    '#type' => 'textfield',
    '#default_value' => 'date',
    '#field_prefix' => 'field_',
    '#title' => t('Date field name'),
    '#description' => t('Machine-readable name. Allowed values: (a-z, 0-9, _) Must not be an existing field name.'),
  );
  $form['field']['label'] = array(
    '#tree' => TRUE,
    '#type' => 'textfield',
    '#default_value' => t('Date'),
    '#title' => t('Date field label'),
    '#description' => t('The human-readable label for this field.'),
  );
  $form['field']['widget_type'] = array(
    '#type' => 'select',
    '#options' => date_tools_wizard_widget_types(),
    '#default_value' => 'date_select',
    '#title' => t('Date widget type'),
  );
  $form['field']['repeat'] = array(
    '#type' => 'select',
    '#default_value' => 0,
    '#options' => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#title' => t('Show repeating date options'),
    '#access' => module_exists('date_repeat_field'),
  );
  $form['field']['advanced'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Advanced options'),
  );
  $form['field']['advanced']['todate'] = array(
    '#type' => 'select',
    '#default_value' => 'optional',
    '#options' => array(
      '' => t('Never'),
      'optional' => t('Optional'),
      'required' => t('Required'),
    ),
    '#title' => t('End Date'),
    '#description' => t("Display a matching second date field as a 'End date'."),
  );
  $form['field']['advanced']['field_type'] = array(
    '#type' => 'select',
    '#options' => date_tools_wizard_field_types(),
    '#default_value' => 'datetime',
    '#title' => t('Date field type'),
    '#description' => t("The recommend type is Datetime, except for historical dates or dates with only year or month granularity. Older or incomplete dates should use the Date type (an ISO date)."),
  );
  $form['field']['advanced']['granularity'] = array(
    '#type' => 'select',
    '#options' => date_granularity_names(),
    '#default_value' => array('month', 'day', 'year', 'hour', 'minute'),
    '#title' => t('Granularity'),
    '#multiple' => TRUE,
  );
  $form['field']['advanced']['year_range'] = array(
    '#type' => 'textfield',
    '#default_value' => '-1:+1',
    '#title' => t('Year range'),
    '#description' => t("Range of allowed years, oldest to newest. '-1:+1 means oldest date is one year back, newest is one year forward from current year."),
  );
  $form['field']['advanced']['tz_handling'] = array(
    '#type' => 'select',
    '#options' => date_tools_wizard_tz_handling(),
    '#default_value' => 'site',
    '#title' => t('Date timezone handling'),
    '#description' => t("Timezone handling should be set to 'none' for granularity without time elements."),
  );
  $form['calendar'] = array(
    '#type' => 'select',
    '#default_value' => module_exists('calendar'),
    '#options' => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#title' => t('Create a calendar for this date field'),
    '#access' => module_exists('calendar'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Form validate.
 *
 * @todo.
 */
function date_tools_wizard_form_validate(&$form, &$form_state) {
  $bundle = $form_state['values']['bundle'];
  $field_name = 'field_' . $form_state['values']['field_name'];

  $args = array(
    ':field_name' => $field_name,
    ':bundle' => $bundle,
    ':entity_type' => 'node',
  );

  $query = "SELECT type FROM {node_type} WHERE type=:bundle";
  $existing_type = db_query($query, array(':bundle' => $args[':bundle']))->fetchField();

  $query = "SELECT field_name FROM {field_config_instance} WHERE field_name=:field_name AND bundle=:bundle AND entity_type=:entity_type";
  $existing_instance = db_query($query, $args)->fetchField();

  if ($existing_type) {
    backdrop_set_message(t('This content type name already exists, adding new field to existing content type.'));
  }
  if (!preg_match('!^[a-z0-9_]+$!', $bundle)) {
    form_set_error('bundle', t('The machine-readable name must contain only lowercase letters, numbers, and underscores.'));
  }
  if (!empty($form_state['values']['calendar']) && !empty($form_state['values']['blocks']) && strlen($bundle) > 12) {
    form_set_error('bundle', t('The content type name must be no more than 12 characters long when using it to create a calendar and blocks.'));
  }
  if ($existing_instance) {
    form_set_error('field_name', t('This field name already exists.'));
  }
  if (strlen($field_name) > 32) {
    form_set_error('field_name', t('The field name must be no more than 26 characters long.'));
  }
  if (!date_has_time($form_state['values']['granularity']) && $form_state['values']['tz_handling'] != 'none') {
    form_set_error('tz_handling', t('Timezone handling must be none for granularity without time.'));
  }
}

/**
 * Form submit.
 *
 * @todo.
 */
function date_tools_wizard_form_submit(&$form, &$form_state) {
  $view_name = date_tools_wizard_build($form_state['values']);
  menu_rebuild();
  if (!empty($form_state['values']['calendar']) && !empty($view_name)) {
    $form_state['redirect'] = 'admin/structure/views/template/' . $view_name . '/add';
  }
  else {
    $form_state['redirect'] = 'admin/structure/types/manage/' . str_replace('_', '-', $form_state['values']['bundle']) . '/fields';
  }
}

/**
 * Wizard build.
 *
 * @todo.
 */
function date_tools_wizard_build($form_values) {
  extract($form_values);

  $field_name = 'field_' . $field_name;
  $base_table = 'node';

  // Create a node type if it doesn't already exist.
  // This makes it possible to add additional date fields to
  // an existing type.
  $types = node_type_get_names();
  $type_settings = array();
  if (!array_key_exists($bundle, $types)) {
    date_tools_wizard_create_content_type($name, $bundle, $type_description, $type_settings);

    backdrop_set_message(t('Your content type @name has been created.', array('@name' => $name)));
  }
  else {
    $types = node_type_get_types();
    $type = $types[$bundle];
    if (!empty($type_settings)) {
      foreach ($type_settings as $key => $setting) {
        $type->$key = $setting;
      }
      node_type_save($type);
    }
    $name = $type->name;
  }

  $field = array(
    'field_name' => $field_name,
    'type' => $field_type,
    'cardinality' => $repeat ? FIELD_CARDINALITY_UNLIMITED : 1,
    'settings' => array(
      'granularity' => $granularity,
      'tz_handling' => $tz_handling,
      'timezone_db' => date_get_timezone_db($tz_handling),
      'repeat' => $repeat,
      'todate' => !empty($todate) ? $todate : 'optional',
    ),
  );
  $instance = array(
    'entity_type' => 'node',
    'field_name' => $field_name,
    'label' => $label,
    'bundle' => $bundle,
    // Move the date right below the title.
    'weight' => -4,
    'widget' => array(
      'type' => $widget_type,
      // Increment for minutes and seconds, can be 1, 5, 10, 15, or 30.
      'settings' => array(
        'increment' => 15,
        // The number of years to go back and forward in drop-down year
        // selectors.
        'year_range' => !empty($year_range) ? $year_range : '-0:+1',
        'input_format' => date_default_format($widget_type),
        'text_parts' => array(),
        'label_position' => 'above',
        'repeat_collapsed' => 0,
      ),
      'weight' => -4,
    ),
    'settings' => array(
      'default_value' => 'now',
      'default_value2' => 'blank',
    ),
  );

  $instance['display'] = array(
    'default' => array(
      'label' => 'above',
      'type' => 'date_default',
      'settings' => array(
        'format_type' => 'long',
        'show_repeat_rule' => 'show',
        'multiple_number' => '',
        'multiple_from' => '',
        'multiple_to' => '',
        'fromto' => 'both',
      ),
      'module' => 'date',
      'weight' => 0 ,
    ),
    'teaser' => array(
      'label' => 'above',
      'type' => 'date_default',
      'weight' => 0,
      'settings' => array(
        'format_type' => 'long',
        'show_repeat_rule' => 'show',
        'multiple_number' => '',
        'multiple_from' => '',
        'multiple_to' => '',
        'fromto' => 'both',
      ),
      'module' => 'date',
    ),
  );

  $field = field_create_field($field);
  $instance = field_create_instance($instance);
  $view_name = 'calendar_node_' . $field_name;

  field_info_cache_clear(TRUE);
  field_cache_clear(TRUE);

  backdrop_set_message(t('Your date field @name has been created.', array('@name' => $label)));

  return $view_name;
}

/**
 * Includes handler.
 *
 * @todo.
 */
function date_tools_wizard_include() {
  module_load_include('inc', 'node', 'content_types');
  module_load_include('inc', 'node', 'node.pages');
  module_load_include('inc', 'field', 'field.crud');
  module_load_include('inc', 'date', 'date_admin');
}

/**
 * Implements hook_field_types().
 *
 * @todo.
 */
function date_tools_wizard_field_types() {
  $field_types = array();
  foreach (date_field_info() as $name => $info) {
    $field_types[$name] = $info['label'];
  }
  return $field_types;
}

/**
 * Implements hook_widget_types().
 * @todo.
 */
function date_tools_wizard_widget_types() {
  $widget_types = array();
  foreach (date_field_widget_info() as $name => $info) {
    if (!strstr($name, '_repeat')) {
      $widget_types[$name] = $info['label'];
    }
  }
  return $widget_types;
}

/**
 * Tz handler.
 *
 * @todo.
 */
function date_tools_wizard_tz_handling() {
  include_once backdrop_get_path('module', 'date') . '/date_admin.inc';
  return date_timezone_handling_options();
}

/**
 * Create date tools wizard content type.
 *
 * @todo.
 */
function date_tools_wizard_create_content_type($name, $bundle, $description, $type_settings = array()) {

  date_tools_wizard_include();

  // Create the content type.
  $values  = array(
    'name' => $name,
    'type' => $bundle,
    'description' => $description,
    'title_label' => 'Title',
    'body_label' => 'Body',
    'min_word_count' => '0',
    'help' => '',
    'node_options' => array(
      'status' => 1,
      'promote' => 1,
      'sticky' => 0,
      'revision' => 0,
    ),
    'language_content_type' => '0',
    'old_type' => $bundle,
    'orig_type' => '',
    'base' => 'node_content',
    'custom' => '1',
    'modified' => '1',
    'locked' => '0',
    'url_str' => str_replace('_', '-', $bundle),
  );

  // Allow overrides of these values.
  foreach ($type_settings as $key => $value) {
    $values[$key] = $value;
  }

  node_type_save((object) $values);

  // Add the body field.
  $trim_length = config_get('date.settings','teaser_length');

  $field = field_info_field('body');
  $instance = array(
    'field_name' => 'body',
    'entity_type' => 'node',
    'bundle' => $bundle,
    'label' => t('Description'),
    'widget' => array(
      'type' => 'text_textarea_with_summary',
      'settings' => array(
        'rows' => 20,
        'summary_rows' => 5,
      ),
      'weight' => -4,
      'module' => 'text',
    ),
    'settings' => array(
      'display_summary' => TRUE,
    ),
    'display' => array(
      'default' => array(
        'label' => 'hidden',
        'type' => 'text_default',
      ),
      'teaser' => array(
        'label' => 'hidden',
        'type' => 'text_summary_or_trimmed',
        'trim_length' => $trim_length,
      ),
    ),
  );
  field_create_instance($instance);

}
