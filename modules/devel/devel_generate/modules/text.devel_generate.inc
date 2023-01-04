<?php
/**
 * @file
 * Devel generate support for Text module.
 */

function text_devel_generate($object, $field, $instance, $bundle) {
  if (field_behaviors_widget('multiple values', $instance) == FIELD_BEHAVIOR_CUSTOM) {
    return devel_generate_multiple('_text_devel_generate', $object, $field, $instance, $bundle);
  }
  else {
    return _text_devel_generate($object, $field, $instance, $bundle);
  }
}

function _text_devel_generate($object, $field, $instance, $bundle) {
  $object_field = array();

  $formats = filter_formats();
  if (!empty($instance['settings']['text_processing'])) {
    $format_name = array_rand($formats);
  }
  else {
    $format_name = filter_fallback_format();
  }
  $format = $formats[$format_name];

  // If escaping all tags, do not add <p> tags.
  $include_p_tags = TRUE;
  if (array_key_exists('filter_html_escape', $format->filters)) {
    $include_p_tags = FALSE;
  }

  if ($instance['widget']['type'] != 'text_textfield') {
    // Textarea handling
    $object_field['value'] = devel_create_content($include_p_tags);
    if ($instance['widget']['type'] == 'text_textarea_with_summary' && !empty($instance['display_summary'])) {
      $object_field['summary'] = devel_create_content($include_p_tags);
    }
  }
  else {
    // Textfield handling.
    // Generate a value that respects max_length.
    if (empty($field['settings']['max_length'])) {
      $field['settings']['max_length'] = 12;
    }
    $object_field['value'] = user_password($field['settings']['max_length']);
  }
  $object_field['format'] = $format_name;
  return $object_field;
}
