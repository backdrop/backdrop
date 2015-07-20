<?php

/**
 * @file
 * A form to change the type of date used in date fields.
 */

/**
 * Form constructor for the date type change form.
 *
 * @see date_tools_change_type_form_validate()
 * @see date_tools_change_type_form_submit()
 */
function date_tools_change_type_form() {
  $form = array();
  // This is broken, still needs to be adjusted for the D6->D7 changes.
  backdrop_set_message(t('This operation does not yet work for the Drupal 7 version.'), 'error');
  return $form;
  $fields = content_fields();
  $date_options = array();
  $type_options = array();

  $labels = array();
  foreach (date_field_info() as $type => $info) {
    $type_options[$type] = $info['label'] . ': ' . $info['description'];
    $labels[$type] = $info['label'];
  }
  // Get the available date fields.
  foreach ($fields as $field_name => $field) {
    if ($field['type'] == 'date' || $field['type'] == 'datestamp' || $field['type'] == 'datetime') {
      $date_options[$labels[$field['type']]][$field_name] = t('Field @label (@field_name)', array(
        '@label' => $field['widget']['label'],
        '@field_name' => $field_name,
        '@type' => $labels[$field['type']]
      ));
    }
  }
  if (count($date_options) < 1) {
    backdrop_set_message(t('There are no date fields in this database.'));
    return $form;
  }
  $form['date_field'] = array(
    '#type' => 'select',
    '#options' => $date_options,
    '#title' => t('Date field'),
    '#default_value' => '',
    '#description' => t('The date field which whose type should be changed.'),
  );
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => $type_options,
    '#default_value' => '',
    '#required' => TRUE,
    '#description' => t('The type of date to change the field to.'),
    '#prefix' => '<strong>' . t('New type:') . '</strong>',
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Change'));
  return $form;
}

/**
 * Form validation handler for date_tools_change_type_form().
 *
 * @see date_tools_change_type_form_submit()
 */
function date_tools_change_type_form_validate($form, &$form_state) {
  $field_name = $form_state['values']['date_field'];
  $new_type = $form_state['values']['type'];
  $field = content_fields($field_name);
  $old_type = $field['type'];
  if ($new_type == $old_type) {
    form_set_error('type', t('The current type is the same as the chosen type. There is nothing to change.'));
  }
}

/**
 * Form submission handler for date_tools_change_type_form().
 *
 * @see date_tools_change_type_form_validate()
 */
function date_tools_change_type_form_submit($form, &$form_state) {
  $field_name = $form_state['values']['date_field'];
  $new_type = $form_state['values']['type'];
  $field = content_fields($field_name);
  $old_type = $field['type'];
  if ($new_type == $old_type) {
    return;
  }
  $db_info = content_database_info($field);
  $table = $db_info['table'];
  $columns = $db_info['columns'];
  $labels = array();
  foreach (date_field_info() as $type => $info) {
    $labels[$type] = $info['label'];
  }

  // Is there any data in this field? If not, we can
  // skip some steps.
  $has_data = db_query("SELECT COUNT(*) FROM {" . $table . "}")->fetchField();

  // Create a backup copy of the original values.
  // The values are going to get corrupted when we
  // change the column type.
  if ($has_data) {
    $temp_table = $table . '_temp';
    db_query("CREATE TABLE {" . $temp_table . "} SELECT * FROM {" . $table . "}");
  }

  // Change the field definition to the new type.
  $field['type'] = $new_type;
  require_once './' . backdrop_get_path('module', 'content') . '/includes/content.crud.inc';
  content_field_instance_update($field, FALSE);
  content_clear_type_cache();

  // If there's no data to update, we're finished.
  if (!$has_data) {
    backdrop_set_message(t('The field @field_name has been changed from @old_type to @new_type.', array(
    '@field_name' => $field['widget']['label'], '@old_type' => $labels[$old_type], '@new_type' => $labels[$new_type])));
    return;
  }

  // Replace old values with modified values, massaging the original values as
  // necessary for the new type.
  require_once './' . backdrop_get_path('module', 'date_api') . '/date_api_sql.inc';
  $date_handler = new date_sql_handler();
  $date_handler->granularity = $field['granularity'];
  $date_handler->date_type = $old_type;

  $new_columns = array();
  $old_columns = array('nid', 'vid');
  $new_columns[] = $temp_table . '.nid AS nid';
  $new_columns[] = $temp_table . '.vid AS vid';
  if ($field->multiple) {
    $new_columns[] = $temp_table . '.delta AS delta';
    $old_columns[] = 'delta';
  }
  foreach ($columns as $column => $info) {
    if ($column != 'value' && $column != 'value2') {
      continue;
    }
    $old_columns[] = $info['column'];
    $db_field = $date_handler->sql_field($temp_table . '.' . $info['column'], 0);
    switch ($old_type) {
      case 'date':
        switch ($new_type) {
          case 'datestamp':
            $new_columns[] = $date_handler->sql_format('U', $db_field) . ' AS ' . $info['column'];
            break;

          case 'datetime':
            $new_columns[] = $date_handler->sql_format('Y-m-d H:i:s', $db_field) . ' AS ' . $info['column'];
            break;
        }
        break;

      case 'datestamp':
        switch ($new_type) {
          case 'date':
            $new_columns[] = $date_handler->sql_format('Y-m-d/TH:i:s', $db_field) . ' AS ' . $info['column'];
            break;

          case 'datetime':
            $new_columns[] = $date_handler->sql_format('Y-m-d H:i:s', $db_field) . ' AS ' . $info['column'];
            break;
        }
        break;

      case 'datetime':
        switch ($new_type) {
          case 'date':
            $new_columns[] = $date_handler->sql_format('Y-m-d/TH:i:s', $db_field) . ' AS ' . $info['column'];
            break;

          case 'datestamp':
            $new_columns[] = $date_handler->sql_format('U', $db_field) . ' AS ' . $info['column'];
            break;
        }
        break;
    }
  }

  // Make sure database timezone is set to UTC.
  $date_handler->set_db_timezone();

  // Make the replacement.
  $sql = 'REPLACE INTO {' . $table . '} (' . implode(', ', $old_columns) . ') ' . ' SELECT ' . implode(', ', $new_columns) . ' FROM {' . $temp_table . '}';
  db_query($sql);
  db_query("DROP TABLE {" . $temp_table . "}");

  backdrop_set_message(t('The field @field_name has been changed from @old_type to @new_type.', array(
    '@field_name' => $field['widget']['label'],
    '@old_type' => $labels[$old_type],
    '@new_type' => $labels[$new_type]
  )));
}
