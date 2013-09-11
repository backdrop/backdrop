<?php

/**
 * @file
 * Initiates a browser-based installation of Backdrop.
 */

// Change the directory to the Backdrop root.
chdir('..');

/**
 * Root directory of Backdrop installation.
 */
define('DRUPAL_ROOT', getcwd());

/**
 * Global flag to indicate that site is in installation mode.
 *
 * This constant is defined using define() instead of const so that PHP
 * versions older than 5.3 can display the proper PHP requirements instead of
 * causing a fatal error.
 */
define('MAINTENANCE_MODE', 'install');

// Exit early if running an incompatible PHP version to avoid fatal errors.
// The minimum version is specified explicitly, as DRUPAL_MINIMUM_PHP is not
// yet available. It is defined in bootstrap.inc, but it is not possible to
// load that file yet as it would cause a fatal error on older versions of PHP.
if (version_compare(PHP_VERSION, '5.3.2') < 0) {
  print 'Your PHP installation is too old. Backdrop requires at least PHP 5.3.2. See the <a href="http://backdrop.org/requirements">system requirements</a> page for more information.';
  exit;
}

// Start the installer.
require_once DRUPAL_ROOT . '/core/includes/install.core.inc';
install_backdrop();
