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
  $info['multisite'] = array(
    'label' => t('Multisite'),
    'description' => t('Is this Backdrop installation set up as a multisite?'),
    'project' => 'backdrop',
  );
  $info['ckeditor_module_version'] = array(
    'label' => t('CKEditor Module Version'),
    'description' => t('The CKEditor module version that your site uses (either 4, 5, both, or neither).'),
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
    case 'install_profile':
      return backdrop_get_profile();
    case 'multisite':
      return conf_path() == '.' ? 'No' : 'Yes';
    case 'ckeditor_module_version':
      $ckeditor4 = module_exists('ckeditor');
      $ckeditor5 = module_exists('ckeditor5');
      if ($ckeditor4 && $ckeditor5) {
        return 'Both CKEditor 4 and 5';
      }
      elseif ($ckeditor4) {
        return 'CKEditor 4';
      }
      elseif ($ckeditor5) {
        return 'CKEditor 5';
      }
      else {
        return 'Neither';
      }
  }
}
