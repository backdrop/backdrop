<?php
/**
 * @file
 * Telemetry hook implementations.
 */

/**
 * Implements hook_telemetry_info().
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

  // Telemetry currently cannot handle arrays of data, so we need to report back
  // each core module as an individual metric.
  $module_info = system_rebuild_module_data();
  $telemetry_modules = _telemetry_core_module_list();
  foreach ($telemetry_modules as $module) {
    $info[$module] = array(
      'label' => t('Core module: @module', array('@module' => $module_info[$module]->info['name'])),
      'description' => $module_info[$module]->info['description'],
      'project' => 'backdrop',
    );
  }

  return $info;
}

/**
 * Helper function that returns a list of core module that we are interested to
 * get metrics on whether they are enabled/disabled on the majority of sites.
 */
function _telemetry_core_module_list() {
  return array(
    'book',
    'contact',
    'translation',
    'entityreference',
    'language',
    'locale',
    'syslog',
    // 'simpletest',
  );
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
    case 'install_profile':
      return backdrop_get_profile();
  }

  $telemetry_modules = _telemetry_core_module_list();
  if (in_array($key, $telemetry_modules)) {
    return module_exists($key) ? 'enabled' : 'disabled';
  }
}
