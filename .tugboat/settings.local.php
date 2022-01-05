<?php
// Database.
$database = 'mysql://tugboat:tugboat@mariadb/tugboat';
$database_charset = 'utf8mb4';

// Config.
$config_directories['active'] = 'files/config/active';
$config_directories['staging'] = 'files/config/staging';

// Trusted hosts.
$settings['trusted_host_patterns'] = array('^.+\.tugboat\.qa$');

// Miscellaneous.
