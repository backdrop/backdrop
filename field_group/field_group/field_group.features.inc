<?php
// $Id$

/**
 * @file
 * Features integration for field_group.
 */

/**
 * Implements hook_features_export_options().
 */
function field_group_features_export_options() {
  $options = array();
  $field_group_instances = field_group_info_groups();
  foreach ($field_group_instances as $entity_type => $bundles) {
    foreach ($bundles as $bundle => $view_modes) {
      foreach ($view_modes as $view_mode => $field_groups) {
        foreach ($field_groups as $field_group) {
          $identifier = "{$entity_type}-{$bundle}-{$view_mode}-{$field_group->group_name}";
          $options[$identifier] = $identifier;
        }
      }
    }
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function field_group_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  $map = features_get_default_map('field_group');

  // The field_default_fields() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  foreach ($data as $identifier) {
    if ($field_group = features_field_group_load($identifier)) {
      // If this field is already provided by another module, remove the field
      // and add the other module as a dependency.
      // @todo check this ..
      if (isset($map[$identifier]) && $map[$identifier] != $module_name) {
        if (isset($export['features']['field'][$identifier])) {
          unset($export['features']['field'][$identifier]);
        }
        $module = $map[$identifier];
        $export['dependencies'][$module] = $module;
      }
      // If the field has not yet been exported, add it
      else {
        $export['features']['field_group'][$identifier] = $identifier;
        $export['dependencies']['field_group'] = 'field_group';
      }
    }
  }
}

/**
 * Implementation of hook_features_export_render().
 */
function field_group_features_export_render($module, $data, $export = NULL) {
  $translatables = $code = array();

  $code[] = '  $field_groups = array();';
  $code[] = '';
  foreach ($data as $identifier) {
    if ($field_group = features_field_group_load($identifier)) {
      unset($field_group->id);

      $field_group_export = features_var_export($field_group, '  ');
      // @todo check this name.
      $field_group_identifier = features_var_export($identifier);
      $code[] = "  // Exported field_group: {$field_group_identifier}";
      $code[] = "  \$field_groups[{$field_group_identifier}] = {$field_group_export};";
      $code[] = "";

      // Add any labels to translatables array.
      // @todo check this.
      /*if (!empty($field_info['widget']['label'])) {
        $translatables[] = $field_info['widget']['label'];
      }*/
    }
  }
  // @todo translatables ?
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }
  $code[] = '  return $field_groups;';
  $code = implode("\n", $code);
  return array('field_group_default_field_groups' => $code);
}

/**
 * Implements hook_features_revert().
 */
function field_group_features_revert($module) {
  field_group_features_rebuild($module);
}

/**
 * Implements of hook_features_rebuild().
 * Rebuilds fields from code defaults.
 */
function field_group_features_rebuild($module) {
  if ($field_groups = features_get_default('field_group', $module)) {
    cache_clear_all('field_groups', 'cache_field');

    // Get existing field groups to be able to know if we should create or
    // update when rebuilding.
    $existing_field_groups = field_group_read_groups();

    foreach ($field_groups as $field_group) {
      $field_group = (object) $field_group;
      // Check to see if the field group already exists.
      if (isset($existing_field_groups[$field_group->entity_type][$field_group->bundle][$field_group->mode][$field_group->group_name])) {
        $field_group->id = $existing_field_groups[$field_group->entity_type][$field_group->bundle][$field_group->mode][$field_group->group_name]->id;
      }
      field_group_save($field_group);
    }
  }
}

/**
 * Load a field_group's configuration and instance configuration by an
 * entity_type-bundle-view_mode-field_name identifier.
 */
function features_field_group_load($identifier) {
  $all_field_groups = field_group_info_groups();
  list($entity_type, $bundle, $view_mode, $field_group_name) = explode('-', $identifier);
  if (isset($all_field_groups[$entity_type][$bundle][$view_mode][$field_group_name])) {
    return $all_field_groups[$entity_type][$bundle][$view_mode][$field_group_name];
  }
  return FALSE;
}

