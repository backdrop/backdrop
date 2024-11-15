<?php
/**
 * @file
 * Contains functionality related to creating and restoring site backups.
 */

/**
 * Get all backup/restore class names and file paths.
 *
 * This acts a miniature version of backdrop_autoload() that loads needed
 * backup handlers. Unlike the normal autoloader, this is not modifiable by
 * modules, since a full module bootstrap depends on the database. As an
 * emergency restore cannot depend on either the database nor config to be
 * available, settings.php is the only mechanism to configuring backup/restore
 * handlers. Additional backup handlers can be added by adding the following
 * lines to settings.php:
 *
 * @code
 * $settings['backup_handlers'] = array(
 *   'MyBackupHandler' = BACKDROP_ROOT . '/modules/my_module/includes/my_module.backup.inc';
 * );
 * @endcode
 *
 * @return array
 *   An array of paths to classes, keyed by the class name.
 */
function backup_get_handlers() {
  $backup_directory = 'core/includes/backup/';
  $class_map = array(
    'Backup' => $backup_directory . 'backup.class.inc',
    'BackupConfig' => $backup_directory . 'backup.config.inc',
    'BackupDatabase' => $backup_directory . 'backup.database.inc',
    'BackupFile' => $backup_directory . 'backup.file.inc',
    'BackupMySql' => $backup_directory . 'backup.mysql.inc',
  );
  return array_merge($class_map, settings_get('backup_handlers', array()));
}

/**
 * Get a list of all available backups.
 */
function backup_directory_list() {
  $backups = array();
  $backup_directory = settings_get('backup_directory');
  if (empty($backup_directory)) {
    return $backups;
  }

  $current_directory = '';
  $skip_this_directory = FALSE;
  $backup_file_list = file_scan_directory($backup_directory, '/\.backup\.json$/', array(
    'max_depth' => 2,
  ));
  foreach ($backup_file_list as $backup_file_path => $backup_file) {
    $backup_file_directory = dirname($backup_file->uri);
    $backup_file_path = $backup_file->uri;
    if ($backup_file_directory != $current_directory) {
      $current_directory = $backup_file_directory;
      $skip_this_directory = FALSE;
    }

    // If a single backup or JSON file in this directory is invalid, skip the
    // entire backup.
    if ($skip_this_directory) {
      continue;
    }

    // Check if the JSON file is valid, if not, continue to the next file.
    $backup_info = @backdrop_json_decode(file_get_contents($backup_file_path));
    if (!$backup_info || !isset($backup_info['backups'])) {
      $backups[$backup_file_directory] = array(
        'label' => basename($backup_file_directory),
        'valid' => FALSE,
      );
      $skip_this_directory = TRUE;
      continue;
    }

    // If valid, add to list of backups.
    if (!isset($backups[$backup_file_directory])) {
      $backups[$backup_file_directory] = array(
        'valid' => TRUE,
      ) + $backup_info;
    }
  }

  return $backups;
}

/**
 * Starts a batch process to create a backup.
 *
 * @param $backups
 *   An array of backup settings, keyed by the backup type and key. For example
 *   a common set of keys would be:
 *   - mysql:default
 *   - config:active
 *   The value of each array is another array of backup settings, as would be
 *   passed to backup_execute().
 * @param $redirect
 *   Path to redirect to when the batch has finished processing.
 * @param $url
 *   URL of the batch processing page (should only be used for separate
 *   scripts like update.php).
 * @param $batch
 *   Optional parameters to pass into the batch API.
 * @param $redirect_callback
 *   (optional) Specify a function to be called to redirect to the progressive
 *   processing page.
 *
 * @see update_resolve_dependencies()
 */
function backup_batch($backups, $redirect = NULL, $url = NULL, $batch = array(), $redirect_callback = 'backdrop_goto') {
  $operations = array();

  foreach ($backups as $backup) {
    $settings = isset($backup['settings']) ? $backup['settings'] : array();
    $operations[] = array('backup_execute', array($backup['class'], $backup['name'], $backup['target'], $settings));
  }
  $batch['operations'] = $operations;
  $batch += array(
    'title' => t('Creating backup'),
    'init_message' => t('Starting backup'),
    'error_message' => t('A backup could not be created.'),
    'finished' => 'backup_batch_finished',
    'file' => 'core/includes/backup.inc',
  );
  batch_set($batch);
  batch_process($redirect, $url, $redirect_callback);
}

/**
 * Finishes the backup process and stores the results for eventual display.
 *
 * @param $success
 *   Indicate that the batch API tasks were all completed successfully.
 * @param $results
 *   An array of all the results that were updated in update_do_one().
 * @param $operations
 *   A list of all the operations that had not been completed by the batch API.
 *
 * @see backup_batch()
 */
function backup_batch_finished($success, $results, $operations) {
  $_SESSION['backup_results'] = $results;
  $_SESSION['backup_success'] = $success;
}

/**
 * Prepare for a batch backup process, ensuring requirements are met.
 *
 * @param array $backups
 *   An array of backup configurations, keyed by backup ID. This value is
 *   modified by reference.
 * @param array $errors
 *    Any errors that were encountered while preparing for the backup. This
 *    value is modified by reference.
 *
 * @return boolean
 *   TRUE if the batch is ready to run, FALSE if errors were encountered.
 */
function backup_batch_prepare(&$backups, &$errors) {
  foreach ($backups as $backup_name => $backup) {
    if (!isset($backups[$backup_name]['settings'])) {
      $backups[$backup_name]['settings'] = array();
    }
    if (!$backup['enabled']) {
      unset($backups[$backup_name]);
    }
    elseif (!backup_prepare($backup_name, $backup['target'], $backups[$backup_name]['settings'], $errors)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Prepare for a single backup process, ensuring requirements are met.
 *
 * @param string $backup_name
 *  The overall name of the backup, used as the base name of the created backup
 *  file.
 * @param string $backup_target
 *  The target to be backed up (the source). This is usually an indicator
 *  such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of backup settings. This value is modified by reference.
 * @param array $errors
 *   Any errors that were encountered while preparing for the backup. This
 *   value is modified by reference.
 *
 * @return boolean
 *   TRUE if the backup is ready to run, FALSE if errors were encountered.
 */
function backup_prepare($backup_name, $backup_target, &$backup_settings, &$errors) {
  // Populate the destination directory if not specified.
  $default_directory = settings_get('backup_directory', '');
  $timestamp = isset($backup_settings['timestamp']) ? $backup_settings['timestamp'] : REQUEST_TIME;
  $date = date('Y-m-d\TH-i-s', $timestamp);

  // Ensure required properties are set and at the top of the settings.
  $backup = array(
    'name' => $backup_name,
    'target' => $backup_target,
    'settings' => $backup_settings,
  );

  // Set the backup directory if needed.
  if (!isset($backup_settings['backup_directory'])) {
    // This should never occur as the Backup step is disabled if the
    // $settings['backup_directory'] variable is empty.
    if (empty($default_directory)) {
      $errors[] = t('No backup can be created because the "$settings[\'backup_directory\']" variable is not set in settings.php.');
      return FALSE;
    }
    $backup_settings['backup_directory'] = $default_directory . '/' . $date;
  }
  $backup_directory = $backup_settings['backup_directory'];
  $directory_name = basename($backup_directory);

  // Ensure the destination backup directory exists and is writable.
  $directory_writeable = file_prepare_directory($backup_directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
  if (!$directory_writeable) {
    $errors[] = t('The destination backup directory %directory is not writable.', array(
      '%directory' => $backup_directory,
    ));
    return FALSE;
  }

  // Multiple backups may be placed in the same directory, such as multiple
  // databases or the database + config. Check if this is adding to an existing
  // backup directory.
  $backup_info_file = $backup_directory . '/' . $directory_name . '.backup.json';
  if (file_exists($backup_info_file)) {
    $backup_info = backdrop_json_decode(file_get_contents($backup_info_file));
  }
  else {
    $backup_info = array(
      'timestamp' => $timestamp,
      'label' => isset($backup_settings['label']) ? $backup_settings['label'] : $directory_name,
      'description' => isset($backup_settings['description']) ? $backup_settings['description'] : '',
      'backup_directory' => $backup_directory,
      'backups' => array(),
    );
  }

  // Add information about this specific backup to the directory, excluding the
  // values that are shared.
  $backup['settings'] = array_diff_key($backup['settings'], array(
    'backup_directory' => NULL,
    'label' => NULL,
    'description' => NULL,
    'timestamp' => NULL,
  ));
  $backup_info['backups'][$backup_name] = $backup;

  // Save the backup settings as a JSON file for restore.php to read later.
  // This also guarantees that the destination is writable by attempting to
  // put a file in it before the real backup.
  $json_success = file_put_contents($backup_info_file, backdrop_json_encode($backup_info, TRUE));
  if (!$json_success) {
    $errors[] = t('The backup settings file could not be saved into the %directory backup directory.', array(
      '%directory' => $backup_directory,
    ));
    return FALSE;
  }

  return TRUE;
}

/**
 * Run a backup process.
 *
 * @param string $backup_class
 *   The name of the Backup class to save the backup, such as BackupMySql or
 *   BackupConfig.
 * @param string $backup_name
 *   The overall name of the backup, used as the base name of the created backup
 *   file.
 * @param string $backup_target
 *   The target to be backed up (the source). This is usually an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of settings that are read by the Backup class, modifying its
 *   behavior. Each Backup class may have different settings. Check the
 *   defaultSettings() method within each class for a full list.
 *
 * @return false|mixed|void
 */
function backup_execute($backup_class, $backup_name, $backup_target, array $backup_settings) {
  // Safety check that the passed-in class uses the Backup base class.
  if (!is_subclass_of($backup_class, 'Backup', TRUE)) {
    return FALSE;
  }

  backup_prepare_environment();

  // If no filepath is set, a backup cannot be created.
  if (empty($backup_settings['backup_directory'])) {
    return FALSE;
  }

  // Create the BackupFile instance to hold the backup output.
  $file = new BackupFile($backup_name, $backup_settings['backup_directory']);

  // Register shutdown callback to deal with timeouts.
//  register_shutdown_function('backup_migrate_shutdown', $settings);

  /** @var Backup $backup_instance */
  $backup_instance = new $backup_class($backup_name, $backup_target, $backup_settings);

  // Pre and post steps are separate to allow offloading compression and
  // decompression into separate requests (eventually).
  $backup_instance->preBackup($file);
  $success = $backup_instance->backup($file);
  $backup_instance->postBackup($file);

  if (!$success) {
    return FALSE;
  }

  return $file;
}

/**
 * Prepare the PHP environment for a backup/restore.
 *
 * @return void
 */
function backup_prepare_environment() {
  // If not in 'safe mode', increase the maximum execution time.
  //  if (!ini_get('safe_mode') && strpos(ini_get('disable_functions'), 'set_time_limit') === FALSE && ini_get('max_execution_time') < 1200) {
  //    _backup_migrate_set_timeout();
  //  }
  //
  //  // Conditionally increase the memory limit.
  //  // Get the current php memory limit.
  //  $php_memory_limit = ini_get('memory_limit');
  //
  //  // If the memory limit is set to -1, there's no need to do anything.
  //  if ($php_memory_limit !== -1) {
  //    // Convert the php memory limit to bytes.
  //    $php_memory_limit_bytes = _backup_migrate_to_bytes($php_memory_limit);
  //
  //    // Get the current Backup Migrate memory limit.
  //    $backup_migrate_memory_limit = config_get('backup_migrate.settings', 'backup_migrate_memory_limit');
  //
  //    // Convert the Backup Migrate memory limit to bytes.
  //    $backup_migrate_memory_limit_bytes = _backup_migrate_to_bytes($backup_migrate_memory_limit);
  //
  //    // Increase the memory limit.
  //    if ($php_memory_limit_bytes < $backup_migrate_memory_limit_bytes) {
  //      // Ensure ini_set() is not disabled.
  //      if (!strpos(ini_get('disable_functions'), 'ini_set')) {
  //        // Convert bytes to megabytes, add the 'M' unit.
  //        $backup_migrate_memory_limit = round($backup_migrate_memory_limit_bytes / 1024 / 1024) . 'M';
  //
  //        // Update the php memory limit.
  //        ini_set('memory_limit', $backup_migrate_memory_limit);
  //      }
  //    }
  //  }
  //
  //  if (config_get('backup_migrate.settings', 'backup_migrate_verbose')) {
  //    _backup_migrate_message("Memory limit: %limit", array('%limit' => ini_get('memory_limit')), 'success');
  //  }
  //
}

/**
 * Prepare for a single restore process, ensuring requirements are met.
 *
 * @param string $backup_file
 *   The path to the backup file to be restored.
 * @param string $restore_target
 *   The target to which the backup should be restored. Typically an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of backup settings. This value is modified by reference.
 * @param array $errors
 *   Any errors that were encountered while preparing for the backup. This
 *   value is modified by reference.
 *
 * @return boolean
 *   TRUE if the restore is ready to run, FALSE if errors were encountered.
 */
function backup_restore_prepare($backup_file, $restore_target, &$backup_settings, &$errors) {
  // @todo: check if anything is needed here.
  return TRUE;
}

/**
 * Run a backup process.
 *
 * @param string $backup_class
 *   The name of the Backup class to save the backup, such as BackupMySql or
 *   BackupConfig.
 * @param string $backup_name
 *   The overall name of the backup, used as the base name of the created backup
 *   file.
 * @param string $backup_directory
 *   The directory containing the backup file to restore.
 * @param string $restore_target
 *   The target to which the backup should be restored. Typically an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of settings that are read by the Backup class, modifying its
 *   behavior. Each Backup class may have different settings. Check the
 *   defaultSettings() method within each class for a full list.
 *
 * @return bool
 *   TRUE if the restore was successful, FALSE on failure.
 */
function backup_restore_execute($backup_class, $backup_name, $backup_directory, $restore_target, array $backup_settings) {
  // Safety check that the passed-in class uses the Backup base class.
  if (!is_subclass_of($backup_class, 'Backup', TRUE)) {
    return FALSE;
  }

  backup_prepare_environment();

  // Create the BackupFile instance to hold the backup output.
  $file = new BackupFile($backup_name, $backup_directory);
  // @todo: Determine these extensions in a more flexible way.
  if (strpos($restore_target, 'db:') === 0) {
    $file->pushExtension('sql');
    $file->pushExtension('gz');
  }
  elseif (strpos($restore_target, 'config:') === 0) {
    $file->pushExtension('tar');
    $file->pushExtension('gz');
  }

  // Register shutdown callback to deal with timeouts.
  //  register_shutdown_function('backup_restore_shutdown', $settings);

  /** @var Backup $backup_instance */
  $backup_instance = new $backup_class($backup_name, $restore_target, $backup_settings);

  // Pre and post steps are separate to allow offloading compression and
  // decompression into separate requests (eventually).
  $backup_instance->preRestore($file);
  $success = $backup_instance->restore($file);
  $backup_instance->postRestore($file);

  return $success;
}
