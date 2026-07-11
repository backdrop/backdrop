<?php
/**
 * @file
 * Contains the base class for Code Sign signing engines.
 */

/**
 * Base class for Code Sign signing engines.
 *
 * Modules that provide a signing engine must extend this class and register
 * it with hook_code_sign_info().
 *
 * @see hook_code_sign_info()
 */
abstract class CodeSignEngine {

  /**
   * The machine name of this signing engine.
   *
   * @var string
   */
  var $name;

  /**
   * Constructs a signing engine.
   *
   * @param string $name
   *   The machine name of the signing engine, as defined by the key used in
   *   hook_code_sign_info().
   */
  function __construct($name) {
    $this->name = $name;
  }

  /**
   * Signs data with the specified profile.
   *
   * @param string $profile
   *   The profile ID to use.
   * @param string $data
   *   The data to be signed.
   * @param array $options
   *   Engine-specific options.
   *
   * @return string|null
   *   A printable string with the signature, including all information
   *   needed to identify the signing key and the signature data. Returns
   *   NULL if the profile does not exist.
   */
  abstract function sign($profile, $data, array $options = array());

  /**
   * Verifies a signature.
   *
   * @param string $data
   *   The signed data.
   * @param string $signature
   *   The signature data, as created by self::sign().
   * @param array $options
   *   Engine-specific options.
   *
   * @return bool|null
   *   TRUE if the signature is valid, FALSE if it is invalid, or NULL if the
   *   signature is malformed.
   */
  abstract function verify($data, $signature = NULL, array $options = array());

  /**
   * Returns a render array with the current status of the engine.
   *
   * Used to build the report at admin/reports/code-sign.
   *
   * @return array
   *   A renderable array.
   */
  abstract function getStatus();

  /**
   * Returns the list of profiles that can be used to sign data.
   *
   * @return array
   *   An array of signing profiles, indexed by a unique ID, such as a
   *   machine name or a key fingerprint. Each profile must include a 'name'
   *   key with a human-readable name of the profile.
   */
  abstract function getPrivateKeychain();

  /**
   * Returns a list of "keys" that can be used to validate signatures.
   *
   * In a PKI engine, this would return the known public keys.
   *
   * @return array
   *   An array of available public keys.
   */
  abstract function getPublicKeychain();
}

/**
 * An exception class for Code Sign.
 */
class CodeSignException extends Exception {}
