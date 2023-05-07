<?php
 /**
 * @file
 * Theme settings file for the test_theme theme.
 */
$form['test_theme_checkbox'] = array(
  '#type' => 'checkbox',
  '#title' => 'Test theme checkbox',
  '#default_value' => theme_settings_get('test_theme_checkbox', NULL, 'test_theme'),
);
$form['test_theme_checkbox_default_value_true'] = array(
  '#type' => 'checkbox',
  '#title' => 'Test theme checkbox with its default value set to TRUE',
  '#default_value' => theme_settings_get('test_theme_checkbox_default_value', TRUE, 'test_theme'),
);
$form['test_theme_checkbox_default_value_false'] = array(
  '#type' => 'checkbox',
  '#title' => 'Test theme checkbox with default value set to FALSE',
  '#default_value' => theme_settings_get('test_theme_checkbox_default_value', FALSE, 'test_theme'),
);
// Force the form to be cached so we can test that this file is properly
// loaded and the custom submit handler is properly called even on a cached
// form build.
$form_state['cache'] = TRUE;
