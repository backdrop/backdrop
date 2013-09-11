<?php

/**
 * @file
 * The PHP page that serves all page requests on a Backdrop installation.
 *
 * The routines here dispatch control to the appropriate handler, which then
 * prints the appropriate page.
 *
 * All Backdrop code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt files in the "core" directory.
 */

/**
 * Root directory of Backdrop installation.
 */
define('DRUPAL_ROOT', getcwd());

require_once DRUPAL_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(DRUPAL_BOOTSTRAP_FULL);
menu_execute_active_handler();
