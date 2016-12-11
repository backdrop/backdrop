<?php

/**
 * @file
 * Administrative page for running menu_rebuild.
 *
 * Point your browser to "http://www.example.com/core/menu_rebuild.php" and follow the
 * instructions.
 *
 * If you are not logged in using either the site maintenance account or an
 * account with the "Administer software updates" permission, you will need to
 * modify the access check statement inside your settings.php file. After
 * finishing the upgrade, be sure to open settings.php again, and change it
 * back to its original state!
 */

/**
 * Defines the root directory of the Backdrop installation.
 *
 * The dirname() function is used to get path to Backdrop root folder, which
 * avoids resolving of symlinks. This allows the code repository to be a symlink
 * and hosted outside of the web root. See issue #1297.
 */
define('BACKDROP_ROOT', dirname(dirname($_SERVER['SCRIPT_FILENAME'])));
// Change the directory to the Backdrop root.
chdir(BACKDROP_ROOT);

require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);

menu_router_build();
