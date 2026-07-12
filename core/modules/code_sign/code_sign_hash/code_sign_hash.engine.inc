<?php
/**
 * @file
 * Contains the hash signing engine for Code Sign.
 */

/**
 * A Code Sign engine that "signs" data using a simple hash.
 *
 * This is not a cryptographically secure signing method, it is provided as a
 * reference implementation and for use in automated tests.
 */
class CodeSignHashEngine extends CodeSignEngine {

  /**
   * {@inheritdoc}
   */
  public function sign($profile, $data, array $options = array()) {
    $private_keychain = $this->getPrivateKeychain();
    if (!array_key_exists($profile, $private_keychain)) {
      return NULL;
    }

    $algo = $private_keychain[$profile]['hash_algorithm'];
    if (empty($algo)) {
      return NULL;
    }

    return $algo . ':' . hash($algo, $data);
  }

  /**
   * {@inheritdoc}
   */
  public function verify($data, $signature = NULL, array $options = array()) {
    $sig_parts = explode(':', $signature);
    if (!is_array($sig_parts) || count($sig_parts) != 2) {
      return NULL;
    }

    list($sig_algo, $sig_hash) = $sig_parts;

    return hash($sig_algo, $data) === $sig_hash;
  }

  /**
   * {@inheritdoc}
   */
  public function getStatus() {
    $profiles = $this->getPrivateKeychain();

    $rows = array();
    foreach ($profiles as $p) {
      $rows[] = array_intersect_key($p, array_flip(array('name', 'hash_algorithm')));
    }

    $header = array(
      t('Profile Name'),
      t('Hash Algorithm'),
    );

    return array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getPrivateKeychain() {
    return config_get('code_sign_hash.settings', 'profiles');
  }

  /**
   * {@inheritdoc}
   */
  public function getPublicKeychain() {
    return code_sign_hash_algorithms();
  }

  /**
   * {@inheritdoc}
   */
  public function form(array &$form, array &$form_state, array $profile = array()) {
    parent::form($form, $form_state, $profile);

    $form['profile']['hash_algorithm'] = array(
      '#type' => 'select',
      '#title' => t('Signing Hash Algorithm'),
      '#default_value' => isset($profile['hash_algorithm']) ? $profile['hash_algorithm'] : 'sha256',
      '#options' => code_sign_hash_algorithms(),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function formSubmit(array $form, array &$form_state) {
    $values = $form_state['values']['profile'];
    $profile_id = $values['machine_name'];
    $original_profile_id = $form_state['values']['profile_id'];

    $all_profiles = $this->getPrivateKeychain();
    $all_profiles[$profile_id] = array(
      'name' => $values['name'],
      'machine_name' => $profile_id,
      'hash_algorithm' => $values['hash_algorithm'],
    );

    // If the machine_name changes, delete the original.
    if ($original_profile_id && $original_profile_id != $profile_id) {
      unset($all_profiles[$original_profile_id]);
    }

    config_set('code_sign_hash.settings', 'profiles', $all_profiles);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteProfile($profile_id) {
    config_clear('code_sign_hash.settings', 'profiles.' . $profile_id);
  }
}
