<?php

define('DRUPAL_ROOT', getcwd());
require_once DRUPAL_ROOT . '/core/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$modules = system_rebuild_module_data();
foreach ($modules as $module) {
  write_yaml($module->info, str_replace('.module', '.info.yml', $module->uri));
}

$themes = system_rebuild_theme_data();
foreach ($themes as $themes) {
  write_yaml($module->info, str_replace('.module', '.info.yml', $module->uri));
}

function write_yaml($info, $uri) {
  // Clean up the information.
  $keys = array(
    'name',
    'description',
    'core',
    'package',
    'configure',
    'version',
    'hidden',
    'php',
    'dependencies',
    'stylesheets',
    'scripts',
    'files',
  );
  $new_info = array();
  unset($info['bootstrap']);
  if ($info['version'] == '8.0-dev') {
    $info['version'] = 'VERSION';
  }
  if ($info['php'] == '5.3.2') {
    unset($info['php']);
  }
  foreach ($keys as $key) {
    if (empty($info[$key])) {
      unset($info[$key]);
    }
    else {
      $new_info[$key] = $info[$key];
    }
  }
  $info = array_merge($new_info, $info);
  $yaml = Spyc::YAMLDump($info, 2, 0, TRUE);
  file_put_contents($uri, $yaml);
}
