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
   *   machine name or a key fingerprint. Each profile must include a 'label'
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
   * Determines whether this engine is currently usable.
   *
   * Implementations should check for required PHP extensions, external
   * binaries, or other runtime dependencies, without any side effects (such
   * as logging) since this may be called frequently, including on every page
   * view of the centralized profile administration pages. Centralized
   * profile management uses this to skip an engine entirely rather than
   * calling into it and risking a fatal error.
   *
   * @return bool
   *   TRUE if the engine can be used, FALSE otherwise.
   */
  abstract public function isAvailable();

  /**
   * Builds the profile settings form for this engine.
   *
   * Modeled after the way the Block class builds its settings form: this
   * engine is only responsible for adding its own elements to $form, not for
   * the surrounding page, submit button, or persistence. The default
   * implementation provides the 'label' and 'name' fields common to most
   * engines.
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
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Profile Name'),
      '#required' => TRUE,
      '#default_value' => isset($profile['label']) ? $profile['label'] : '',
      '#description' => t('The human-friendly name of the profile.'),
    );
    $form['name'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine name'),
      '#default_value' => isset($profile['name']) ? $profile['name'] : '',
      '#disabled' => isset($profile['name']),
      '#description' => t('A unique name for the profile.'),
      '#machine_name' => array(
        'exists' => array($this, 'profileExists'),
        'source' => array('label'),
        'label' => t('Machine name'),
        'replace_pattern' => '[^a-z0-9-]+',
        'replace' => '-',
      ),
    );
  }

  /**
   * Validates the profile settings form for this engine.
   *
   * The default implementation performs no additional validation beyond the
   * machine name uniqueness check already built into the 'name' form
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
   * Processes the submitted profile form before it is centrally persisted.
   *
   * Unlike the rest of the profile lifecycle, persistence is not the
   * engine's responsibility: code_sign_save_profile() centrally saves the
   * entire contents of $form_state['values'] (minus internal Form API keys)
   * into a single configuration file, code_sign.profile.[machine_name]. This
   * method exists only so an engine can process or augment those values
   * first - for example, generating a keypair for a new profile, or
   * preserving one for an existing profile being renamed. The default
   * implementation does nothing, since most profile fields (such as those
   * added by self::form()) need no further processing.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   The form state.
   *
   * @see code_sign_save_profile()
   */
  public function formSubmit(array $form, array &$form_state) {
  }

  /**
   * Allows the engine to react to or veto deletion of one of its profiles.
   *
   * Removal of the profile's configuration file is handled centrally by
   * code_sign_delete_profile(); this method exists so an engine can perform
   * its own cleanup, or refuse the deletion entirely (by throwing
   * CodeSignException) when its profiles are not simple configuration-backed
   * records - for example, an engine backed by an external keyring. The
   * default implementation does nothing, allowing the deletion to proceed.
   *
   * @param string $profile_name
   *   The machine name of the profile to delete.
   *
   * @throws CodeSignException
   *   Thrown if this engine does not allow the profile to be deleted.
   *
   * @see code_sign_delete_profile()
   */
  public function deleteProfile($profile_name) {
  }

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

  /**
   * Loads all profiles belonging to this engine from the centralized store.
   *
   * Profiles are saved by code_sign_save_profile() into individual
   * configuration files named code_sign.profile.[machine_name], each
   * recording which engine it belongs to. This helper is available to any
   * engine whose profiles are stored this way; engines with a different
   * storage mechanism (such as one backed by an external keyring) should
   * override getPrivateKeychain() directly instead.
   *
   * @return array
   *   An array of profiles, indexed by machine name.
   *
   * @see code_sign_save_profile()
   */
  protected function loadProfiles() {
    $profiles = array();

    $all_profiles = code_sign_load_all_profiles();
    foreach ($all_profiles as $profile_name => $profile) {
      if ($profile['engine'] === $this->name) {
        $profiles[$profile_name] = $profile;
      }
    }

    return $profiles;
  }
}

/**
 * An exception class for Code Sign.
 */
class CodeSignException extends Exception {}
