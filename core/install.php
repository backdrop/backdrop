<?php

/**
 * @file
 * Initiates a browser-based installation of Backdrop.
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

/**
 * Global flag to indicate the site is in installation mode.
 */
define('MAINTENANCE_MODE', 'install');

// Exit early if running an incompatible PHP version to avoid fatal errors.
// The minimum version is specified explicitly, as BACKDROP_MINIMUM_PHP is not
// yet available. It is defined in bootstrap.inc, but it is not possible to
// load that file yet as it would cause a fatal error on older versions of PHP.
if (version_compare(PHP_VERSION, '5.3.2') < 0) {
  print 'Your PHP installation is too old. Backdrop CMS requires at least PHP 5.3.2. See the <a href="http://backdropcms.org/guide/requirements">system requirements</a> page for more information.';
  exit;
}

// Start the installer.
require_once BACKDROP_ROOT . '/core/includes/install.core.inc';
install_backdrop();
