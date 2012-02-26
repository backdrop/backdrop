<?php

/**
 * @file
 * Views integration for Entity Reference.
 */

/**
 * Implements hook_field_views_data().
 */
function entityreference_field_views_data($field) {
  $data = field_views_field_default_views_data($field);
  $entity_info = entity_get_info($field['settings']['target_type']);
  foreach ($data as $table_name => $table_data) {
    if (isset($entity_info['base table'])) {
      $entity = $entity_info['label'];
      if ($entity == t('Node')) {
        $entity = t('Content');
      }

      $field_name = $field['field_name'] . '_target_id';
      $parameters = array('@entity' => $entity, '!field_name' => $field['field_name']);
      $data[$table_name][$field_name]['relationship'] = array(
        'handler' => 'views_handler_relationship',
        'base' => $entity_info['base table'],
        'base field' => $entity_info['entity keys']['id'],
        'label' => t('@entity entity referenced from !field_name', $parameters),
        'group' => t('Entity Reference'),
        'title' => t('Referenced Entity'),
        'help' => t('A bridge to the @entity entity that is referenced via !field_name', $parameters),
      );
    }
  }

  // Invoke the behaviors to allow them to change the properties.
  foreach (entityreference_get_behavior_handlers($field) as $handler) {
    $handler->views_data_alter($data, $field);
  }

  return $data;
}

/**
 * Options callback for Views handler views_handler_filter_in_operator.
 */
function entityreference_views_handler_options_list($field_name) {
  $field = field_info_field($field_name);
  return entityreference_options_list($field);
}

/**
 * Implements hook_field_views_data_views_data_alter().
 *
 * Views integration to provide reverse relationships on entityreference fields.
 */
function entityreference_field_views_data_views_data_alter(&$data, $field) {
  foreach ($field['bundles'] as $entity_type => $bundles) {
    $target_entity_info = entity_get_info($field['settings']['target_type']);
    if (isset($target_entity_info['base table'])) {
      $entity_info = entity_get_info($entity_type);
      $entity = $entity_info['label'];
      if ($entity == t('Node')) {
        $entity = t('Content');
      }
      $target_entity = $target_entity_info['label'];
      if ($target_entity == t('Node')) {
        $target_entity = t('Content');
      }

      $pseudo_field_name = 'reverse_' . $field['field_name'] . '_' . $entity_type;
      $replacements = array('@entity' => $entity, '@target_entity' => $target_entity, '!field_name' => $field['field_name']);
      $data[$target_entity_info['base table']][$pseudo_field_name]['relationship'] = array(
        'handler' => 'views_handler_relationship_entity_reverse',
        'field_name' => $field['field_name'],
        'field table' => _field_sql_storage_tablename($field),
        'field field' => $field['field_name'] . '_target_id',
        'base' => $entity_info['base table'],
        'base field' => $entity_info['entity keys']['id'],
        'label' => t('@entity referencing @target_entity from !field_name', $replacements),
        'group' => t('Entity Reference'),
        'title' => t('Referencing entity'),
        'help' => t('A bridge to the @entity entity that is referencing @target_entity via !field_name', $replacements),
      );
    }
  }
}
