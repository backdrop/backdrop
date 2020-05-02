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
    if ($language->language == 'en') {
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

  /**
   * Collect strings to be exported to code.
   *
   * String identifiers are not supported so strings are anonymously in an
   * array.
   *
   * @param array $source
   *   Full data for the string to be translated.
   */
  public function export($source) {
    if (!empty($source['value'])) {
      $this->export_strings[] = $source['value'];
    }
  }

  /**
   * Render any collected exported strings to code.
   *
   * @param string $indent
   *   An optional indentation for prettifying nested code.
   */
  public function export_render($indent = '  ') {
    $output = '';
    if (!empty($this->export_strings)) {
      $this->export_strings = array_unique($this->export_strings);
      $output = $indent . '$translatables[\'' . $this->view->name . '\'] = array(' . "\n";
      foreach ($this->export_strings as $string) {
        $output .= $indent . "  t('" . str_replace("'", "\'", $string) . "'),\n";
      }
      $output .= $indent . ");\n";
    }
    return $output;
  }

}
