<?php
/**
 * @file
 * Theme settings file for Basis.
 */

/**
 * Implements hook_form_system_theme_settings_alter().
 */
function basis_form_system_theme_settings_alter(&$form, &$form_state) {
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
  elseif (user_access('administer modules')) {
    $form['color'] = array(
      '#markup' => '<p>' . t('This theme supports custom color palettes if the Color module is enabled on the <a href="!url">modules page</a>. Enable the Color module to customize this theme.', array('!url' => url('admin/modules'))) . '</p>',
    );
  }
  else {
    $form['color'] = array(
      '#markup' => '<p>' . t('This theme supports custom color palettes if the Color module is enabled. Contact your website administrator to enable the Color module to customize this theme.') . '</p>',
    );
  }
}
