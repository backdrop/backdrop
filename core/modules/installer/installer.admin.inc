<?php
/**
 * @file
 * Admin page callbacks for the Installer module.
 */

/**
 * Form constructor for the "System updates" settings form.
 *
 * @see installer_settings_submit()
 * @ingroup forms
 */
function installer_settings($form, &$form_state) {
  $config = config('installer.settings');
  $form['#config'] = 'installer.settings';

  $docs = l('File permissions and ownership', 'https://backdropcms.org/file-permissions-and-ownership');
  $form['self_update'] = array(
    '#type' => 'fieldset',
    '#title' => t('Self-updates'),
    '#description' => t('This website has the ability to update itself, but the file system permissions must be set properly. For more details, see !link.',
      array('!link' => $docs)),
  );
  $form['self_update']['core_update'] = array(
    '#type' => 'checkbox',
    '#title' => t('Manual self-update'),
    '#default_value' => $config->get('core_update'),
    '#description' => t('When updating manually, allow this website to delete the old version of core, and then replace it with the new version.'),
  );
  $form['self_update']['automatic_update'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatic self-update (coming soon!)'),
    '#description' => t('When updates are available, allow this website to <strong>automatically</strong> delete the old version of core, and then replace it with the new version.'),
    '#default_value' => FALSE, // @todo update when #414 goes in.
    '#disabled' => TRUE, // @todo update when #414 goes in.
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
 * Form submission handler for installer_settings().
 *
 * @see installer_settings()
 */
function installer_settings_submit($form, &$form_state) {
  unset($form_state['values']['automatic_update']); // @todo remove when #414 goes in.
  system_settings_form_submit($form, $form_state);
}
