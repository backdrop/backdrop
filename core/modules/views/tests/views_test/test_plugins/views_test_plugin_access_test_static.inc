<?php
/**
 * @file
 * Definition of views_test_plugin_access_test_static.
 */

/**
 * Tests a static access plugin.
 */
class views_test_plugin_access_test_static extends views_plugin_access {
  function option_definition() {
    $options = parent::option_definition();
    $options['access'] = ['default' => FALSE, 'bool' => TRUE];

    return $options;
  }

  function access($account) {
    return !empty($this->options['access']);
  }

  function get_access_callback() {
    return ['views_test_test_static_access_callback', [!empty($options['access'])]];
  }
}
