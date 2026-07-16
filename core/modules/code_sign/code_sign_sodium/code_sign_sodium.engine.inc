<?php
/**
 * @file
 * Contains the Sodium signing engine for Code Sign.
 */

// cspell:ignore PUBLICKEYBYTES publickey secretkey memzero

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
    if (!$this->isAvailable()) {
      watchdog('code_sign_sodium', 'The Sodium extension is not available.', array(), WATCHDOG_ERROR);
      return NULL;
    }

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
    if (!$this->isAvailable()) {
      watchdog('code_sign_sodium', 'The Sodium extension is not available.', array(), WATCHDOG_ERROR);
      return NULL;
    }

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
        check_plain($profile['label']),
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
    return $this->loadProfiles();
  }

  /**
   * {@inheritdoc}
   *
   * In addition to profile-backed keys, this merges in any public keys
   * bundled in this module's keys/ directory (files with a .pub extension,
   * each containing a base64-encoded Ed25519 public key). Bundling a key
   * this way trusts it for verification only; it never appears in
   * getPrivateKeychain() and can't be used to sign data.
   */
  public function getPublicKeychain() {
    $keychain = array();
    foreach ($this->getPrivateKeychain() as $key_id => $profile) {
      $keychain[$key_id] = array(
        'label' => $profile['label'],
        'public_key' => $profile['public_key'],
      );
    }

    if ($this->isAvailable()) {
      foreach ($this->loadBundledKeyFiles('code_sign_sodium', 'pub') as $key_name => $key_data) {
        $public_key = trim($key_data);
        $decoded = base64_decode($public_key, TRUE);
        if ($decoded === FALSE || strlen($decoded) !== SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES) {
          watchdog('code_sign_sodium', 'Bundled key file %file does not contain a valid Ed25519 public key.', array('%file' => $key_name), WATCHDOG_WARNING);
          continue;
        }
        $keychain['bundled:' . $key_name] = array(
          'label' => $key_name,
          'public_key' => $public_key,
        );
      }
    }

    return $keychain;
  }

  /**
   * {@inheritdoc}
   */
  public function isAvailable() {
    return extension_loaded('sodium');
  }

  /**
   * Generates a new Ed25519 keypair.
   *
   * @return array
   *   An array with 'public_key' and 'secret_key' keys, both base64-encoded.
   *
   * @throws CodeSignException
   * @throws SodiumException
   *   Thrown if the Sodium extension is not available.
   */
  public function generateKeypair() {
    if (!$this->isAvailable()) {
      throw new CodeSignException(t('The Sodium extension is not available.'));
    }

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

    if (empty($profile['name'])) {
      // Set a default machine name to match the date and algorithm.
      $date = new BackdropDateTime();
      $form['label']['#default_value'] = 'ed25519-' . $date->format('Y-m-d');
      $form['name']['#default_value'] = 'ed25519-' . $date->format('Y-m-d');
    }

    if (!empty($profile['public_key'])) {
      $form['public_key'] = array(
        '#type' => 'item',
        '#title' => t('Public key'),
        '#markup' => '<pre>' . check_plain($profile['public_key']) . '</pre>',
        '#description' => t('Share this key with anyone who needs to verify signatures created by this profile. The keypair itself cannot be changed; delete and re-add the profile to generate a new one.'),
      );
    }
    else {
      $form['info'] = array(
        '#type' => 'markup',
        '#markup' => '<p>' . t('A new Ed25519 keypair will be generated automatically when this profile is saved.') . '</p>',
      );
    }
  }

  /**
   * {@inheritdoc}
   *
   * Adds the keypair to the submitted values before they are centrally
   * persisted: a freshly generated one for a new profile, or the existing
   * one preserved for a profile being edited (the keypair itself cannot be
   * changed once created).
   */
  public function formSubmit(array $form, array &$form_state) {
    $original_profile_name = $form_state['values']['profile_name'];
    $original_profile = code_sign_load_profile($original_profile_name);

    if ($original_profile) {
      $keypair = array(
        'public_key' => $original_profile['public_key'],
        'secret_key' => $original_profile['secret_key'],
      );
    }
    else {
      try {
        $keypair = $this->generateKeypair();
      }
      catch (CodeSignException | SodiumException $e) {
        backdrop_set_message(t('Could not generate keypair: @error', array('@error' => $e->getMessage())), 'error');
        watchdog('code_sign_sodium', 'Could not generate keypair: @error', array('@error' => $e->getMessage()), WATCHDOG_ERROR);
        $keypair = array('public_key' => '', 'secret_key' => '');
      }
    }

    // @todo Do not store the secret key directly in config.
    // Ideally we would leverage the Key module to store the secret key in
    // a keystore location. For now, this secret key is stored in config but
    // should be pulled out of the code_sign.profile.[name].json file and stored
    // in settings.php as a config override, such as:
    // $config['code_sign.profile.ed25519-2026-07-15']['secret_key'] = 'secret key value';
    // See https://github.com/backdrop/backdrop-issues/issues/7082
    $form_state['values']['public_key'] = $keypair['public_key'];
    $form_state['values']['secret_key'] = $keypair['secret_key'];
  }
}
