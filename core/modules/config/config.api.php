<?php
/**
 * @file
 * Documentation for hooks provided by Config module.
 *
 * @ingroup hooks
 * @{
 */

/**
 * Provides a list of configuration prefixes for describing configuration files.
 *
 * This hook may be used to expose individual configuration files in the UI for
 * exporting. Each entry in the returned array should contain at least the
 * following values:
 *   - label: A translated string for the name of the configuration file.
 *   - label_key: A string indicating the entry within the configuration file
 *       that will be used as a label.
 *   - group: A translated string to be used as the configuration group.
 */
function hook_config_info() {
  // If there are a large number of configuration files prefixed with this
  // string, provide a "name_key" that will be read from the configuration file
  // and used when listing the configuration file.
  $prefixes['image.styles'] = array(
    'name_key' => 'name',
    'label_key' => 'name',
    'group' => t('Image styles'),
  );
  // If this configuration file points to one particular file, a "name" key
  // will display that exact string for that file.
  $prefixes['system.performance'] = array(
    'label' => t('System performance'),
    'group' => t('Configuration'),
  );
  return $prefixes;
}

/**
 * Validate a new configuration before saving it.
 *
 * If any problems are encountered with the configuration, implementations of
 * this hook should throw a ConfigValidateException to prevent the configuration
 * from saving.
 *
 * @param Config $staging_config
 *   The configuration object for the settings about to be saved.
 * @param array|NULL $all_changes
 *   An array of all configuration changes that are pending, keyed by the config
 *   name with the value of either "create", "update", or "delete". This may be
 *   useful if one configuration's validation depends on another. This array
 *   will be a NULL value if only a single configuration is being imported.
 *
 * @throws ConfigValidateException
 */
function hook_config_create_validate(Config $staging_config, $all_changes) {
  if ($staging_config->getName() === 'mymodule.settings') {
    // Ensure that the name key is no longer than 64 characters.
    if (strlen($staging_config->get('name')) > 64) {
      throw new ConfigValidateException(t('The configuration "@file" must have a "name" attribute less than 64 characters.', array('@file' => $staging_config->getName())));
    }
  }
}

/**
 * Validate configuration changes before saving them.
 *
 * If any problems are encountered with the configuration, implementations of
 * this hook should throw a ConfigValidateException to prevent the configuration
 * from saving.
 *
 * @param Config $staging_config
 *   The configuration object for the settings about to be saved.
 * @param Config $active_config
 *   The current configuration object, whose values will be replaced.
 * @param array|NULL $all_changes
 *   An array of all configuration changes that are pending, keyed by the config
 *   name with the value of either "create", "update", or "delete". This may be
 *   useful if one configuration's validation depends on another. This array
 *   will be a NULL value if only a single configuration is being imported.
 *
 * @throws ConfigValidateException
 */
function hook_config_update_validate(Config $staging_config, Config $active_config, $all_changes) {
  if ($staging_config->getName() === 'mymodule.settings') {
    // Ensure that the name key is no longer than 64 characters.
    if (strlen($staging_config->get('name')) > 64) {
      throw new ConfigValidateException(t('The configuration "@file" must have a "name" attribute less than 64 characters.', array('@file' => $staging_config->getName())));
    }
  }
}

/**
 * Validate configuration deletions before deleting them.
 *
 * If any problems are encountered with the configuration, implementations of
 * this hook should throw a ConfigValidateException to prevent the configuration
 * from being deleted.
 *
 * @param Config $active_config
 *   The configuration object for the settings about to be deleted.
 * @param array|NULL $all_changes
 *   An array of all configuration changes that are pending, keyed by the config
 *   name with the value of either "create", "update", or "delete". This may be
 *   useful if one configuration's validation depends on another. This array
 *   will be a NULL value if only a single configuration is being imported.
 *
 * @throws ConfigValidateException
 */
function hook_config_delete_validate(Config $active_config, $all_changes) {
  if (strpos($active_config->getName(), 'image.styles') === 0) {
    // Check if another configuration depends on this configuration.
    if (!isset($all_changes['mymodule.settings']) || $all_changes['mymodule.settings'] !== 'delete') {
      $my_config = config('mymodule.settings');
      $image_style_name = $active_config->get('name');
      if ($my_config->get('image_style') === $image_style_name) {
        throw new ConfigValidateException(t('The configuration "@file" cannot be deleted because the image style "@style" is in use by "@mymodule".', array('@file' => $active_config->getName(), '@style' => $image_style_name, '@mymodule' => $my_config->getName())));
      }
    }
  }
}

/**
 * Respond to or modify configuration creation.
 *
 * @param Config $staging_config
 *   The configuration object for the settings about to be saved. This object
 *   is always passed by reference and may be modified to adjust the settings
 *   that are saved.
 */
function hook_config_create(Config $staging_config) {
  if (strpos($staging_config->getName(), 'image.styles') === 0) {
    // Set a value before the config is saved.
    $staging_config->set('some_key', 'default');
  }
}

/**
 * Respond to or modify configuration creation.
 *
 * @param Config $staging_config
 *   The configuration object for the settings about to be saved. This object
 *   is always passed by reference and may be modified to adjust the settings
 *   that are saved.
 * @param Config $active_config
 *   The configuration object for the settings being replaced.
 */
function hook_config_update(Config $staging_config, Config $active_config) {
  if (strpos($staging_config->getName(), 'image.styles') === 0) {
    // Set a value before the config is saved.
    if (is_null($active_config->get('some_key'))) {
      $staging_config->set('some_key', 'default');
    }
  }
}

/**
 * Respond to configuration deletion.
 *
 * This may be used when modules need to clean up data that is no longer needed
 * that is related to the configuration being deleted.
 *
 * @param Config $active_config
 *   The configuration object for the settings being deleted.
 */
function hook_config_delete(Config $active_config) {
  if (strpos($active_config->getName(), 'image.styles') === 0) {
    $image_style_name = $active_config->get('name');
    config('mymodule.image_style_addons.' . $image_style_name)->delete();
  }
}

/**
 * @} End of "ingroup hooks"
 */
