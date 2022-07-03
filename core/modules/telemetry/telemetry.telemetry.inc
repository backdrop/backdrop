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
  $info['mysql_version'] = array(
    'label' => t('MySQL Version'),
    'description' => t('The version number of your database (either MySQL or MariaDB).'),
    'project' => 'backdrop',
  );
  $info['server_os'] = array(
    'label' => t('Server OS'),
    'description' => t('The operating system of your server, such as RedHat Linux, Debian, Windows, etc.'),
    'project' => 'backdrop',
  );
  $info['web_server'] = array(
    'label' => t('Web server'),
    'description' => t('The web server that delivers HTTP pages to the web browser, such as Apache or nginx.'),
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
    case 'web_server':
      if (backdrop_is_apache()) {
        return 'Apache';
      }
      elseif (backdrop_is_nginx()) {
        return 'nginx';
      }
      elseif (backdrop_is_litespeed()) {
        return 'LiteSpeed';
      }
      else {
        return 'Other';
      }
  }
}
