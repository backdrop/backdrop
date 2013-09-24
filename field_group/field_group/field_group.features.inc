<?php


/**
 * Implements hook_features_export_alter().
 *
 * For a given feature, add field groups that contain any fields that
 * are a part of this feature.  Also, add parent groups of any groups
 * that are a part of this feature.
 */
function field_group_features_export_alter(&$export, $module_name) {
  // Make sure we have fresh data by loading directly.
  ctools_include('export');
  $field_groups = ctools_export_load_object('field_group');

  // Support the separate field base -vs- field instance structure that was
  // added in Features v7.x-2.0-beta2.
  if (function_exists('field_instance_features_export')) {
    $export_var = 'field_instance';
  }
  else {
    $export_var = 'field';
  }

  // Add fieldgroups based on the fields that are present.
  if (!empty($export['features'][$export_var])) {
    if (!isset($export['features']['field_group'])) {
      $export['features']['field_group'] = array();
    }
    foreach ($export['features'][$export_var] as $field) {
      list($entity_type, $bundle, $field_name) = explode('-', $field);

      foreach ($field_groups as $group_id => $group) {

        if ($group->entity_type == $entity_type && $group->bundle == $bundle && in_array($field_name, $group->data['children']) && !in_array($group->identifier, $export['features']['field_group'])) {
          if (isset($group->export_module) && $group->export_module != $module_name) {
            $export['dependencies'][$group->export_module] = $group->export_module;
          }
          else {
            $export['features']['field_group'][$group_id] = $group_id;
          }
        }
      }
    }
  }

  // Add any parent field groups that haven't been selected.
  if (!empty($export['features']['field_group'])) {
    foreach ($export['features']['field_group'] as $id) {
      $group = isset($field_groups[$id]) ? $field_groups[$id] : FALSE;

      if ($group && !empty($group->parent_name)) {
        $parent_id = $group->parent_name . '|' . $group->entity_type . '|' . $group->bundle . '|' . $group->mode;
        $parent_group = isset($field_groups[$parent_id]) ? $field_groups[$parent_id] : FALSE;

        if ($parent_group && !isset($export['features']['field_group'][$parent_id])) {
          if (isset($parent_group->export_module) && $parent_group->export_module != $module_name) {
            $export['dependencies'][$parent_group->export_module] = $parent_group->export_module;
          }
          else {
            $export['features']['field_group'][$parent_id] = $parent_id;
          }
        }
      }
    }
    if(empty($export['dependencies']['field_group'])) {
      $export['dependencies']['field_group'] = 'field_group';
    }
  }
}
