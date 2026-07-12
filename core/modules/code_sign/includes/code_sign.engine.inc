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
  public function __construct($name) {
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
  abstract public function sign($profile, $data, array $options = array());

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
  abstract public function verify($data, $signature = NULL, array $options = array());

  /**
   * Returns a render array with the current status of the engine.
   *
   * Used to build the report at admin/reports/code-sign.
   *
   * @return array
   *   A renderable array.
   */
  abstract public function getStatus();

  /**
   * Returns the list of profiles that can be used to sign data.
   *
   * @return array
   *   An array of signing profiles, indexed by a unique ID, such as a
   *   machine name or a key fingerprint. Each profile must include a 'name'
   *   key with a human-readable name of the profile.
   */
  abstract public function getPrivateKeychain();

  /**
   * Returns a list of "keys" that can be used to validate signatures.
   *
   * In a PKI engine, this would return the known public keys.
   *
   * @return array
   *   An array of available public keys.
   */
  abstract public function getPublicKeychain();

  /**
   * Builds the profile settings form for this engine.
   *
   * Modeled after the way the Block class builds its settings form: this
   * engine is only responsible for adding its own elements to $form, not for
   * the surrounding page, submit button, or persistence. The default
   * implementation provides the 'name' and 'machine_name' fields common to
   * most engines.
   *
   * Subclasses that need additional profile fields (such as a hash algorithm
   * or a generated keypair) should call
   * parent::form($form, $form_state, $profile) and add their own elements to
   * $form afterwards. Subclasses whose profiles are not created through this
   * form (for example, an engine backed by an external keyring) may override
   * this completely.
   *
   * @param array $form
   *   The form array to add elements to.
   * @param array $form_state
   *   The form state.
   * @param array $profile
   *   The existing profile being edited, or an empty array when adding a new
   *   profile.
   */
  public function form(array &$form, array &$form_state, array $profile = array()) {
    $form['#tree'] = TRUE;
    $form['profile']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Profile Name'),
      '#required' => TRUE,
      '#default_value' => isset($profile['name']) ? $profile['name'] : '',
      '#description' => t('The human-friendly name of the profile.'),
    );
    $form['profile']['machine_name'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine name'),
      '#default_value' => isset($profile['machine_name']) ? $profile['machine_name'] : '',
      '#description' => t('A unique name for the profile.'),
      '#machine_name' => array(
        'exists' => array($this, 'profileExists'),
        'source' => array('profile', 'name'),
        'label' => t('URL path'),
        'replace_pattern' => '[^a-z0-9-]+',
        'replace' => '-',
      ),
    );
  }

  /**
   * Validates the profile settings form for this engine.
   *
   * The default implementation performs no additional validation beyond the
   * machine name uniqueness check already built into the 'machine_name' form
   * element added by self::form().
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   The form state.
   */
  public function formValidate(array $form, array &$form_state) {
  }

  /**
   * Processes the submitted profile form and saves the resulting profile.
   *
   * Implementations are responsible for their own persistence, since the
   * storage mechanism (configuration, an external keyring, etc.) varies by
   * engine.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   The form state.
   */
  abstract public function formSubmit(array $form, array &$form_state);

  /**
   * Deletes a profile.
   *
   * @param string $profile_id
   *   The machine name of the profile to delete.
   *
   * @throws CodeSignException
   *   Thrown if this engine does not support deleting profiles.
   */
  abstract public function deleteProfile($profile_id);

  /**
   * Determines whether a profile with the given machine name already exists.
   *
   * @param string $machine_name
   *   The machine name to check.
   *
   * @return bool
   *   TRUE if a profile with this machine name exists, FALSE otherwise.
   */
  public function profileExists($machine_name) {
    $profiles = $this->getPrivateKeychain();
    return array_key_exists($machine_name, $profiles);
  }
}

/**
 * An exception class for Code Sign.
 */
class CodeSignException extends Exception {}
