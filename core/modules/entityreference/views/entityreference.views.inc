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
        'label' => t('@entity referenced from !field_name', $parameters),
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
      if (empty($entity_info)) {
        watchdog(
          'entityreference',
          'Field %field_name refers to nonexistent entity type %type. You might want to remove this broken field instance. <a href="https://www.drupal.org/docs/7/modules/views/data-fix-field-field-refers-to-nonexistent-entity-type-type">See documentation.</a>',
          array('%field_name' => $field['field_name'], '%type' => $entity_type),
          WATCHDOG_WARNING
        );
        continue;
      }
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
        'join_extra' => array(
          0 => array(
            'field' => 'entity_type',
            'value' => $entity_type,
          ),
          1 => array(
            'field' => 'deleted',
            'value' => 0,
            'numeric' => TRUE,
          ),
        ),
      );
    }
  }
}

/**
 * Implements hook_views_plugins().
 */
function entityreference_views_plugins() {
  $plugins = array(
    'display' => array(
      'entityreference' => array(
        'title' => t('Entity Reference'),
        'admin' => t('Reference Source'),
        'help' => 'Selects referenceable entities for a reference field',
        'handler' => 'entityreference_plugin_display',
        'uses hook menu' => FALSE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
        // Custom property, used with views_get_applicable_views() to retrieve
        // all views with a 'Entity Reference' display.
        'entityreference display' => TRUE,
      ),
    ),
    'style' => array(
      'entityreference_style' => array(
        'title' => t('Entity Reference list'),
        'help' => 'Returns results as a list of labels and rendered rows.',
        'handler' => 'entityreference_plugin_style',
        'theme' => 'views_view_unformatted',
        'theme path' => backdrop_get_path('module', 'views') . '/templates',
        'theme file' => 'theme.inc',
        'uses row plugin' => TRUE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'entityreference',
        'even empty' => TRUE,
      ),
    ),
    'row' => array(
      'entityreference_fields' => array(
        'title' => t('Inline fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'entityreference_plugin_row_fields',
        'theme' => 'views_view_fields',
        'theme path' => backdrop_get_path('module', 'views') . '/templates',
        'theme file' => 'theme.inc',
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'entityreference',
      ),
    ),
  );
  return $plugins;
}

/**
 * Element validation callback for EntityReferenceSelectionHandlerViews.
 */
function entityreference_view_settings_validate($element, &$form_state, $form) {
  // Split view name and display name from the 'view_and_display' value.
  if (!empty($element['view_and_display']['#value'])) {
    list($view, $display) = explode(':', $element['view_and_display']['#value']);
  }
  else {
    form_error($element, t('The views entity selection mode requires a view.'));
    return;
  }

  // Explode the 'args' string into an actual array. Beware, explode() turns an
  // empty string into an array with one empty string. We'll need an empty array
  // instead.
  $args_string = trim($element['args']['#value']);
  if ($args_string === '') {
    $args = array();
  }
  else {
    // array_map is called to trim whitespaces from the arguments.
    $args = array_map('trim', explode(',', $args_string));
  }

  $value = array('view_name' => $view, 'display_name' => $display, 'args' => $args);
  form_set_value($element, $value, $form_state);
}
