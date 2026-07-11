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
  function sign($profile, $data, array $options = array()) {
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
  function verify($data, $signature = NULL, array $options = array()) {
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
  function getStatus() {
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
  function getPrivateKeychain() {
    return config_get('code_sign_hash.settings', 'profiles');
  }

  /**
   * {@inheritdoc}
   */
  function getPublicKeychain() {
    return code_sign_hash_algorithms();
  }
}
