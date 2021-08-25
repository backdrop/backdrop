<?php
/**
 * @file
 * Admin page callbacks for the Update Manager module.
 */

/**
 * Form constructor for the update settings form.
 *
 * @see update_settings_validate()
 * @see update_settings_submit()
 * @ingroup forms
 */
function update_settings($form, &$form_state) {
  $config = config('update.settings');

  $form['check'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check for available updates'),
    '#description' =>  t('This website will check for new releases of Backdrop core, as well as contributed modules, themes, and layouts. If there are updates available, a message will appear on the <a href="@status_report">status report</a> page. An error message will appear if there is a security update available.',
      array('@status_report' => url('admin/reports/status'))),
  );
  $form['check']['update_check_frequency'] = array(
    '#type' => 'radios',
    '#title' => t('How frequently should this website check for updates?'),
    '#default_value' => $config->get('update_interval_days'),
    '#options' => array(
      '1' => t('Check daily'),
      '7' => t('Check weekly'),
      '0' => t('Do not check automatically'),
    ),
  );
  $form['check']['update_check_disabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Also check for updates of disabled modules, themes, and layouts'),
    '#default_value' => $config->get('update_disabled_extensions'),
  );

  $form['email'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update Notifications'),
    '#description' => t('This website can send notifications via e-mail when updates are available.'),
    '#states' => array(
      'visible' => array(
        'input[name=update_check_frequency]' => array('!value' => 0),
      ),
    ),
  );
  $form['email']['update_notification_threshold'] = array(
    '#type' => 'radios',
    '#title' => t('When should this website send e-mail notifications?'),
    '#default_value' => $config->get('update_threshold'),
    '#options' => array(
      'all' => t('When any update is available'),
      'security' => t('Only if a security update is available'),
      'none' => t('Do not send email notifications'),
    ),
  );
  $notify_emails = $config->get('update_emails');
  if (empty($notify_emails)) {
    $notify_emails = array();
  }
  $form['email']['update_notify_emails'] = array(
    '#type' => 'textarea',
    '#title' => t('E-mail addresses to notify when updates are available'),
    '#rows' => 4,
    '#default_value' => implode("\n", $notify_emails),
    '#description' => t('Put each email address on a separate line. If blank, no e-mails will be sent.'),
    '#states' => array(
      'visible' => array(
        'input[name=update_notification_threshold]' => array('!value' => 'none'),
      ),
    ),
  );

  $docs = l('File permissions and ownership', 'https://backdropcms.org/file-permissions-and-ownership');
  $form['self_update'] = array(
    '#type' => 'fieldset',
    '#title' => t('Self-updates'),
    '#description' => t('This website has the ability to update itself, but the file system permissions must be set properly. For more details, see !link.',
      array('!link' => $docs)),
  );
  $core_update = config_get('installer.settings', 'core_update');
  $form['self_update']['core_update'] = array(
    '#type' => 'checkbox',
    '#title' => t('Manual self-update'),
    '#default_value' => $core_update,
    '#description' => t('When updating manually, allow this website to delete the old version of core, and then replace it with the new version.'),
  );
  $form['self_update']['automatic_update'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatic self-update (coming soon!)'),
    '#description' => t('When updates are available, allow this website to <strong>automatically</strong> delete the old version of core, and then replace it with the new version.'),
    '#default_value' => FALSE, // @todo update when #414 goes in
    '#disabled' => TRUE, // @todo update when #414 goes in
    '#states' => array(
      'visible' => array(
        'input[name=update_check_frequency]' => array('!value' => 0),
      ),
    ),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

/**
 * Form validation handler for update_settings().
 *
 * Validates the e-mail addresses and ensures the field is formatted correctly.
 *
 * @see update_settings_submit()
 */
function update_settings_validate($form, &$form_state) {
  $form_state['notify_emails'] = array();
  if (!empty($form_state['values']['update_notify_emails'])) {
    $valid = array();
    $invalid = array();
    foreach (explode("\n", trim($form_state['values']['update_notify_emails'])) as $email) {
      $email = trim($email);
      if (!empty($email)) {
        if (valid_email_address($email)) {
          $valid[] = $email;
        }
        else {
          $invalid[] = $email;
        }
      }
    }
    if (empty($invalid)) {
      $form_state['notify_emails'] = $valid;
    }
    elseif (count($invalid) == 1) {
      form_set_error('update_notify_emails', t('%email is not a valid e-mail address.', array('%email' => reset($invalid))));
    }
    else {
      form_set_error('update_notify_emails', t('%emails are not valid e-mail addresses.', array('%emails' => implode(', ', $invalid))));
    }
  }
}

/**
 * Form submission handler for update_settings().
 *
 * Also invalidates the cache of available updates if the "Check for updates of
 * disabled and uninstalled modules, themes, and layouts." setting is being
 * changed. The available updates report needs to refetch available update data
 * after this setting changes or it would show misleading things (e.g., listing
 * the disabled projects on the site with the "No available releases found"
 * warning).
 *
 * @see update_settings_validate()
 */
function update_settings_submit($form, $form_state) {
  $config = config('update.settings');

  // See if the update_check_disabled setting is being changed, and if so,
  // invalidate all cached update status data.
  if ($form_state['values']['update_check_disabled'] != $config->get('update_disabled_extensions')) {
    _update_cache_clear();
  }

  $config
    ->set('update_disabled_extensions', $form_state['values']['update_check_disabled'])
    ->set('update_interval_days', $form_state['values']['update_check_frequency'])
    ->set('update_emails', $form_state['notify_emails'])
    ->set('update_threshold', $form_state['values']['update_notification_threshold'])
    ->save();

  // Save the installer setting.
  config_set('installer.settings', 'core_update', $form_state['values']['core_update']);

  // Display status messages for this form.
  backdrop_set_message(t('The configuration options have been saved.'));
}
