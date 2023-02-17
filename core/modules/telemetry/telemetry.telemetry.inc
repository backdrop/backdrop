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
  $info['web_server'] = array(
    'label' => t('Web server'),
    'description' => t('The web server that delivers HTTP pages to the web browser, such as Apache or nginx.'),
    'project' => 'backdrop',
  );
  $info['install_profile'] = array(
    'label' => t('Installation profile'),
    'description' => t('The profile that was used to install your site.'),
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
      // Trim the version number to only major + minor, otherwise we'd flood the
      // table with dozens of bug fix versions.
      $db_info = _system_get_database_version();
      $version = explode('.', $db_info['version']);
      $version = array_slice($version, 0, 2);
      $version = implode('.', $version);
      return $db_info['type'] . ' ' . $version;
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
    case 'install_profile':
      return backdrop_get_profile();
  }
}
