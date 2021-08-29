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
    'label' => t('PHP version'),
    'description' => t('The current version of PHP running on your server.'),
    'project' => 'backdrop',
  );
  $info['mysql_version'] = array(
    'label' => t('MySQL version'),
    'description' => t('The version number of your database (either MySQL or MariaDB).'),
    'project' => 'backdrop',
  );
  $info['server_os'] = array(
    'label' => t('Server OS'),
    'description' => t('The operating system of your server, such as RedHat Linux, Debian, Windows, etc.'),
    'project' => 'backdrop',
  );
  $info['installation_profile'] = array(
    'label' => t('Installation profile'),
    'description' => t('The profile that was used to install this site.'),
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
      return PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION . '.' . PHP_RELEASE_VERSION;
    case 'mysql_version':
      return Database::getConnection()->databaseType() === 'mysql' ? Database::getConnection()->version() : NULL;
    case 'server_os':
      return PHP_OS;
    case 'installation_profile':
      return backdrop_get_profile();
  }
}
