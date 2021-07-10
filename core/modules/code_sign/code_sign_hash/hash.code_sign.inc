<?php
/**
 * @file
 * Implement all the code_sign callbacks for the hash signing engine.
 */

/**
 * Sign data with the specified profile.
 *
 * @param string $profile
 *   The profile ID to use.
 * @param string $data
 *   The data to be signed.
 * @param  array  $options
 *   Crypto engine specific options.
 * @return
 *   A printable string with the signature including all information needed
 *   to identify the signing key and the signature data.
 */
function code_sign_hash_sign($profile, $data, $options = array()) {
  $private_keychain = code_sign_hash_get_private_keychain();
  if (!array_key_exists($profile, $private_keychain)) {
    return NULL; // Throw?
  }

  $algo = $private_keychain[$profile]['hash_algorithm'];
  if (empty($algo)) {
    return NULL; // Throw?
  }

  return $algo . ':' . hash($algo, $data);
}

/**
 * Verify the signature.
 * @param $data
 *   The signed data.
 * @param $signature
 *   The signature data as created by code_sign_hash_sign()
 * @param  array $options
 *   Options for the signature.  Ignored in this module.
 * @return
 *   TRUE if it is valid, FALSE if it is invalid, NULL if it is malformed.
 */
function code_sign_hash_verify($data, $signature = NULL, $options = array()) {
  $sig_parts = explode(':', $signature);
  if (!is_array($sig_parts) || count($sig_parts) != 2) {
    return NULL;
  }

  $sig_algo = $sig_parts[0];
  $sig_hash = $sig_parts[1];

  return hash($sig_algo, $data) === $sig_hash;
}

/**
 * Provide a render array with whatever status the module wants to show.
 * This is used in the admin/reports/code-sign page.
 *
 * @return
 *   A render array with the engine status.
 */
function code_sign_hash_status() {
  $profiles = code_sign_hash_get_private_keychain();

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
 * Returns a list of "keys" that can be used to validate signatures. In a PKI
 *  crypto engine, this would return the known public keys.
 *
 * @return
 *   Array of available public keys.
 */
function code_sign_hash_get_public_keychain() {
  return code_sign_hash_algorithms();
}

/**
 * Returns a list of profiles that can be used to sign data.  The array is
 * is a list of entries keyed by the unique ID of the profile.  The code_sign_hash
 * module uses the profile's machine_name, a PKI crypto engine might use the
 * fingerprint of the key.
 *
 * Each profile must include a 'name' field, with a human-readable name of the
 * profile.  The code_sign_hash module uses the profile name provided by the
 * administrator.  PKI engine should use the key's common name.
 *
 * No other fields are defined for use by Code Sign.  Crypto engine writers can
 * be assured that any field not prefixed with a hash ('#') will be ignored by
 * Code Sign, and can be added to the array for internal use.
 *
 * @return
 *   An array of profiles, indexed by a unique name, such as a machine_name
 *   or a key fingerprint.
 */
function code_sign_hash_get_private_keychain() {
  $keychain = array();

  $profiles = config_get('code_sign_hash.settings', 'profiles');
  return $profiles;

  foreach ($profile as $key => $profile) {
    $keychain[$key] = array(
      'name' => $profile['name'],
      'algorithm' => $profile['hash_algorithm'],
    );
  }
  return $keychain;
}
