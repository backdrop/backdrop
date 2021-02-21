<?php

/**
 * @file
 * Definition of views_plugin_localization_none.
 */

/**
 * Localization plugin for no localization.
 *
 * @ingroup views_localization_plugins
 */
class views_plugin_localization_none extends views_plugin_localization {

  /**
   * {@inheritdoc}
   */
  public $translate = FALSE;

  /**
   * Translate a string; simply return the string.
   */
  public function translate($source) {
    return $source['value'];
  }

  /**
   * Save a string for translation; not supported.
   */
  public function save($source) {
    return FALSE;
  }

  /**
   * Delete a string; not supported.
   */
  public function delete($source) {
    return FALSE;
  }

}
