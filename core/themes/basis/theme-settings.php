<?php
/**
 * @file
 * Theme settings file for Basis.
 *
 */

$form['menu_toggle'] = array(
  '#type' => 'checkbox',
  '#title' => 'Enable menu toggle button for the primary navigation',
  '#default_value' => theme_get_setting('menu_toggle', 'basis'),
  '#description' => t('When enabled, a menu toggle button—commonly known as a "hamburger" icon—will appear and allow the user to toggle the visibility of the primary navigation. This will only work if the primary navigation is visible.'),
);
