<?php
/**
 * @file
 * Theme settings file for Seven.
 */

/**
 * Implements hook_preprocess_page().
 */
// @todo: create a function for this form, so that other themes can use it, or
// make it a theme-independent setting somewhere in the admin UI.
global $theme;
$themes = list_themes();

$form['help_text_position'] = array(
  '#type' => 'fieldset',
  '#title' => t('Help text position'),
  '#description' => t('The help/description text in forms is by default positioned after all elements, except for fieldsets, where it is positioned right after the legend. The settings below allow you to change that for the %theme theme.', array('%theme' => isset($themes[$theme]->info['name']) ? $themes[$theme]->info['name'] : $theme)),
  '#collapsible' => FALSE,
);

$position_options = array(
  'after' => t('After the element'),
  'before' => t('Before the element'),
  'invisible' => t('Invisible'),
);

$respect_explicit_description_display = theme_get_setting('respect_explicit_description_display');
$form['help_text_position']['respect_explicit_description_display'] = array(
  '#type' => 'checkbox',
  '#title' => t('Respect the <code>#description_display</code> property'),
  '#default_value' => isset($respect_explicit_description_display) ? $respect_explicit_description_display : TRUE,
  '#description' => t('This setting controls whether this property will be respected if it is explicitly set for an element, or if it will be overridden by the settings configured below regardless.'),
);

$fieldsets = theme_get_setting('fieldsets_description_position');
$form['help_text_position']['fieldsets_description_position'] = array(
  '#type' => 'select',
  '#title' => t('Fieldsets'),
  '#options' => $position_options,
  '#default_value' => isset($fieldsets) ? $fieldsets : 'before',
);
$radios_and_checkboxes = theme_get_setting('radios_and_checkboxes_description_position');
$form['help_text_position']['radios_and_checkboxes_description_position'] = array(
  '#type' => 'select',
  '#title' => t('Radios and checkboxes'),
  '#options' => $position_options,
  '#default_value' => isset($radios_and_checkboxes) ? $radios_and_checkboxes : 'after',
);
$default = theme_get_setting('default_description_position');
$form['help_text_position']['default_description_position'] = array(
  '#type' => 'select',
  '#title' => t('All other elements'),
  '#options' => $position_options,
  '#default_value' => isset($default) ? $default : 'after',
);
