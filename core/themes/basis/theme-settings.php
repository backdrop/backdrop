<?php
/**
 * @file
 * Theme settings file for Basis.
 *
 * Although Basis itself does not provide any settings, we use this file to
 * inform the user that the module supports color schemes if the Color module
 * is enabled.
 */
$form['color'] = array(
  '#markup' => '<p>' . t('This theme supports custom color palettes if the Color module is enabled on the <a href="!url">modules page</a>. Enable the Color module to customize this theme.', array('!url' => url('admin/modules'))) . '</p>',
);

$form['header'] = array(
  '#type' => 'fieldset',
  '#title' => t('Header Settings'),
  '#collapsible' => TRUE,
  'color_header' => array(),
  'color_base' => array(),
  'color_slogan' => array(),
  'color_titleslogan' => array(),
  'color_hovermenu' => array(),
);

$form['main'] = array(
  '#type' => 'fieldset',
  '#title' => t('Main Settings'),
  '#collapsible' => TRUE,
  'color_bg' => array(),
  'color_text' => array(),
  'color_link' => array(),
  'color_borders' => array(),
  'color_formfocusborder' => array(),
  'color_buttons' => array(),
);

$form['footer'] = array(
  '#type' => 'fieldset',
  '#title' => t('Footer Settings'),
  '#collapsible' => TRUE,
  'color_footer' => array(),
);
