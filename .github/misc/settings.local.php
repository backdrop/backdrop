<?php
/**
 * @file
 * Settings applied during automated test runs.
 *
 * @see .github/workflows/functional-tests.yml
 */

// Disable sending of telemetry data from GitHub Action runners.
$settings['telemetry_enabled'] = FALSE;

// Turn off Drupal compatibility layer for tests.
$settings['backdrop_drupal_compatibility'] = FALSE;
