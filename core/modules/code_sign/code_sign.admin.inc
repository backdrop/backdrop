<?php

/**
 * @file
 * Central administrative pages for Code Sign profile management.
 *
 * Individual signing engines do not provide their own profile management UI.
 * Instead, each engine's class provides form(), formValidate(), and
 * formSubmit() methods (similar to the way the Block class builds its
 * settings form), which are invoked generically below. This allows profiles
 * from any number of enabled Code Sign engines to be listed, added, edited,
 * and deleted together on one set of pages.
 *
 * @see CodeSignEngine::form()
 * @see CodeSignEngine::formValidate()
 * @see CodeSignEngine::formSubmit()
 * @see CodeSignEngine::deleteProfile()
 */

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
  $add_links = array();
  foreach ($all_signers as $engine_name => $info) {
    $engine = code_sign_get_engine($engine_name);
    if (!$engine) {
      continue;
    }

    foreach ($engine->getPrivateKeychain() as $profile_id => $profile) {
      $rows[] = array(
        !empty($profile['name']) ? check_plain($profile['name']) : $profile_id,
        check_plain($info['label']),
        l(t('Edit'), 'admin/config/system/code-sign/profile/' . $engine_name . '/' . $profile_id . '/edit') . ' ' .
          l(t('Delete'), 'admin/config/system/code-sign/profile/' . $engine_name . '/' . $profile_id . '/delete'),
      );
    }

    $add_links[] = l(t('Add @label profile', array('@label' => $info['label'])), 'admin/config/system/code-sign/profile/' . $engine_name . '/add');
  }

  $form['add'] = array(
    '#type' => 'markup',
    '#markup' => theme('item_list', array('items' => $add_links)),
  );
  $form['profiles'] = array(
    '#theme' => 'table',
    '#header' => array(t('Profile name'), t('Engine'), t('Operations')),
    '#rows' => $rows,
    '#empty' => t('No profiles defined.'),
  );

  return $form;
}

/**
 * Page callback: Builds the add/edit profile form for a signing engine.
 *
 * @param string $engine_name
 *   The machine name of the signing engine, as defined by
 *   hook_code_sign_info().
 * @param string $profile_id
 *   The machine name of the profile to edit, or NULL to add a new profile.
 *
 * @return array|int
 *   A form render array, or MENU_NOT_FOUND if the engine or profile is
 *   unknown.
 */
function code_sign_profile_edit_page($engine_name, $profile_id = NULL) {
  $engine = code_sign_get_engine($engine_name);
  if (!$engine) {
    return MENU_NOT_FOUND;
  }
  if (isset($profile_id)) {
    $profiles = $engine->getPrivateKeychain();
    if (!isset($profiles[$profile_id])) {
      return MENU_NOT_FOUND;
    }
  }

  return backdrop_get_form('code_sign_profile_form', $engine_name, $profile_id);
}

/**
 * Builds the add/edit profile form, delegating to the engine's form().
 *
 * @param array $form
 *   The initial form.
 * @param array &$form_state
 *   The form state.
 * @param string $engine_name
 *   The machine name of the signing engine.
 * @param string $profile_id
 *   The machine name of the profile to edit, or NULL to add a new profile.
 *
 * @return array
 *   The profile form.
 */
function code_sign_profile_form($form, &$form_state, $engine_name, $profile_id = NULL) {
  $engine = code_sign_get_engine($engine_name);
  $profile = array();
  if (isset($profile_id)) {
    $all_profiles = $engine->getPrivateKeychain();
    $profile = $all_profiles[$profile_id];
  }

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
      '#value' => isset($profile_id) ? t('Save profile') : t('Add profile'),
    ),
  );

  return $form;
}

/**
 * Validate handler for code_sign_profile_form().
 */
function code_sign_profile_form_validate($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  $engine->formValidate($form, $form_state);
}

/**
 * Submit handler for code_sign_profile_form().
 */
function code_sign_profile_form_submit($form, &$form_state) {
  $engine = code_sign_get_engine($form_state['values']['engine']);
  $engine->formSubmit($form, $form_state);

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

  try {
    $engine->deleteProfile($form_state['values']['profile_id']);
    backdrop_set_message(t('Profile deleted.'));
  }
  catch (CodeSignException $e) {
    backdrop_set_message($e->getMessage(), 'error');
  }

  $form_state['redirect'] = 'admin/config/system/code-sign';
}
