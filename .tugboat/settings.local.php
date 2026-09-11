<?php
/**
 * @file settings.local.php
 *
 * Local environment overrides and settings.
 */

// Database.
$database = array (
  'database' => 'tugboat',
  'username' => 'tugboat',
  'password' => 'tugboat',
  'host' => 'mariadb',
  'charset' => 'utf8mb4',
  'collation' => 'utf8mb4_general_ci',
);

// Config.
$config_directories['active'] = 'files/config/active';
$config_directories['staging'] = 'files/config/staging';

// Trusted hosts.
$settings['trusted_host_patterns'] = array('^.+\.tugboat\.qa$', '^.+\.tugboatqa\.com$');

// Disable sending Telemetry data on cron runs.
$settings['telemetry_enabled'] = FALSE;

// Show all error/warning messages.
$config['system.core']['error_level'] = 'all';

// Miscellaneous.
