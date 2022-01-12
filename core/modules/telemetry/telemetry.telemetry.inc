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
    'description' => t('The version of PHP running on your server.'),
    'project' => 'backdrop',
  );
  $info['mysql_version'] = array(
    'label' => t('Database version'),
    'description' => t('The version of your database server (either MySQL or MariaDB).'),
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
      return PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION . '.' . PHP_RELEASE_VERSION;
    case 'mysql_version':
      $version = Database::getConnection()->databaseType() === 'mysql' ? Database::getConnection()->version() : NULL;
      // Backdrop core only supports MySQL and MariaDB. MariaDB version strings
      // tend to have 2 versions, one for the equivalent MySQL version + another
      // for the actual MariaDB version. For example, in 5.5.5-10.5.11-MariaDB
      // 5.5.5 would denote the MySQL equivalency, and 10.5.11 would be the
      // actual MariaDB version.
      // Grab all strings within the version that are a sequence of numbers and
      // dots.
      preg_match_all('/[\d\.]+/', $version, $versions);
      // Only one version number detected. Report that.
      if (count($versions[0]) == 1) {
        $version = $versions[0][0];
      }
      // Multiple versions detected within the version string.
      else {
        // Some variations of the MariaDB version string contain the version
        // number more than once. For example:
        // 5.5.5-10.5.11-MariaDB-1:10.5.11+maria~focal-log.
        $versions = array_unique($versions[0]);
        // Sort versions in reverse (descending) order.
        rsort($versions, SORT_NATURAL);
        // Get the greatest version (or the only version).
        $version = $versions[0];
      }
      return $version;
    case 'server_os':
      return PHP_OS;
  }
}
