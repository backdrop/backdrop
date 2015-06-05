<?php

/**
 * @file
 * Support for processing entity reference fields in devel generate.
 */

function entityreference_devel_generate($object, $field, $instance, $bundle) {
  if (field_behaviors_widget('multiple values', $instance) == FIELD_BEHAVIOR_CUSTOM) {
    return devel_generate_multiple('_entityreference_devel_generate', $object, $field, $instance, $bundle);
  }
  else {
    return _entityreference_devel_generate($object, $field, $instance, $bundle);
  }
}

function _entityreference_devel_generate($object, $field, $instance, $bundle) {
  $object_field = array();
  // Get all the entity that are referencable here.
  $referencable_entity = entityreference_get_selection_handler($field, $instance)->getReferencableEntities();
  if (is_array($referencable_entity) && !empty($referencable_entity)) {
    // $referencable_entity is keyed by bundle type.
    $random_bundle = array_rand($referencable_entity);
    if (!empty($random_bundle)) {
      $target_id = array_rand($referencable_entity[$random_bundle]);
      if (!empty($referencable_entity[$random_bundle][$target_id])) {
        $object_field['target_id'] = $target_id;
      }
    }
  }
  return $object_field;
}
