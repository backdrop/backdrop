<?php

/**
 * @file
 * Definition of views_plugin_localization_core.
 */

/**
 * Localization plugin to pass translatable strings through t().
 *
 * @ingroup views_localization_plugins
 */
class views_plugin_localization_core extends views_plugin_localization {

  /**
   * Translate a string.
   *
   * @param string $string
   *   The string to be translated.
   * @param array $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   * @param string $format
   *   The input format of the string. This is optional.
   */
  public function translate_string($string, $keys = array(), $format = '') {
    return t($string);
  }

  /**
   * Save a string for translation.
   *
   * @param string $string
   *   The string to be translated.
   * @param array $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   * @param string $format
   *   The input format of the string. This is optional.
   */
  public function save_string($string, $keys = array(), $format = '') {
    // Find a language that is not "en" (if available) to trigger an entry
    // in module locale's db tables, even if current language is "en".
    $langcode = 'en';
    $languages = language_list();
    unset($languages['en']);
    if (!empty($languages)) {
      $other_language = reset($languages);
      $langcode = $other_language->langcode;
    }
    t($string, array(), array('langcode' => $langcode));

    return TRUE;
  }

  /**
   * Delete a string.
   *
   * Deletion is not supported.
   *
   * @param mixed $source
   *   Full data for the string to be translated.
   */
  public function delete($source) {
    return FALSE;
  }

}
