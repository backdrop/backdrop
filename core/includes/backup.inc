<?php

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
    'title' => 'Creating backup',
    'init_message' => 'Starting backup',
    'error_message' => 'A backup could not be created.',
    'finished' => 'backup_batch_finished',
    'file' => 'core/includes/backup.inc',
  );
  batch_set($batch);
  batch_process($redirect, $url, $redirect_callback);
}

/**
 * Finishes the backup process and stores the results for eventual display.
 *
 * After the updates run, all caches are flushed. The update results are
 * stored into the session (for example, to be displayed on the update results
 * page in update.php). Additionally, if the site was off-line, now that the
 * update process is completed, the site is set back online.
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
 * @param array $backup_settings
 *   An array of backup settings. This value is modified by reference.
 * @param array $errors
 *   Any errors that were encountered while preparing for the backup. This
 *   value is modified by reference.
 *
 * @return boolean
 *   TRUE if the batch is ready to run, FALSE if errors were encountered.
 */
function backup_prepare($backup_name, $backup_target, &$backup_settings, &$errors) {
  // Populate the destination directory if not specified.
  $default_directory = settings_get('backup_directory', '');
  // ISO 8601 and file-system friendly date (no colons).
  $timestamp = isset($backup_settings['timestamp']) ? $backup_settings['timestamp'] : REQUEST_TIME;
  $date = date('Y-m-d\TH-i-s', $timestamp);

  // Ensure required properties are set and at the top of the settings.
  $backup_settings = array(
    'name' => $backup_name,
    'target' => $backup_target,
    'timestamp' => $timestamp,
  ) + $backup_settings;

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

  // Ensure the destination backup directory exists and is writable.
  $directory_writeable = file_prepare_directory($backup_directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
  if (!$directory_writeable) {
    $errors[] = t('The destination backup directory %directory is not writable.', array(
      '%directory' => $backup_directory,
    ));
    return FALSE;
  }

  // Save the backup settings as a JSON file for restore.php to read later.
  // This also guarantees that the the destination is writable by attempting to
  // put a file in it before the real backup.
  $json_success = file_put_contents($backup_directory . '/' . $backup_name . '.json', backdrop_json_encode($backup_settings, TRUE));
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

  timer_start('backdrop_backup');

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
//  // Confirm the destinations are valid
//  $valid = backup_migrate_destination_confirm_destination($settings);
//  if (!$valid) {
//    backup_migrate_backup_fail("Could not run backup because a there was no valid destination to save to.", array(), $settings);
//    return FALSE;
//  }

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
