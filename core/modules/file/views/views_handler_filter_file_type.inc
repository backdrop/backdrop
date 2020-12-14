<?php

/**
 * @file
 * Definition of views_handler_filter_file_type.
 */

/**
 * Filter by file type
 */
class views_handler_filter_file_type extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('File types');
      $options = array();
      foreach (file_type_get_names() as $type => $name) {
        $options[$type] = t($name);
      }
      asort($options);
      $this->value_options = $options;
    }
  }
}
