<?php
/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */

/**
 * menu callback for settings page
 */
function dashboard_admin_settings() {
  $config = config('dashboard.settings');
  // Redirect on log in.
  $form['#config'] = 'dashboard.settings';
  $form['login_redirect'] = array(
    '#type' => 'checkbox',
    '#title' => t('Redirect administrators to Dashboard after log in'),
    '#default_value' => $config->get('login_redirect'),
    '#description' => t('Users with the !permission can be automatically 
      redirected to the Dashboard when they log in.', array(
      '!permission' => l(t('Have Total Control permission'), 'admin/config/people/permissions'),
    )),
  );

  return system_settings_form($form);
}
