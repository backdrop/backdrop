<?php
/**
 * @file
 * Telemetry hook implementations.
 */

/**
 * Implements hook_telementry_info().
 */
function telemetry_telemetry_info() {
  $info['php_version'] = array(
    'label' => t('PHP Version'),
    'description' => t('The current version of PHP running on your server.'),
    'project' => 'backdrop',
  );
  $info['server_os'] = array(
    'label' => t('Server OS'),
    'description' => t('The operating system of your server, such as RedHat Linux, Debian, Windows, etc.'),
    'project' => 'backdrop',
  );
  return $info;
}

/**
 * Implements hook_telemetry_data().
 */
function telemetry_telemetry_data($key) {
  switch ($key) {
    case 'php_version':
      return PHP_VERSION;
    case 'server_os':
      return PHP_OS;
  }
}
