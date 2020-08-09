<?php
/**
 * @file
 * Theme settings file for Basis.
 *
 * Although Basis itself does not provide any settings, we use this file to
 * inform the user that the module supports color schemes if the Color module
 * is enabled.
 */

if (module_exists('color')) {
  $form['header'] = array(
    '#type' => 'fieldset',
    '#title' => t('Header Settings'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'header',
    'base',
    'slogan',
    'titleslogan',
    'hovermenu',
    'menutoggle',
  );
  foreach ($fields as $field) {
    $form['header'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'bg',
    'text',
    'link',
    'borders',
    'formfocusborder',
  );
  foreach ($fields as $field) {
    $form['general'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  $form['primary_tabs'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tabs and Breadcrumb'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'primarytabs',
    'primarytabstext',
    'buttons',
  );
  foreach ($fields as $field) {
    $form['primary_tabs'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }

  $form['footer'] = array(
    '#type' => 'fieldset',
    '#title' => t('Footer Settings'),
    '#collapsible' => TRUE,
  );
  $fields = array(
    'footerborder',
    'footer',
    'footertext',
  );
  foreach ($fields as $field) {
    $form['footer'][$field] = color_get_color_element($form['theme']['#value'], $field, $form);
  }
}
else {
  $form['color'] = array(
    '#markup' => '<p>' . t('This theme supports custom color palettes if the Color module is enabled on the <a href="!url">modules page</a>. Enable the Color module to customize this theme.', array('!url' => url('admin/modules'))) . '</p>',
  );
}

function basis_form_system_theme_settings_alter(&$form, &$form_state) {
  $form['supplemental'] = array(
    '#type'      => 'fieldset',
    '#title'     => t('CSS Updates'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['supplemental']['basis_supplemental_css'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Use CSS updates for Basis'),
    '#default_value' => theme_get_setting('basis_supplemental_css', 'basis'),
    '#description'   => 'Checking this box includes css improvements that may break sites build prior to 1.17.0'
  );
}
