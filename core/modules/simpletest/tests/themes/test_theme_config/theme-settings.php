<?php
 /**
 * @file
 * Theme settings file for the test_theme_config theme.
 */

$form['test_theme_checkbox_one'] = array(
  '#type' => 'checkbox',
  '#title' => 'Test theme checkbox one',
  '#default_value' => theme_get_setting('test_theme_checkbox_one', 'test_theme_config'),
);
$form['test_theme_checkbox_two'] = array(
  '#type' => 'checkbox',
  '#title' => 'Test theme checkbox two',
  '#default_value' => theme_get_setting('test_theme_checkbox_two', 'test_theme_config'),
);
$form['test_theme_textfield'] = array(
  '#type' => 'textfield',
  '#title' => 'Test theme textfield',
  '#default_value' => theme_get_setting('test_theme_textfield', 'test_theme_config'),
);
