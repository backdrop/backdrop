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
    global $language;

    // If the current language is 'en', we need to reset the language
    // in order to trigger an update.
    // @todo add test for number of languages.
    if ($language->langcode == 'en') {
      $changed = TRUE;
      $languages = language_list();
      $cached_language = $language;
      unset($languages['en']);
      if (!empty($languages)) {
        $language = current($languages);
      }
    }

    t($string);

    if (isset($cached_language)) {
      $language = $cached_language;
    }
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
