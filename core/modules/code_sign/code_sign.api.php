<?php
/**
 * @file
 * Hooks provided by the Code Sign module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Registers a signing engine with Code Sign.
 *
 * A signing engine provides the actual cryptographic implementation used to
 * sign and verify data. Code Sign itself does not implement any signing
 * engines; see the code_sign_hash module for a reference implementation.
 *
 * Each engine is implemented as a class extending CodeSignEngine. The class
 * must also be registered with hook_autoload_info().
 *
 * @return array
 *   An array of signing engines, keyed by the machine name of the engine.
 *   Each entry is an array with the following keys and values:
 *   - label: A human-readable name for the engine.
 *   - class: The name of the CodeSignEngine subclass that implements the
 *     engine.
 *
 * @see CodeSignEngine
 * @see hook_autoload_info()
 */
function hook_code_sign_info() {
  $signers['example'] = array(
    'label' => t('Example'),
    'class' => 'ExampleCodeSignEngine',
  );

  return $signers;
}

/**
 * @} End of "addtogroup hooks".
 */
