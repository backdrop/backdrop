<?php
// $Id$

/**
 * @file
 * Provide Views data and handlers for field.module
 */

/**
 * @defgroup views_field_module field.module handlers
 *
 * @{
 */

/**
 * Implements hook_views_data()
 */
function field_views_data() {
  $data = array();
  foreach (field_info_fields() as $field) {
    if ($field['storage']['type'] != 'field_sql_storage') {
      continue;
    }

    $module = $field['module'];
    $result = (array) module_invoke($module, 'field_views_data', $field);
    drupal_alter('field_views_data', $result, $field);

    if (empty($result)) {
      $result = field_views_field_default_views_data($field);
    }
    if (is_array($result)) {
      $data = array_merge($data, $result);
    }
  }

  return $data;
}

/**
 * Default views data implementation for a field.
 */
function field_views_field_default_views_data($field) {
  $field_types = field_info_field_types();

  // Check the field module is available.
  if (!isset($field_types[$field['type']])) {
    return;
  }

  // Note: this should be in the $field object already, see http://drupal.org/node/645926.
  $field_storage_details = field_sql_storage_field_storage_details($field, NULL);
  $field += $field_storage_details;

  $data = array();

  $current_table = _field_sql_storage_tablename($field);
  $revision_table = _field_sql_storage_revision_tablename($field);

  // The list of entity:bundle that this field is used in.
  $bundles_names = array();
  $supports_revisions = FALSE;

  // Build the relationships between the field table and the entity tables.
  foreach ($field['bundles'] as $entity => $bundles) {
    $entity_info = entity_get_info($entity);

    $data[$current_table]['table']['join'][$entity_info['base table']] = array(
      'left_field' => $entity_info['object keys']['id'],
      'field' => 'entity_id',
      'extra' => array(array('field' => 'etid', 'value' => _field_sql_storage_etid($entity), 'numeric' => TRUE)),
    );

    if (!empty($entity_info['object keys']['revision']) && !empty($entity_info['revision table'])) {
      $data[$revision_table]['table']['join'][$entity_info['revision table']] = array(
        'left_field' => $entity_info['object keys']['revision'],
        'field' => 'revision_id',
        'extra' => array(array('field' => 'etid', 'value' => _field_sql_storage_etid($entity), 'numeric' => TRUE)),
      );

      $supports_revisions = TRUE;
    }

    foreach ($bundles as $bundle) {
      $bundles_names[] = t('@entity:@bundle', array('@entity' => $entity, '@bundle' => $bundle));
    }
  }

  $tables = array();
  $tables[FIELD_LOAD_CURRENT] = $current_table;
  if ($supports_revisions) {
    $tables[FIELD_LOAD_REVISION] = $revision_table;
  }

  // Add the field handler for this field.
  $title_short = $field['field_name'];
  foreach ($tables as $type => $table) {
    if ($type == FIELD_LOAD_CURRENT) {
      $group = t('Fields');
      $column = 'entity_id';
    }
    else {
      $group = t('Fields (historical data)');
      $column = 'revision_id';
    }

    $data[$table][$column] = array(
      'group' => $group,
      'title' => $title_short,
      'title short' => $title_short,
      'help' =>  t('Appears in: @bundles', array('@bundles' => implode(', ', $bundles_names))),
    );
    $data[$table][$column]['field'] = array(
      'field' => $column,
      'table' => $table,
      'handler' => 'views_handler_field_field',
      'click sortable' => TRUE,
      'field_name' => $field['field_name'],
      'additional fields' => array('etid'),
    );
  }

  foreach ($field['columns'] as $column => $attributes) {
    $sort = !empty($attributes['sortable']) ? TRUE : FALSE;

    // Identify likely filters and arguments for each column based on field type.
    switch ($attributes['type']) {
      case 'int':
      case 'mediumint':
      case 'tinyint':
      case 'bigint':
      case 'serial':
        $filter = 'views_handler_filter_numeric';
        $argument = 'views_handler_argument_numeric';
        break;
      case 'numeric':
      case 'float':
        $filter = 'views_handler_filter_float';
        $argument = 'views_handler_argument_numeric';
        break;

      case 'text':
      case 'blob':
        // TODO add markup handlers for these types
      default:
        $filter = 'views_handler_filter_string';
        $argument = 'views_handler_argument_string';
        break;
    }

    // Note: we don't have a label available here, because we are at the field
    // level, not at the instance level.
    if (count($field['columns']) == 1) {
      $title = t('@label (!name)', array('@label' => $field['field_name'], '!name' => $field['field_name']));
      $title_short = $field['field_name'];
    }
    else {
      $title = t('@label (!name) - !column', array('@label' => $field['field_name'], '!name' => $field['field_name'], '!column' => $column));
      $title_short = t('@label-truncated - !column', array('@label-truncated' => $field['field_name'], '!column' => $column));
    }

    foreach ($tables as $type => $table) {
      $group = $type == FIELD_LOAD_CURRENT ? t('Fields') : t('Fields (historical data)');
      $column_real_name = $field['sql'][$type][$table][$column];

      // Load all the fields from the table by default.
      $additional_fields = array_values($field['sql'][$type][$table]);

      $data[$table][$column_real_name] = array(
        'group' => $group,
        'title' => $title,
        'title short' => $title_short,
        'help' =>  t('Appears in: @bundles', array('@bundles' => implode(', ', $bundles_names))),
      );

      $data[$table][$column_real_name]['argument'] = array(
        'field' => $column_real_name,
        'table' => $table,
        'handler' => $argument,
        'additional fields' => $additional_fields,
        'content_field_name' => $field['field_name'],
        'empty field name' => t('<No value>'),
      );
      $data[$table][$column_real_name]['filter'] = array(
        'field' => $column_real_name,
        'table' => $table,
        'handler' => $filter,
        'additional fields' => $additional_fields,
        'content_field_name' => $field['field_name'],
        'allow empty' => TRUE,
      );
      if (!empty($sort)) {
        $data[$table][$column_real_name]['sort'] = array(
          'field' => $column_real_name,
          'table' => $table,
          'handler' => 'content_handler_sort',
          'additional fields' => $additional_fields,
          'content_field_name' => $field['field_name'],
        );
      }

      // Expose additional delta column for multiple value fields.
      if ($field['cardinality'] > 1 || $field['cardinality'] == FIELD_CARDINALITY_UNLIMITED) {
        $title = t('@label (!name) - delta', array('@label' => $field['field_name'], '!name' => $field['field_name']));
        $title_short = t('!name - delta', array('@name' => $field['field_name']));

        $data[$table]['delta'] = array(
          'group' => $group,
          'title' => $title,
          'title short' => $title_short,
          'help' => t('Delta - Appears in: @bundles', array('@bundles' => implode(', ', $bundles_names))),
        );
        $data[$table]['delta']['argument'] = array(
          'field' => 'delta',
          'table' => $table,
          'handler' => 'views_handler_argument_numeric',
          'additional fields' => $additional_fields,
          'empty field name' => t('<No value>'),
        );
        $data[$table]['delta']['filter'] = array(
          'field' => 'delta',
          'table' => $table,
          'handler' => 'views_handler_filter_numeric',
          'additional fields' => $additional_fields,
          'allow empty' => TRUE,
        );
        $data[$table]['delta']['sort'] = array(
          'field' => 'delta',
          'table' => $table,
          'handler' => 'views_handler_sort',
          'additional fields' => $additional_fields,
        );
      }
    }
  }

  return $data;
}

/**
 * Implements hook_views_handlers().
 */
function field_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/field',
    ),
    'handlers' => array(
      'views_handler_field_field' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}


/**
 * @}
 */
