<?php
/**
 * @file
 * Returns a pure file extension of the file, for example 'module'.
 */

class views_handler_field_file_extension extends views_handler_field {
  function render($values) {
    $value = $this->get_value($values);
    if (preg_match('/\.([^\.]+)$/', $value, $match)) {
      return $this->sanitize_value($match[1]);
    }
  }
}
