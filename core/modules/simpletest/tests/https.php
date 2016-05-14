<?php

/**
 * @file
 * Fake an HTTPS request, for use during testing.
 */

// Set a global variable to indicate a mock HTTPS request.
$is_https_mock = empty($_SERVER['HTTPS']);

// Change to HTTPS.
$_SERVER['HTTPS'] = 'on';
foreach ($_SERVER as $key => $value) {
  $_SERVER[$key] = str_replace('core/modules/simpletest/tests/https.php', 'index.php', $value);
  $_SERVER[$key] = str_replace('http://', 'https://', $_SERVER[$key]);
}

// Change current directory to the Backdrop root.
chdir('../../../..');
define('BACKDROP_ROOT', getcwd());
require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';

// Make sure this file can only be used by simpletest.
backdrop_bootstrap(BACKDROP_BOOTSTRAP_CONFIGURATION);
if (!backdrop_valid_test_ua()) {
  header($_SERVER['SERVER_PROTOCOL'] . ' 403 Forbidden');
  exit;
}

backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);
menu_execute_active_handler();
