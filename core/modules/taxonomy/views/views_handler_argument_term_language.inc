<?php

/**
 * @file
 * Definition of views_handler_argument_term_language.
 */

/**
 * Argument handler to accept a language.
 */
class views_handler_argument_term_language extends views_handler_argument {
  function construct() {
    parent::construct('langcode');
  }

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the language.
   */
  function summary_name($data) {
    return $this->term_language($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * node type.
   */
  function title() {
    return $this->term_language($this->argument);
  }

  function term_language($langcode) {
    $languages = views_language_list();
    return isset($languages[$langcode]) ? $languages[$langcode] : t('Unknown language');
  }
}
