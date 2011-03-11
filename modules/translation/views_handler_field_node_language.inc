<?php

/**
 * Field handler to translate a language into its readable form.
 */
class views_handler_field_node_language extends views_handler_field_node {
  function render($values) {
    $languages = locale_language_list();
    $value = $this->get_value($values);
    $value = isset($languages[$value]) ? $languages[$value] : '';
    return $this->render_link($value, $values);
  }
}
