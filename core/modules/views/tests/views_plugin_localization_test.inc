<?php

/**
 * @file
 * Definition of views_plugin_localization_test.
 */

/**
 * A stump localisation plugin which has static variables to cache the input.
 */
class views_plugin_localization_test extends views_plugin_localization {

  /**
   * Store the strings which was translated.
   */
  public $translated_strings;
  public $exportStrings = array();

  /**
   * Return the string and take sure that the test can find out whether the
   * string got translated.
   */
  public function translate_string($string, $keys = array(), $format = '') {
    $this->translated_strings[] = $string;
    return $string . "-translated";
  }

  /**
   * Store the export strings.
   */
  public function export($source) {
    if (!empty($source['value'])) {
      $this->exportStrings[] = $source['value'];
    }
  }

  /**
   * Return the stored strings for the simpletest.
   */
  public function getExportStrings() {
    return $this->exportStrings;
  }

}
