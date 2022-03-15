<?php
/**
 * @file
 * Theme settings file for Bartik.
 *
 * Although Bartik itself does not provide any settings, we use this file to
 * inform the user that the module supports color schemes if the Color module
 * is enabled.
 */
$form['tabs_wrapper'] = array(
  '#type' => 'fieldset',
  '#title' => t('Main menu tab style'),
);
$form['tabs_wrapper']['main_menu_tabs'] = array(
  '#type' => 'radios',
  '#options' => array(
    'no-tabs' => t('No tabs'),
    'rounded-tabs' => t('Rounded tabs'),
    'square-tabs' => t('Square tabs'),
  ),
  '#default_value' => theme_get_setting('main_menu_tabs', 'bartik'),
  '#description' => t('When rounded or square tabs are selected, menu link color is overridden and set to #333 for better visibility.'),
);
