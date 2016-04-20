<?php

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 */

/**
 * Defines the root directory of the Backdrop installation.
 *
 * We are using dirname to get path to backdrop root folder without symlink resolving.
 * This way you can keep github repository out of the DOCROOT and have files directory
 * and settings.php out of github repository.
 * Relate to issues: #1297, #346.
 */
define('BACKDROP_ROOT', dirname(dirname($_SERVER['SCRIPT_FILENAME'])));

// Change the directory to the Backdrop root.
chdir(BACKDROP_ROOT);

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
