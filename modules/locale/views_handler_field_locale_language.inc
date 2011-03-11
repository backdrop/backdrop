<?php

/**
 * Field handler to translate a language into its readable form.
 */
class views_handler_field_locale_language extends views_handler_field {
  function render($values) {
    $languages = language_list();
    $value = $this->get_value($values);
    return isset($languages[$value]) ? $languages[$value]->name : '';
  }
}
