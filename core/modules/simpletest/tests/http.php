<?php

/**
 * @file
 * Fake an HTTP request, for use during testing.
 */

// Set a global variable to indicate a mock HTTP request.
$is_http_mock = !empty($_SERVER['HTTPS']);

// Change to HTTP.
$_SERVER['HTTPS'] = NULL;
ini_set('session.cookie_secure', FALSE);
foreach ($_SERVER as $key => $value) {
  $_SERVER[$key] = str_replace('core/modules/simpletest/tests/http.php', 'index.php', $value);
  $_SERVER[$key] = str_replace('https://', 'http://', $_SERVER[$key]);
}

// Change current directory to the Backdrop root.
chdir('../../../..');
define('DRUPAL_ROOT', getcwd());
require_once DRUPAL_ROOT . '/core/includes/bootstrap.inc';

// Make sure this file can only be used by simpletest.
backdrop_bootstrap(DRUPAL_BOOTSTRAP_CONFIGURATION);
if (!backdrop_valid_test_ua()) {
  header($_SERVER['SERVER_PROTOCOL'] . ' 403 Forbidden');
  exit;
}

backdrop_bootstrap(DRUPAL_BOOTSTRAP_FULL);
menu_execute_active_handler();
