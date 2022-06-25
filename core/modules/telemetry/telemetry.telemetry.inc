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
  $info['theme_default'] = array(
    'label' => t('Default theme'),
    'description' => t('The default theme used on your site (includes any base themes).'),
    'project' => 'backdrop',
  );
  $info['theme_admin'] = array(
    'label' => t('Admin theme'),
    'description' => t('The administration theme used on your site (includes any base themes).'),
    'project' => 'backdrop',
  );
  $info['theme_node_admin'] = array(
    'label' => t('Admin theme used when adding/editing content'),
    'description' => t('Whether or not the admin theme is also used when creating and editing content.'),
    'project' => 'backdrop',
  );
  return $info;
}

/**
 * Implements hook_telemetry_data().
 */
function telemetry_telemetry_data($key) {
  $system_config = config('system.core');
  $all_themes = list_themes();

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
    case 'theme_default':
      $themes = array();
      $default_theme = $system_config->get('theme_default');
      $themes[] = $all_themes[$default_theme]->info['name'];
      if (!empty($all_themes[$default_theme]->base_themes)) {
        foreach (array_reverse($all_themes[$default_theme]->base_themes) as $base_theme) {
          $themes[] = $base_theme;
        }
      }
      return implode(', ', $themes);
    case 'theme_admin':
      $themes = array();
      $admin_theme = $system_config->get('admin_theme');
      $themes[] = $all_themes[$admin_theme]->info['name'];
      if (!empty($all_themes[$admin_theme]->base_themes)) {
        foreach (array_reverse($all_themes[$admin_theme]->base_themes) as $base_theme) {
          $themes[] = $base_theme;
        }
      }
      return implode(', ', $themes);
    case 'theme_node_admin':
      return $system_config->get('node_admin_theme') ? 'Yes' : 'No';
  }
}
