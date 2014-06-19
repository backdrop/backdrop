<?php

/**
 * @file
 * Initiates a browser-based installation of Backdrop.
 */

// Change the directory to the Backdrop root.
chdir('..');

/**
 * Defines the root directory of the Backdrop installation.
 */
define('BACKDROP_ROOT', getcwd());

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
