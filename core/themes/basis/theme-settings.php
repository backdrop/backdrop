<?php
/**
 * @file
 * Theme settings file for Basis.
 *
 * Although Basis itself does not provide any settings, we use this file to
 * inform the user that the module supports color schemes if the Color module
 * is enabled.
 *
 * @var array $form
 *   Modifications to this variable affect the contents of the theme form.
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

$theme_name = $form['theme']['#value'];
$theme_path = backdrop_get_path('theme', 'basis');
require_once $theme_path . '/template.php';
$install_version = config_get('system.core', 'install_version');

$css_update_options = array(
  'install_version' => t('Default'),
  'all_updates' => t('All updates'),
  'custom' => t('Specific version'),
);

$form['css_updates'] = array(
  '#type' => 'details',
  '#open' => FALSE,
  '#summary' => t('CSS Updates'),
  '#details' => t('There are occasionally updates available for Basis that may affect existing sites, changing this setting may cause unexpected changes to your theme. For more information and details, see the <a href="https://docs.backdropcms.org/documentation/layouts-and-templates" target="_blank">online documentation</a>.</br></br>Select which updates you would like to accept. '),
  '#attributes' => array(
    'class' => array('description'),
  ),
);

// Load the saved configuration value.
$form['css_updates']['css_update'] = array(
  '#title' => t('CSS Update Options'),
  '#type' => 'radios',
  '#options' => $css_update_options,
  '#default_value' => theme_get_setting('css_update', $theme_name),
  '#config' => 'basis.settings',
  'default' => array(
    '#description' => t('Accept no updates introduced after installation.'),
  ),
  'all_updates' => array(
    '#description' => t('Warning: Some future changes may effect your site.'),
  ),
  'custom' => array(
  ),
);

// Custom select css update versions.
$options = backdrop_map_assoc(basis_updated_css_versions());
$options[''] = t('No updates');
$default_version_value = theme_get_setting('css_update_version', $theme_name);
$default_version_key = array_search($default_version_value, $options, TRUE);
$form['css_updates']['css_update_version'] = array(
  '#type' => 'select',
  '#title' => t('Version'),
  '#description' => t('Accept CSS changes up to this specific version of Backdrop.'),
  '#options' => $options,
  '#default_value' => $default_version_key,
  '#empty_option' => t('Default'),
  '#empty_value' => NULL,
  '#states' => array(
    'visible' => array(
      ':input[name="css_update"]' => array('value' => 'custom'),
    ),
  ),
  '#process' => array('basis_process_css_update_value'),
);

/**
 * Process function to adjust the CSS update version value before saving.
 */
function basis_process_css_update_value($element, &$form_state, $form) {
  $css_update = isset($form_state['values']['css_update']) ? $form_state['values']['css_update'] : 'install_version';
  $basis_css_versions = basis_updated_css_versions();
  $install_version = config_get('system.core', 'install_version');

  // Find the CSS update version within Basis that matches the install version.
  $basis_install_version = '';
  foreach ($basis_css_versions as $css_version) {
    if (version_compare($css_version, $install_version, '<')) {
      break;
    }
    $basis_install_version = $css_version;
  }

  // Adjust version value based on preference.
  switch ($css_update) {
    case 'install_version':
      $form_state['values']['css_update_version'] = $basis_install_version;
      break;

    case 'custom':
      $selected_key = isset($form_state['values']['css_update_version']) ? $form_state['values']['css_update_version'] : NULL;
      if (isset($basis_css_versions[$selected_key])) {
        $form_state['values']['css_update_version'] = $basis_css_versions[$selected_key];
      }
      break;
  }

  return $element;
}
