<?php

/**
 * @file
 * Definition of views_plugin_localization.
 */

/**
 * @defgroup views_localization_plugins Views localization plugins
 * @{
 * @todo.
 *
 * @see hook_views_plugins()
 */

/**
 * The base plugin to handle localization of Views strings.
 */
class views_plugin_localization extends views_plugin {

  /**
   * Store for exported strings.
   */
  public $export_strings = array();

  /**
   *
   */
  public $translate = TRUE;

  /**
   * Initialize the plugin.
   *
   * @param view $view
   *   The view object.
   */
  public function init(&$view) {
    $this->view = &$view;
  }

  /**
   * Translate a string / text with format.
   *
   * The $source parameter is an array with the following elements:
   * - value, source string
   * - format, input format in case the text has some format to be applied
   * - keys. An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g., 'header'.
   *
   * @param string $source
   *   Full data for the string to be translated.
   *
   * @return string
   *   Translated string / text.
   */
  public function translate($source) {
    // Allow other modules to make changes to the string before and after
    // translation.
    $source['pre_process'] = $this->invoke_translation_process($source, 'pre');
    $source['translation'] = $this->translate_string($source['value'], $source['keys'], $source['format']);
    $source['post_process'] = $this->invoke_translation_process($source, 'post');
    return $source['translation'];
  }

  /**
   * Translate a string.
   *
   * @param string $string
   *   The string to be translated.
   * @param array $keys
   *   An array of keys to identify the string. Generally constructed from
   *   view name, display_id, and a property, e.g. 'header'.
   * @param string $format
   *   The input format of the string. This is optional.
   */
  public function translate_string($string, $keys = array(), $format = '') {}

  /**
   * Save string source for translation.
   *
   * @param string $source
   *   Full data for the string to be translated.
   */
  public function save($source) {
    // Allow other modules to make changes to the string before saving.
    $source['pre_process'] = $this->invoke_translation_process($source, 'pre');
    $this->save_string($source['value'], $source['keys'], isset($source['format']) ? $source['format'] : '');
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
  public function save_string($string, $keys = array(), $format = '') {}

  /**
   * Delete a string.
   *
   * @param string $source
   *   Full data for the string to be translated.
   */
  public function delete($source) {
  }

  /**
   * Collect strings to be exported to code.
   *
   * @param string $source
   *   Full data for the string to be translated.
   */
  public function export($source) {
  }

  /**
   * Render any collected exported strings to code.
   *
   * @param string $indent
   *   An optional indentation for prettifying nested code.
   */
  public function export_render($indent = '  ') {
  }

  /**
   * Invoke hook_translation_pre_process() or hook_translation_post_process().
   *
   * Like node_invoke_nodeapi(), this function is needed to enable both passing
   * by reference and fetching return values.
   */
  public function invoke_translation_process(&$value, $op) {
    $return = array();
    $hook = 'translation_' . $op . '_process';
    foreach (module_implements($hook) as $module) {
      $function = $module . '_' . $hook;
      $result = $function($value);
      if (isset($result)) {
        $return[$module] = $result;
      }
    }
    return $return;
  }

  /**
   *
   */
  public function process_locale_strings($op) {
    $this->view->init_display();

    foreach ($this->view->display as $display_id => $display) {
      $translatable = array();
      // Special handling for display title.
      if (isset($display->display_title)) {
        $translatable[] = array('value' => $display->display_title, 'keys' => array('display_title'));
      }
      // Unpack handlers.
      if (is_object($this->view->display[$display_id]->handler)) {
        $this->view->display[$display_id]->handler->unpack_translatables($translatable);
      }
      foreach ($translatable as $data) {
        $data['keys'] = array_merge(array($this->view->name, $display_id), $data['keys']);
        switch ($op) {
          case 'save':
            $this->save($data);
            break;

          case 'delete':
            $this->delete($data);
            break;

          case 'export':
            $this->export($data);
            break;
        }
      }
    }
  }

}

/**
 * @}
 */
