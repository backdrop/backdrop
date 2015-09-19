<?php

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 */

// Change the directory to the Backdrop root.
chdir('..');

/**
 * Root directory of Backdrop installation.
 */
define('BACKDROP_ROOT', getcwd());

include_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);

if (!isset($_GET['cron_key']) || state_get('cron_key') != $_GET['cron_key']) {
  watchdog('cron', 'Cron could not run because an invalid key was used.', array(), WATCHDOG_NOTICE);
  backdrop_access_denied();
}
elseif (state_get('maintenance_mode', FALSE)) {
  watchdog('cron', 'Cron could not run because the site is in maintenance mode.', array(), WATCHDOG_NOTICE);
  backdrop_access_denied();
}
else {
  backdrop_cron_run();
}
