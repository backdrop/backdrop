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
    // Get a random key.
    foreach ($referencable_entity as $type => $eids) {
      $object_field['target_id'] = array_rand($eids);
    }
  }
  return $object_field;
}
