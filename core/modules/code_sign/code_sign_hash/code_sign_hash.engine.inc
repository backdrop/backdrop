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
      $rows[] = array_intersect_key($p, array_flip(array('label', 'hash_algorithm')));
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
    return $this->loadProfiles();
  }

  /**
   * {@inheritdoc}
   */
  public function getPublicKeychain() {
    return code_sign_hash_algorithms();
  }

  /**
   * {@inheritdoc}
   *
   * The hash engine has no external dependencies; it is always available.
   */
  public function isAvailable() {
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function form(array &$form, array &$form_state, array $profile = array()) {
    parent::form($form, $form_state, $profile);

    // Set a default machine name to match the hashing algorithm.
    if (empty($profile['label'])) {
      $form['label']['#default_value'] = 'sha256';
      $form['name']['#default_value'] = 'sha256';
    }

    $form['hash_algorithm'] = array(
      '#type' => 'select',
      '#title' => t('Signing Hash Algorithm'),
      '#default_value' => $profile['hash_algorithm'] ?? 'sha256',
      '#options' => code_sign_hash_algorithms(),
    );
  }
}
