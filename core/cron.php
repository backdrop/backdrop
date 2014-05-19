<?php

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 */

// Change the directory to the Drupal root.
chdir('..');

/**
 * Root directory of Drupal installation.
 */
define('DRUPAL_ROOT', getcwd());

include_once DRUPAL_ROOT . '/core/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

if (!isset($_GET['cron_key']) || state_get('cron_key') != $_GET['cron_key']) {
  watchdog('cron', 'Cron could not run because an invalid key was used.', array(), WATCHDOG_NOTICE);
  drupal_access_denied();
}
elseif (state_get('maintenance_mode', 0)) {
  watchdog('cron', 'Cron could not run because the site is in maintenance mode.', array(), WATCHDOG_NOTICE);
  drupal_access_denied();
}
else {
  drupal_cron_run();
}
