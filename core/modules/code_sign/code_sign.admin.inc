<?php

/**
 * @file
 * Central administrative pages for Code Sign profile management.
 *
 * Individual signing engines do not provide their own profile management UI,
 * nor do they save their own configuration. Instead, each engine's class
 * provides form(), formValidate(), and formSubmit() methods (similar to the
 * way the Block class builds its settings form), which are invoked
 * generically below to build and process the submitted values. This module
 * then centrally persists those values - the entire contents of
 * $form_state['values'], minus internal Form API keys - into a single
 * configuration file per profile, code_sign.profile.[machine_name], which
 * also records which engine the profile belongs to. This allows profiles
 * from any number of enabled Code Sign engines to be listed, added, edited,
 * and deleted together on one set of pages.
 *
 * @see CodeSignEngine::form()
 * @see CodeSignEngine::formValidate()
 * @see CodeSignEngine::formSubmit()
 * @see CodeSignEngine::deleteProfile()
 * @see CodeSignEngine::isAvailable()
 * @see code_sign_save_profile()
 * @see code_sign_delete_profile()
 */

/**
 * Sets an error message and redirects when an engine is unavailable.
 *
 * @param string $engine_name
 *   The machine name of the unavailable signing engine.
 */
function _code_sign_engine_unavailable($engine_name) {
  $all_signers = code_sign_get_engines();
  $label = isset($all_signers[$engine_name]['label']) ? $all_signers[$engine_name]['label'] : $engine_name;

  backdrop_set_message(t('The %label signing engine is currently unavailable.', array('%label' => $label)), 'error');
  backdrop_goto('admin/config/system/code-sign');
}

/**
 * Page callback: Lists the profiles for every enabled signing engine.
 */
function code_sign_admin_form($form, &$form_state) {
  $all_signers = code_sign_get_engines();

  if (empty($all_signers)) {
    $form['empty'] = array(
      '#type' => 'markup',
      '#markup' => t('No algorithm modules have been enabled. Try enabling !code_sign_hash.', array('!code_sign_hash' => l(t('Hashing Code Sign'), 'admin/modules/list'))),
    );
    return $form;
  }

  $rows = array();
  $unavailable = array();
  foreach ($all_signers as $engine_name => $info) {
    $engine = code_sign_get_engine($engine_name);
    if (!$engine) {
      continue;
    }
    if (!$engine->isAvailable()) {
      $unavailable[] = $info['label'];
      continue;
    }

    foreach ($engine->getPrivateKeychain() as $profile_id => $profile) {
      $operations = array(
        'edit' => array(
          'title' => t('Edit'),
          'href' => 'admin/config/system/code-sign/profile/' . $engine_name . '/' . $profile_id . '/edit',
        ),
        'delete' => array(
          'title' => t('Delete'),
          'href' => 'admin/config/system/code-sign/profile/' . $engine_name . '/' . $profile_id . '/delete',
        ),
      );

      $rows[] = array(
        !empty($profile['name']) ? check_plain($profile['name']) : $profile_id,
        check_plain($info['label']),
        array(
          'data' => array(
            '#type' => 'operations',
            '#links' => $operations,
          ),
        ),
      );
    }
  }

  if ($unavailable) {
    $form['unavailable'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages warning">' . t('The following signing engines are currently unavailable and have been hidden: @engines.', array('@engines' => implode(', ', $unavailable))) . '</div>',
    );
  }

  $form['profiles'] = array(
    '#theme' => 'table',
    '#header' => array(t('Profile name'), t('Engine'), t('Operations')),
    '#rows' => $rows,
    '#empty' => t('No profiles defined.'),
  );

  return $form;
}

/**
 * Returns the available signing engines, keyed by machine name.
 *
 * @return array
 *   An array of engine labels, keyed by machine name, for engines that are
 *   both registered and currently available.
 */
function _code_sign_available_engines() {
  $available = array();

  foreach (code_sign_get_engines() as $engine_name => $info) {
    $engine = code_sign_get_engine($engine_name);
    if ($engine && $engine->isAvailable()) {
      $available[$engine_name] = $info['label'];
    }
  }

  return $available;
}

/**
 * Page callback: Builds the consolidated add-profile form.
 *
 * @return array
 *   A form render array.
 */
function code_sign_profile_add_page() {
  return backdrop_get_form('code_sign_profile_add_form');
}

/**
 * Builds the consolidated add-profile form.
 *
 * This is a two-step form: the first step asks which signing engine to use,
 * and the second step builds that engine's profile fields via
 * CodeSignEngine::form(). Splitting it this way (rather than dynamically
 * swapping fields as the engine selection changes) keeps the form fully
 * functional without JavaScript.
 *
 * @param array $form
 *   The initial form.
 * @param array &$form_state
 *   The form state.
 *
 * @return array
 *   The add-profile form.
 */
function code_sign_profile_add_form($form, &$form_state) {
  $available_engines = _code_sign_available_engines();

  if (empty($available_engines)) {
    $form['empty'] = array(
      '#type' => 'markup',
      '#markup' => t('No signing engines are currently available.'),
    );
    return $form;
  }

  // Step 1: choose which engine to use. $form_state['storage']['engine'] is
  // set once the choice is confirmed, moving on to step 2.
  if (empty($form_state['storage']['engine'])) {
    $form['engine'] = array(
      '#type' => 'select',
      '#title' => t('Signing engine'),
      '#options' => $available_engines,
      '#default_value' => key($available_engines),
      '#required' => TRUE,
    );

    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Continue'),
        '#submit' => array('code_sign_profile_add_form_choose_engine'),
      ),
    );

    return $form;
  }

  // Step 2: build the chosen engine's profile fields.
  $engine_name = $form_state['storage']['engine'];
  $engine = code_sign_get_engine($engine_name);

  $form['engine_display'] = array(
    '#type' => 'item',
    '#title' => t('Signing engine'),
    '#markup' => check_plain($available_engines[$engine_name]),
  );
  $form['engine'] = array(
    '#type' => 'value',
    '#value' => $engine_name,
  );
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => NULL,
  );

  $engine->form($form, $form_state, array());

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Add profile'),
    ),
    'back' => array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#submit' => array('code_sign_profile_add_form_change_engine'),
      '#limit_validation_errors' => array(),
    ),
  );

  return $form;
}

/**
 * Submit handler for the "Continue" button: confirms the engine choice.
 */
function code_sign_profile_add_form_choose_engine($form, &$form_state) {
  $form_state['storage']['engine'] = $form_state['values']['engine'];
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Back" button: returns to the engine choice step.
 */
function code_sign_profile_add_form_change_engine($form, &$form_state) {
  unset($form_state['storage']['engine']);
  $form_state['rebuild'] = TRUE;
}

/**
 * Validate handler for code_sign_profile_add_form().
 */
function code_sign_profile_add_form_validate($form, &$form_state) {
  // Nothing to validate yet on the engine-choice step.
  if (empty($form_state['storage']['engine'])) {
    return;
  }

  $engine = code_sign_get_engine($form_state['values']['engine']);
  if (!$engine || !$engine->isAvailable()) {
    form_set_error('', t('This signing engine is currently unavailable.'));
    return;
  }

  $engine->formValidate($form, $form_state);
}

/**
 * Submit handler for code_sign_profile_add_form().
 */
function code_sign_profile_add_form_submit($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  if (!$engine || !$engine->isAvailable()) {
    backdrop_set_message(t('This signing engine is currently unavailable. The profile was not saved.'), 'error');
    $form_state['redirect'] = 'admin/config/system/code-sign';
    return;
  }

  // Let the engine process or augment the submitted values (for example,
  // generating a keypair) before they are centrally persisted.
  $engine->formSubmit($form, $form_state);
  code_sign_save_profile($form_state['values']);

  backdrop_set_message(t('Profile saved.'));
  $form_state['redirect'] = 'admin/config/system/code-sign';
}

/**
 * Page callback: Builds the edit profile form for a signing engine.
 *
 * @param string $engine_name
 *   The machine name of the signing engine, as defined by
 *   hook_code_sign_info().
 * @param string $profile_id
 *   The machine name of the profile to edit.
 *
 * @return array|int
 *   A form render array, or MENU_NOT_FOUND if the engine or profile is
 *   unknown.
 */
function code_sign_profile_edit_page($engine_name, $profile_id) {
  $engine = code_sign_get_engine($engine_name);
  if (!$engine) {
    return MENU_NOT_FOUND;
  }
  if (!$engine->isAvailable()) {
    _code_sign_engine_unavailable($engine_name);
  }
  $profiles = $engine->getPrivateKeychain();
  if (!isset($profiles[$profile_id])) {
    return MENU_NOT_FOUND;
  }

  return backdrop_get_form('code_sign_profile_form', $engine_name, $profile_id);
}

/**
 * Builds the edit profile form, delegating to the engine's form().
 *
 * @param array $form
 *   The initial form.
 * @param array &$form_state
 *   The form state.
 * @param string $engine_name
 *   The machine name of the signing engine.
 * @param string $profile_id
 *   The machine name of the profile to edit.
 *
 * @return array
 *   The profile form.
 */
function code_sign_profile_form($form, &$form_state, $engine_name, $profile_id) {
  $engine = code_sign_get_engine($engine_name);
  $all_profiles = $engine->getPrivateKeychain();
  $profile = $all_profiles[$profile_id];

  $form['engine'] = array(
    '#type' => 'value',
    '#value' => $engine_name,
  );
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => $profile_id,
  );

  $engine->form($form, $form_state, $profile);

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
 * Validate handler for code_sign_profile_form().
 */
function code_sign_profile_form_validate($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  if (!$engine || !$engine->isAvailable()) {
    form_set_error('', t('This signing engine is currently unavailable.'));
    return;
  }

  $engine->formValidate($form, $form_state);
}

/**
 * Submit handler for code_sign_profile_form().
 */
function code_sign_profile_form_submit($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  if (!$engine || !$engine->isAvailable()) {
    backdrop_set_message(t('This signing engine is currently unavailable. The profile was not saved.'), 'error');
    $form_state['redirect'] = 'admin/config/system/code-sign';
    return;
  }

  // Let the engine process or augment the submitted values (for example,
  // generating a keypair) before they are centrally persisted.
  $engine->formSubmit($form, $form_state);
  code_sign_save_profile($form_state['values']);

  backdrop_set_message(t('Profile saved.'));
  $form_state['redirect'] = 'admin/config/system/code-sign';
}

/**
 * Page callback: Builds the delete confirmation form for a profile.
 *
 * @param string $engine_name
 *   The machine name of the signing engine.
 * @param string $profile_id
 *   The machine name of the profile to delete.
 *
 * @return array|int
 *   A form render array, or MENU_NOT_FOUND if the engine or profile is
 *   unknown.
 */
function code_sign_profile_delete_page($engine_name, $profile_id) {
  $engine = code_sign_get_engine($engine_name);
  if (!$engine) {
    return MENU_NOT_FOUND;
  }
  if (!$engine->isAvailable()) {
    _code_sign_engine_unavailable($engine_name);
  }
  $profiles = $engine->getPrivateKeychain();
  if (!isset($profiles[$profile_id])) {
    return MENU_NOT_FOUND;
  }

  return backdrop_get_form('code_sign_profile_delete_form', $engine_name, $profile_id);
}

/**
 * Builds a confirmation form for deleting a profile.
 *
 * @param array $form
 *   The initial form.
 * @param array &$form_state
 *   The form state.
 * @param string $engine_name
 *   The machine name of the signing engine.
 * @param string $profile_id
 *   The machine name of the profile to delete.
 *
 * @return array
 *   The confirmation form.
 */
function code_sign_profile_delete_form($form, &$form_state, $engine_name, $profile_id) {
  $form['engine'] = array(
    '#type' => 'value',
    '#value' => $engine_name,
  );
  $form['profile_id'] = array(
    '#type' => 'value',
    '#value' => $profile_id,
  );

  return confirm_form($form,
    t('Are you sure you want to delete profile @pid?', array('@pid' => $profile_id)),
    'admin/config/system/code-sign',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel'));
}

/**
 * Submit handler for code_sign_profile_delete_form().
 */
function code_sign_profile_delete_form_submit($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  if (!$engine || !$engine->isAvailable()) {
    backdrop_set_message(t('This signing engine is currently unavailable. The profile was not deleted.'), 'error');
    $form_state['redirect'] = 'admin/config/system/code-sign';
    return;
  }

  try {
    // Give the engine a chance to veto or react to the deletion before the
    // profile's configuration is centrally removed.
    $engine->deleteProfile($form_state['values']['profile_id']);
    code_sign_delete_profile($form_state['values']['profile_id']);
    backdrop_set_message(t('Profile deleted.'));
  }
  catch (CodeSignException $e) {
    backdrop_set_message($e->getMessage(), 'error');
  }

  $form_state['redirect'] = 'admin/config/system/code-sign';
}

/**
 * Saves a Code Sign profile.
 *
 * @param array $values
 *   The submitted $form_state['values'], including the 'engine' and
 *   'profile' keys added by code_sign_profile_form(), after any
 *   engine-specific processing performed by CodeSignEngine::formSubmit().
 */
function code_sign_save_profile(array $values) {
  $internal_keys = array('form_build_id', 'form_token', 'form_id', 'op', 'submit', 'actions', 'profile_id');
  $values = array_diff_key($values, array_flip($internal_keys));

  $profile_id = $values['machine_name'];
  config('code_sign.profile.' . $profile_id)->setData($values)->save();
}

/**
 * Deletes a profile's configuration centrally.
 *
 * @param string $profile_id
 *   The machine name of the profile to delete.
 */
function code_sign_delete_profile($profile_id) {
  config('code_sign.profile.' . $profile_id)->delete();
}
