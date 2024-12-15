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

function basis_form_system_theme_settings_alter(&$form, &$form_state, $form_id = NULL) {
  // Ensure template.php is included.
  $theme_name = $form['theme']['#value'];
  $theme_path = backdrop_get_path('theme', 'basis');
  require_once $theme_path . '/template.php';
  $install_version = config_get('system.core', 'install_version');

  $css_update_options = array(
    'installation' => t('Updates thru installation'),
    'all_updates' => t('All updates'),
    'custom' => t('Updates thru specific version.'),
  );

  $form['css_updates'] = array(
    '#type' => 'details',
    '#open' => FALSE,
    '#summary' => t('Expert Options'),
    '#details' => t('There are occasional updates to Basis that may effect existing sites. For more information and details, see the <a href="https://docs.backdropcms.org/documentation/layouts-and-templates" target="_blank">online documentation</a>.</br></br>Select which updates you would like to accept. '),
    '#attributes' => array(
      'class' => array('description'),
    ),
  );

  // Load the saved configuration value.
  $form['css_updates']['css_update_preference'] = array(
    '#title' => t('CSS Update Options'),
    '#type' => 'radios',
    '#options' => $css_update_options,
    '#default_value' => theme_get_setting('css_update_preference', $theme_name),
    '#config' => 'basis.settings',
    'installation' => array(
      '#description' => t('Accept changes that were made prior to or including the initial install version of your Backdrop site (Your site: '. $install_version . ').'),
    ),
    'all_updates' => array(
      '#description' => t('Warning: Some future changes may effect your site.'),
    ),
    'custom' => array(
    ),
  );

  $default_version_value = theme_get_setting('css_update_version_preference', $theme_name);

  // Reverse lookup to get the correct key if a value is stored instead of a key.
  $options = basis_supplemental_css_versions();
  $default_version_key = array_search($default_version_value, $options, TRUE);

  // Conditionally displayed CSS updates select element.
  $form['css_updates']['css_update_version_preference'] = [
    '#type' => 'select',
    '#title' => t('Version'),
    '#description' => t('Accept CSS changes through this specific version of Backdrop.'),
    '#options' => $options,
    '#default_value' => $default_version_key,
    '#empty_option' => t('Default'),
    '#empty_value' => NULL,
    '#states' => [
      'visible' => [
        ':input[name="css_update_preference"]' => ['value' => 'custom'],
      ],
    ],
    '#process' => ['basis_process_css_update_value'],
  ];
}

/**
 * Process function to adjust the CSS update version value before saving.
 */
function basis_process_css_update_value($element, &$form_state, $form) {
  $css_update_preference = $form_state['values']['css_update_preference'] ?? 'installation';
  $options = basis_supplemental_css_versions();
  $install_version = config_get('system.core', 'install_version');

  // Adjust version value based on preference.
  switch ($css_update_preference) {
    case 'installation':
      $form_state['values']['css_update_version_preference'] = $install_version;
      break;

    case 'all_updates':
      $form_state['values']['css_update_version_preference'] = BACKDROP_VERSION; // Latest version.
      break;

    case 'custom':
      $selected_key = $form_state['values']['css_update_version_preference'] ?? NULL;
      if (isset($options[$selected_key])) {
        $form_state['values']['css_update_version_preference'] = $options[$selected_key];
      }
      break;
  }

  return $element;
}
