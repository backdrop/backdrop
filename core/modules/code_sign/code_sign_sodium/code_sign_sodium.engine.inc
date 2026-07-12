<?php
/**
 * @file
 * Contains the Sodium signing engine for Code Sign.
 */

/**
 * A Code Sign engine that signs and verifies data using Sodium (Ed25519).
 *
 * Each profile holds its own Ed25519 keypair. A signature embeds the public
 * key it was created with (base64-encoded) alongside the raw signature, so
 * verification can identify the correct key without trying every key in the
 * keychain. The embedded key is only trusted if it also exists in the local
 * keychain.
 */
class CodeSignSodiumEngine extends CodeSignEngine {

  /**
   * {@inheritdoc}
   */
  public function sign($profile, $data, array $options = array()) {
    $private_keychain = $this->getPrivateKeychain();
    if (!array_key_exists($profile, $private_keychain)) {
      return NULL;
    }

    $secret_key = base64_decode($private_keychain[$profile]['secret_key']);
    $public_key = base64_decode($private_keychain[$profile]['public_key']);

    try {
      $raw_signature = sodium_crypto_sign_detached($data, $secret_key);
    }
    catch (SodiumException $e) {
      watchdog('code_sign_sodium', 'Could not generate signature: @error', array('@error' => $e->getMessage()), WATCHDOG_WARNING);
      return NULL;
    }

    return base64_encode($public_key) . ':' . base64_encode($raw_signature);
  }

  /**
   * {@inheritdoc}
   */
  public function verify($data, $signature = NULL, array $options = array()) {
    $parts = explode(':', (string) $signature, 2);
    if (count($parts) != 2) {
      return NULL;
    }

    list($encoded_public_key, $encoded_signature) = $parts;
    $public_key = base64_decode($encoded_public_key, TRUE);
    $raw_signature = base64_decode($encoded_signature, TRUE);
    if ($public_key === FALSE || $raw_signature === FALSE) {
      return NULL;
    }

    // Only trust a public key that is embedded in the signature if it is
    // also present in our own keychain.
    $trusted = FALSE;
    foreach ($this->getPublicKeychain() as $key) {
      if (hash_equals($key['public_key'], $encoded_public_key)) {
        $trusted = TRUE;
        break;
      }
    }
    if (!$trusted) {
      return FALSE;
    }

    try {
      return sodium_crypto_sign_verify_detached($raw_signature, $data, $public_key);
    }
    catch (SodiumException $e) {
      return NULL;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getStatus() {
    $rows = array();
    foreach ($this->getPrivateKeychain() as $profile) {
      $rows[] = array(
        check_plain($profile['name']),
        check_plain($profile['public_key']),
      );
    }

    return array(
      '#theme' => 'table',
      '#header' => array(t('Profile Name'), t('Public Key')),
      '#rows' => $rows,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getPrivateKeychain() {
    $profiles = config_get('code_sign_sodium.settings', 'profiles');
    return $profiles ? $profiles : array();
  }

  /**
   * {@inheritdoc}
   */
  public function getPublicKeychain() {
    $keychain = array();
    foreach ($this->getPrivateKeychain() as $key_id => $profile) {
      $keychain[$key_id] = array(
        'name' => $profile['name'],
        'public_key' => $profile['public_key'],
      );
    }

    return $keychain;
  }

  /**
   * Generates a new Ed25519 keypair.
   *
   * @return array
   *   An array with 'public_key' and 'secret_key' keys, both base64-encoded.
   */
  public function generateKeypair() {
    $keypair = sodium_crypto_sign_keypair();

    $keys = array(
      'public_key' => base64_encode(sodium_crypto_sign_publickey($keypair)),
      'secret_key' => base64_encode(sodium_crypto_sign_secretkey($keypair)),
    );

    sodium_memzero($keypair);

    return $keys;
  }

  /**
   * {@inheritdoc}
   */
  public function form(array &$form, array &$form_state, array $profile = array()) {
    parent::form($form, $form_state, $profile);

    if (!empty($profile['public_key'])) {
      $form['profile']['public_key'] = array(
        '#type' => 'item',
        '#title' => t('Public key'),
        '#markup' => '<pre>' . check_plain($profile['public_key']) . '</pre>',
        '#description' => t('Share this key with anyone who needs to verify signatures created by this profile. The keypair itself cannot be changed; delete and re-add the profile to generate a new one.'),
      );
    }
    else {
      $form['profile']['info'] = array(
        '#type' => 'markup',
        '#markup' => '<p>' . t('A new Ed25519 keypair will be generated automatically when this profile is saved.') . '</p>',
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function formSubmit(array $form, array &$form_state) {
    $values = $form_state['values']['profile'];
    $profile_id = $values['machine_name'];
    $original_profile_id = $form_state['values']['profile_id'];

    $all_profiles = $this->getPrivateKeychain();

    // Editing an existing profile keeps its existing keypair; a new profile
    // gets a freshly generated one.
    if (isset($all_profiles[$original_profile_id])) {
      $keypair = array(
        'public_key' => $all_profiles[$original_profile_id]['public_key'],
        'secret_key' => $all_profiles[$original_profile_id]['secret_key'],
      );
    }
    else {
      $keypair = $this->generateKeypair();
    }

    $all_profiles[$profile_id] = array(
      'name' => $values['name'],
      'machine_name' => $profile_id,
      'public_key' => $keypair['public_key'],
      'secret_key' => $keypair['secret_key'],
    );

    // If the machine_name changes, delete the original.
    if ($original_profile_id && $original_profile_id != $profile_id) {
      unset($all_profiles[$original_profile_id]);
    }

    config_set('code_sign_sodium.settings', 'profiles', $all_profiles);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteProfile($profile_id) {
    config_clear('code_sign_sodium.settings', 'profiles.' . $profile_id);
  }
}
