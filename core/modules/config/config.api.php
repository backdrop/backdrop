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
    'label_key' => 'name',
    'group' => t('Image styles')
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
 * @} End of "ingroup hooks"
 */
