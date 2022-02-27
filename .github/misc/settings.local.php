<?php
/**
 * @file
 * Disable sending of telemetry data from GitHub Action runners.
 * @see .github/workflows/functional-tests.yml
 */

$settings['telemetry_enabled'] = FALSE;
