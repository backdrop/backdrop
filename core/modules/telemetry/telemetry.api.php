<?php
/**
 * @file
 * Hooks provided by the Telemetry module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Provides information about data collected by this module.
 *
 * @return array $info
 *   An array of Telemetry data that will be collected. Keyed by a unique
 *   identifier. Contributed modules should prefix this key with the name of
 *   their module, such as "example_setting_1" for a module named "example".
 *   Values within the array include the keys:
 *   - label: A translated label for the data being collected.
 *   - description: A translated description of the data being collected.
 *   - project: The name of the project on BackdropCMS.org with which this value
 *     should be associated. Use this if the project name is not the same as
 *     the module name, or if the project contains multiple modules.
 */
function hook_telemetry_info() {
  // If this were in my_module.module, "project" would assume to be "my_module".
  $info['my_module_setting_1'] = array(
    'label' => t('My module setting 1'),
    'description' => t('Some information about what setting 1 is.'),
  );
  // If this were in submodule.module, specify the project name explicitly.
  $info['my_module_submodule_setting_2'] = array(
    'label' => t('Submodule setting 2'),
    'description' => t('A description describing this setting of a sub-module.'),
    'project' => 'my_module',
  );
  return $info;
}

/**
 * Alter the list of data collected by Telemetry.
 *
 * @param array $info
 *   A list of telemetry data to be collected. Keyed as described in
 *   hook_telemetry_info(). In addition to the keys there, the following
 *   additional keys are added prior to this hook being invoked:
 *   - module: The machine-name of the providing module for this bit of data.
 */
function hook_telemetry_info_alter(array &$info) {
  // Prevent this site from reporting server OS:
  unset($info['server_os']);

  // Prevent one particular module from reporting anything.
  foreach ($info as $info_key => $info_data) {
    if ($info_data['module'] === 'my_module') {
      unset($info[$info_key]);
    }
  }
}

/**
 * Populates the telemetry data from this module.
 *
 * Data collected in this hook should be entirely anonymous. Returning data
 * such as IP Addresses, personal information, the site name, or other values
 * is not allowed. Contributed modules that return such information may be
 * subject to removal from the Backdrop Contributed module repository.
 *
 * Only one piece of Telemetry data is requested at a time, with $telemetry_key
 * specifying which value should be returned. If a module provides a lot of
 * Telemetry data, this function may be called many times with different keys
 * to collect all of the current information.
 *
 * @param string $telemetry_key
 *   Unique identifier of the data to be collected.
 *
 * @return string
 *   The value for the requested data.
 */
function hook_telemetry_data($telemetry_key) {
  switch ($telemetry_key) {
    case 'php_version':
      return PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION . '.' . PHP_RELEASE_VERSION;
    case 'my_module_setting_1':
      return config_get('my_module.settings', 'setting_1');
  }
}

/**
 * Alter the complete list of values collected by Telemetry before sending.
 *
 * This hook should rarely be used but is provided for completeness and
 * consistency. Modifications to the returned data may skew analytics collected
 * by the central server (usually BackdropCMS.org).
 *
 * To prevent data from being collected in the first place, use
 * hook_telemetry_info_alter() and remove the key. Or simply disable the
 * Telemetry module entirely.
 *
 * @param array $telemetry_data
 */
function hook_telemetry_data_alter(array $telemetry_data) {
  if (isset($telemetry_data['my_module_setting_1'])) {
    $telemetry_data['my_module_setting_1'] = 'some_different_value';
  }
}

/**
 * @} End of "addtogroup hooks".
 */
