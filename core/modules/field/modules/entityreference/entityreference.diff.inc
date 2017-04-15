<?php

/**
 * @file
 * Provide Diff module field callbacks for the Entity Reference module.
 */

/**
 * Diff field callback for preloading entities.
 */
function entityreference_field_diff_view_prepare(&$old_items, &$new_items, $context) {
  $field = $context['field'];

  // Build an array of entities ID.
  $entity_ids = array();
  foreach (array_merge_recursive($old_items, $new_items) as $item) {
    $entity_ids[] = $item['target_id'];
  }

  // Load those entities and loop through them to extract their labels.
  $entities = entity_load($field['settings']['target_type'], $entity_ids);

  foreach ($old_items as $delta => $info) {
    $old_items[$delta]['entity'] = isset($entities[$info['target_id']]) ? $entities[$info['target_id']] : NULL;
  }
  foreach ($new_items as $delta => $info) {
    $new_items[$delta]['entity'] = isset($entities[$info['target_id']]) ? $entities[$info['target_id']] : NULL;
  }
}

/**
 * Diff field callback for parsing entity field comparative values.
 */
function entityreference_field_diff_view($items, $context) {
  $field = $context['field'];
  $instance = $context['instance'];
  $settings = $context['settings'];
  $entity_type = $field['settings']['target_type'];

  $diff_items = array();

  // We populate as much as possible to allow the best flexability in any
  // string overrides.
  $t_args = array();
  $t_args['!entity_type'] = $entity_type;

  $entity_info = entity_get_info($entity_type);
  $t_args['!entity_type_label'] = $entity_info['label'];

  foreach ($items as $delta => $item) {
    if (isset($item['entity'])) {
      $output = array();

      list($id,, $bundle) = entity_extract_ids($entity_type, $item['entity']);
      $t_args['!id'] = $id;
      $t_args['!bundle'] = $bundle;
      $t_args['!diff_entity_label'] = entity_label($entity_type, $item['entity']);

      $output['entity'] = t('!diff_entity_label', $t_args);
      if ($settings['show_id']) {
        $output['id'] = t('ID: !id', $t_args);
      }
      $diff_items[$delta] = implode('; ', $output);
    }
  }

  return $diff_items;
}

/**
 * Provide default field comparison options.
 */
function entityreference_field_diff_default_options($field_type) {
  return array(
    'show_id' => 0,
  );
}

/**
 * Provide a form for setting the field comparison options.
 */
function entityreference_field_diff_options_form($field_type, $settings) {
  $options_form = array();
  $options_form['show_id'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show ID'),
    '#default_value' => $settings['show_id'],
  );
  return $options_form;
}
