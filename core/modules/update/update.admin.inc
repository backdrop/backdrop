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
  $form['#config'] = 'update.settings';

  $form['check'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check for available updates'),
    '#description' =>  t('This website will check for new releases of Backdrop core, as well as contributed modules, themes, and layouts. If there are updates available, a message will appear on the <a href="@status_report">status report</a> page. An error message will appear if there is a security update available.',
      array('@status_report' => url('admin/reports/status'))),
  );
  $form['check']['update_interval_days'] = array(
    '#type' => 'radios',
    '#title' => t('How frequently should this website check for updates?'),
    '#default_value' => $config->get('update_interval_days'),
    '#options' => array(
      '1' => t('Check daily'),
      '7' => t('Check weekly'),
      '0' => t('Do not check automatically'),
    ),
  );
  $form['check']['update_disabled_extensions'] = array(
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
  $form['email']['update_threshold'] = array(
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
  $form['email']['update_emails'] = array(
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

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Form validation handler for update_settings().
 *
 * Validates entered e-mail addresses, and prepares the field value for being
 * saved into configuration.
 *
 * @see update_settings_submit()
 */
function update_settings_validate($form, &$form_state) {
  $form_state['notify_emails'] = array();
  if (!empty($form_state['values']['update_emails'])) {
    $valid = array();
    $invalid = array();
    foreach (explode("\n", trim($form_state['values']['update_emails'])) as $email) {
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
      $form_state['values']['update_emails'] = $valid;

    }
    else {
      $invalid_emails = format_plural(count($invalid),
        '%email is not a valid e-mail address.',
        '%emails are not valid e-mail addresses.',
        array(
          '%email' => reset($invalid),
          '%emails' => implode(', ', $invalid),
        )
      );
      form_set_error('update_emails', $invalid_emails);
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
function update_settings_submit($form, &$form_state) {
  if ($form_state['values']['update_disabled_extensions'] != config_get('update.settings', 'update_disabled_extensions')) {
    _update_cache_clear();
  }

  system_settings_form_submit($form, $form_state);
}
