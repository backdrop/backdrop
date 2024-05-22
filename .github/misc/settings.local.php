<?php
/**
 * @file
 * Settings applied during automated test runs.
 *
 * @see .github/workflows/simpletest.yml
 */

// Disable sending of telemetry data from GitHub Action runners.
$settings['telemetry_enabled'] = FALSE;

// Turn off Drupal compatibility layer for tests.
$settings['backdrop_drupal_compatibility'] = FALSE;

// Use a consistent config directory.
$config_directories['active'] = 'files/config/active';
$config_directories['staging'] = 'files/config/staging';
