<?php

/**
 * @file
 * Devel Generate support for Link module.
 */

/**
 * Implements hook_devel_generate().
 */
function link_devel_generate($object, $field, $instance, $bundle) {
  if (field_behaviors_widget('multiple values', $instance) == FIELD_BEHAVIOR_CUSTOM) {
    return devel_generate_multiple('_link_devel_generate', $object, $field, $instance, $bundle);
  }
  else {
    return _link_devel_generate($object, $field, $instance, $bundle);
  }
}

/**
 * Callback for hook_devel_generate().
 */
function _link_devel_generate($object, $field, $instance, $bundle) {
  $link = array(
    'url' => url('<front>', array('absolute' => TRUE)),
    'attributes' => _link_default_attributes(),
  );
  if ($instance['settings']['title'] != 'none') {
    $link['title'] = devel_create_greeking(mt_rand(1, 3), TRUE);
  }
  return $link;
}
