<?php

/**
 * Display a list of the available code_sign_hash profiles.  Include edit/delete
 * controls.
 */
function code_sign_hash_admin_form($form, &$form_state) {
  $profiles = config_get('code_sign_hash.settings', 'profiles');
  $headers = array(
    'name' => t('Profile name'),
    'hash_algorithm' => t('Hash Agorithm'),
    'ops' => t('Operations'),
  );

  // Make sure the list of profiles is an array.
  if (!$profiles) {
    $profiles = array();
  }

  // Build the rows for the table from the list of profiles.
  $rows = array();
  foreach ($profiles as $pid => $p) {
    $rows[] = array(
      'name' => $p['name'],
      'hash_algorithm' => $p['hash_algorithm'],
      'ops' => l(t('Edit'), 'admin/config/system/code-sign/hash/' . $pid . '/edit') . ' ' .
        l(t('Delete'), 'admin/config/system/code-sign/hash/' . $pid . '/delete')
    );
  }

  // Build the table
  $form['profiles'] = array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $rows,
    '#empty' => t('No profiles defined.'),
  );

  return $form;
}

/**
 * Create a form to create a new Code Sign Hash profile or to edit an existing
 * one.
 *
 * @param $form
 *   The initial form.
 * @param &$form_state
 *   The form state.
 * @param $profile_id
 *   The machine name of the profile to edit.  NULL to create a new entity.
 * @return
 *   The codesign hash edit profile form.
 */
function code_sign_hash_edit_profile_form($form, &$form_state, $profile_id = NULL) {
  $all_profiles = config_get('code_sign_hash.settings', 'profiles');

  // Load an existing profile or prep the array for a new one.  Fill in any
  // missing fields with reasonable default values.
  $profile = array_key_exists($profile_id, $all_profiles) ? $all_profiles[$profile_id] : array();
  $profile += array(
    'name' => '',
    'machine_name' => '',
    'hash_algorithm' => 'sha256',
  );

  $form['#tree'] = TRUE;
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => $profile_id,
  );
  $form['profile']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile Name'),
    '#default_value' => $profile['name'],
    '#description' => t('The human-friendly name of the profile.'),
  );
  $form['profile']['machine_name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#default_value' => $profile['machine_name'],
    '#description' => t('A unique name for the profile.'),
    '#machine_name' => array(
      'exists' => 'code_sign_hash_profile_name_exists',
      'source' => array('profile', 'name'),
      'label' => t('URL path'),
      'replace_pattern' => '[^a-z0-9-]+',
      'replace' => '-',
    ),
  );
  $form['profile']['hash_algorithm'] = array(
    '#type' => 'select',
    '#title' => t('Signing Hash Algorithm'),
    '#default_value' => $profile['hash_algorithm'],
    '#options' => code_sign_hash_algorithms(),
  );

  $form['#submit'][] = 'code_sign_hash_edit_profile_form_submit';
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save profile'),
    ),
  );

  return $form;
}

/**
 * Return if the machine name exists already.
 * @param $profile
 *   The machine name of the profile.
 * @return
 *   TRUE if it exists, FALSE if it does not.
 */
function code_sign_hash_profile_name_exists($profile) {
  $profiles = config_get('code_sign_hash.settings', 'profiles');
  return !empty($profiles[$profile]);
}

/**
 * Save the new or edited profile.
 *
 * @param $form
 *   The form.
 * @param &$form_state
 *   The submitted form state.
 */
function code_sign_hash_edit_profile_form_submit($form, &$form_state) {
  // Extract data from the form.
  $values = $form_state['values'];
  $profile = $values['profile'];
  $profile_id = $profile['machine_name'];
  $original_profile_id = $values['profile_id'];

  // Get the list of profiles.
  $all_profiles = config_get('code_sign_hash.settings', 'profiles');
  if (!$all_profiles) {
    $all_profiles = array();
  }

  // Update the existing profile.
  $all_profiles[$profile_id] = $profile;

  // If the machine_name changes, delete the original.
  if ($original_profile_id && ($original_profile_id != $profile_id)) {
    unset($all_profiles[$original_profile_id]);
  }

  // Save
  config_set('code_sign_hash.settings', 'profiles', $all_profiles);

  // Return to the list of profiles.
  $form_state['redirect'] = 'admin/config/system/code-sign/hash';
}

function code_sign_hash_delete_profile_form($form, &$form_state, $profile_id) {
  $form['#profile'] = $profile_id;
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => $profile_id,
  );
  return confirm_form($form,
    t('Are you sure you want to delete profile @pid?', array('@pid' => $profile_id)),
    'admin/config/system/code-sign/hash',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel'),
    'code_sign_hash_delete_profile_form');
}

function code_sign_hash_delete_profile_form_submit($form, &$form_state) {
  $profile_id = $form['#profile'];
  config_clear('code_sign_hash.settings', 'profiles.' . $profile_id);
  $form_state['redirect'] = 'admin/config/system/code-sign/hash';
}
