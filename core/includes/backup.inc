<?php
/**
 * @file
 * Contains functionality related to creating and restoring site backups.
 */

/**
 * Get all backup/restore class names and file paths.
 *
 * This acts a miniature version of backdrop_class_list() that lists all
 * available classes that act as backup handlers. Unlike the normal autoloader,
 * this is not modifiable by modules, since a full module bootstrap depends on
 * the database. As an emergency restore cannot depend on either the database
 * nor config to be available, settings.php is the only mechanism to configuring
 * backup/restore handlers. Additional backup handlers can be added by adding
 * the following lines to settings.php:
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
function backup_class_list() {
  $backup_class_directory = 'core/includes/backup/';
  $class_map = array(
    'Backup' => $backup_class_directory . 'backup.class.inc',
    'BackupConfig' => $backup_class_directory . 'backup.config.inc',
    'BackupDatabase' => $backup_class_directory . 'backup.database.inc',
    'BackupFile' => $backup_class_directory . 'backup.file.inc',
    'BackupMySql' => $backup_class_directory . 'backup.mysql.inc',
  );
  return array_merge($class_map, settings_get('backup_handlers', array()));
}

/**
 * Given a particular backup/restore target, find the class that applies.
 *
 * @param string $target
 *   The backup source or restore target as a string, such as "db:default" or
 *   "config:active".
 *
 * @return string|false
 *   The name of the class that indicates it can handle the given backup target.
 *   Additional classes can be added through $settings['backup_handlers'] in
 *   settings.php. If multiple classes match the target, the last class in the
 *   list takes priority.
 */
function backup_get_handler_name($target) {
  $class_list = array_reverse(backup_class_list(), TRUE);
  foreach ($class_list as $class_name => $class_path) {
    if (is_subclass_of($class_name, 'Backup')) {
      if ($class_name::applies($target)) {
        return $class_name;
      }
    }
  }
  return FALSE;
}

/**
 * Get a list of all available backups.
 */
function backup_directory_list() {
  $backups = array();
  $backup_directory = backup_get_backup_directory();
  if (empty($backup_directory)) {
    return $backups;
  }

  $current_directory = '';
  $skip_this_directory = FALSE;
  $backup_file_list = file_scan_directory($backup_directory, '/\.backup\.json$/', array(
    'max_depth' => 2,
  ));
  foreach ($backup_file_list as $backup_file) {
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
    $backup_name = basename($backup_file_directory);
    if (!$backup_info || !isset($backup_info['targets'])) {
      $backups[$backup_name] = array(
        'label' => $backup_name,
        'valid' => FALSE,
      );
      $skip_this_directory = TRUE;
      continue;
    }

    // If valid, add to list of backups.
    if (!isset($backups[$backup_name])) {
      $backups[$backup_name] = array(
        'valid' => TRUE,
      ) + $backup_info;
      // Overwrite the backup_directory value to be the current location.
      $backups[$backup_name]['backup_directory'] = $backup_file_directory;
    }
  }

  return $backups;
}

/**
 * A general-purpose form builder for creating a backup.
 *
 * This form builder is used by update.php to generate the backup form step
 * of running system updates. It can also be used by contributed modules to
 * quickly create a backup.
 *
 * See the backup_test.module for a demonstration of how this can be used.
 *
 * @param array $form
 *   A form API array to which the backup settings should be added.
 * @param array $form_state
 *   The form state array.
 * @param array $options
 *   An array of options containing the following keys:
 *   - show_settings: Whether each backup handler class should show the
 *     advanced settings for that type of backup. Defaults to FALSE.
 *   - show_default_targets: Whether the targets of the default database and the
 *     default configuration location should be shown as checkboxes, allowing
 *     them to not be backed up. If not shown, they are rendered as type =
 *     "hidden" elements. Defaults to FALSE.
 *
 * @see update_backup_form()
 * @see backup_test_settings_form()
 */
function backup_settings_form(array $form, array &$form_state, array $options = array()) {
  $options += array(
    'show_settings' => FALSE,
    'show_default_targets' => FALSE,
  );

  $backup_directory = backup_get_backup_directory();
  $backup_targets = settings_get('backup_targets', array());
  $backup_targets = array_merge($backup_targets, array(
    'db:default',
    'config:active',
  ));

  // Two expected elements:
  $form['options']['#tree'] = TRUE;
  $form['targets']['#tree'] = TRUE;

  foreach ($backup_targets as $backup_target) {
    list($backup_plugin_id, $backup_subkey) = explode(':', $backup_target, 2);
    $backup_class = backup_get_handler_name($backup_target);
    if (!$backup_class) {
      continue;
    }

    $backup_target_name = $backup_plugin_id . '_' . $backup_subkey;
    $backup_settings = array();
    $backup_instance = new $backup_class($backup_target_name, $backup_target, $backup_settings);
    $backup_form = $backup_instance->backupSettingsForm();

    if ($options['show_settings'] && $backup_form) {
      $form['targets'][$backup_target_name] = array(
        '#type' => 'fieldset',
        '#title' => t('@type backup settings', array(
          '@type' => $backup_instance->typeLabel(),
        )),
        '#collapsible' => FALSE,
      );

      $form['targets'][$backup_target_name]['settings'] = $backup_form;
      $form['targets'][$backup_target_name]['settings']['#type'] = 'container';
    }

    // Show a checkbox if non-default targets are present.
    if ($options['show_default_targets'] || ($backup_target != 'db:default' && $backup_target != 'config:active')) {
      $form['targets'][$backup_target_name]['enabled'] = array(
        '#type' => 'checkbox',
        '#title' => t('Optional backup: @target_name', array('@target_name' => $backup_target_name)),
        '#default_value' => TRUE,
        '#weight' => -1,
      );
    }
    else {
      $form['targets'][$backup_target_name]['enabled'] = array(
        '#type' => 'hidden',
        '#value' => TRUE,
      );
    }

    $form['targets'][$backup_target_name]['name'] = array(
      '#type' => 'hidden',
      '#value' => $backup_target_name,
    );
    $form['targets'][$backup_target_name]['target'] = array(
      '#type' => 'hidden',
      '#value' => $backup_target,
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create backup'),
    '#disabled' => empty($backup_directory),
  );

  return $form;
}

/**
 * Helper submit function for creating a backup.
 *
 * This is intended to help execute a backup as configured by
 * backup_settings_form(). To add this submit handler to a custom form, it is
 * recommended to add this to the form #submit property, such as:
 *
 * @code
 * $form['#submit'] = array(
 *   'backup_settings_form_submit',
 *   'my_module_settings_form_submit,
 * );
 * @endcode
 *
 * @param array $form
 *   The form array, as created by backdrop_settings_form().
 * @param array $form_state
 *   The form state, containing $form_state['values']['targets'] and optionally
 *   $form_state['values']['options'].
 *
 * @return void
 *
 * @see backup_batch()
 */
function backup_settings_form_submit(array $form, array &$form_state) {
  $backup_targets = $form_state['values']['targets'];
  $options = $form_state['values']['options'] ?: array();

  $errors = array();
  $ready = backup_batch_prepare($backup_targets, $options, $errors);
  if ($ready) {
    backup_batch($backup_targets, $options);
  }
  else {
    foreach ($errors as $error) {
      backdrop_set_message($error, 'error');
    }
  }
}

/**
 * Starts a batch process to create a backup.
 *
 * @param array $backup_targets
 *   An array of backup settings, keyed by the backup name. For example a common
 *   set of keys would be:
 *   - db_default
 *   - config_active
 *   The value of each array is another array of backup settings, as would be
 *   passed to backup_execute().
 * @param array $options
 *   Additional options to control the Backup process. Available keys:
 *   - backup_limit: Integer indicating how many total backups should be
 *     allowed. Backups exceeding this count will be removed, oldest first.
 *     A value of FALSE will not enforce a limit.
 *   - name: The name of the overall backup directory. If not provided,
 *     a timestamp-based directory name will be used.
 *   - label: A human-readable label for this backup. If not provided, the name
 *     will be used.
 *   - description: A description of the backup.
 * @param string $redirect
 *   Path to redirect to when the batch has finished processing.
 * @param string $url
 *   URL of the batch processing page (should only be used for separate
 *   scripts like update.php).
 * @param array $batch
 *   Optional parameters to pass into the batch API.
 * @param string $redirect_callback
 *   (optional) Specify a function to be called to redirect to the progressive
 *   processing page.
 */
function backup_batch(array $backup_targets, array $options = array(), $redirect = NULL, $url = 'batch', array $batch = array(), $redirect_callback = 'backdrop_goto') {
  $operations = array();

  // Ensure a default backup limit exists. Excess backups are removed upon
  // completion.
  $backup_limit = config_get('system.backup', 'backup_limit');
  $options += array(
    'backup_limit' => isset($backup_limit) ? $backup_limit : 5,
    'name' => NULL,
    'label' => NULL,
    'description' => NULL,
  );

  foreach ($backup_targets as $backup_target) {
    $settings = isset($backup_target['settings']) ? $backup_target['settings'] : array();
    if ($options['name']) {
      $settings['backup_directory'] = backup_get_backup_directory() . '/' . $options['name'];
    }
    $operations[] = array('backup_execute', array(
      $backup_target['name'],
      $backup_target['target'],
      $settings,
    ));
  }

  if ($options['backup_limit']) {
    $operations[] = array('backup_limit_cleanup', array(
      $options['backup_limit'],
    ));
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
 * @param bool $success
 *   Indicate that the batch API tasks were all completed successfully.
 * @param array $results
 *   An array of all the results that were updated in update_do_one().
 * @param array $operations
 *   A list of all the operations that had not been completed by the batch API.
 *
 * @see backup_batch()
 */
function backup_batch_finished($success, array $results, array $operations) {
  if ($success) {
    backdrop_set_message(t('Backup created successfully.'));
  }
  else {
    backdrop_set_message(t('Backup failed.'), 'error');
  }
}

/**
 * Prepare for a batch backup process, ensuring requirements are met.
 *
 * @param array $backup_targets
 *   An array of backup settings, keyed by backup name. This value is modified
 *   by reference.
 * @param array $options
 *   Additional options to control the Backup process. See backup_batch() for
 *   the list of available keys.
 * @param array $errors
 *   Any errors that were encountered while preparing for the backup. This
 *   value is modified by reference.
 *
 * @return boolean
 *   TRUE if the batch is ready to run, FALSE if errors were encountered.
 */
function backup_batch_prepare(array &$backup_targets, array $options, array &$errors) {
  foreach ($backup_targets as $backup_target_name => $backup) {
    if (!isset($backup_targets[$backup_target_name]['settings'])) {
      $backup_targets[$backup_target_name]['settings'] = array();
    }
    $backup_targets[$backup_target_name]['settings'] += $options;
    if (!empty($options['name'])) {
      $clean_name = preg_replace('/[^a-z0-9_\-]+/', '_', $options['name']);
      // Ensure the backup name is valid before proceeding.
      if ($clean_name != $options['name']) {
        $errors[] = t('Invalid backup name: "@name", only letters, numbers, underscores and dashes are allowed.', array('@name' => $options['name']));
        return FALSE;
      }
      $backup_targets[$backup_target_name]['settings']['backup_directory'] = backup_get_backup_directory() . '/' . $options['name'];
    }
    if (empty($backup['enabled'])) {
      unset($backup_targets[$backup_target_name]);
    }
    elseif (!backup_prepare($backup_target_name, $backup['target'], $backup_targets[$backup_target_name]['settings'], $errors)) {
      return FALSE;
    }
  }

  if (empty($backup_targets)) {
    $errors[] = t('No backup targets selected.');
    return FALSE;
  }

  return TRUE;
}

/**
 * Prepare for a single backup process, ensuring requirements are met.
 *
 * @param string $backup_target_name
 *   The name of the backup target, used as the base name of the created backup
 *   file, such as "db_default" or "config_active".
 * @param string $backup_target
 *   The target to be backed up (the source). This is usually an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of backup settings. This value is modified by reference.
 * @param array $errors
 *   Any errors that were encountered while preparing for the backup. This
 *   value is modified by reference.
 *
 * @return boolean
 *   TRUE if the backup is ready to run, FALSE if errors were encountered.
 */
function backup_prepare($backup_target_name, $backup_target, array &$backup_settings, array &$errors) {
  // Populate the destination directory if not specified.
  $default_directory = backup_get_backup_directory();
  $timestamp = isset($backup_settings['timestamp']) ? $backup_settings['timestamp'] : REQUEST_TIME;
  $date = date('Y-m-d\TH-i-s', $timestamp);

  // Ensure required properties are set and at the top of the settings.
  $backup = array(
    'name' => $backup_target_name,
    'target' => $backup_target,
    // Set to TRUE when the backup is finished.
    'completed' => FALSE,
    'settings' => $backup_settings,
  );

  // Set the backup directory if needed.
  if (!isset($backup_settings['backup_directory'])) {
    // This should never occur as the Backup step is disabled if the
    // $settings['backup_directory'] variable is empty.
    if (empty($default_directory)) {
      $errors[] = t('No backup can be created because the !variable variable is not set in !file.', array(
        '!variable' => '<code>$settings[\'backup_directory\']</code>',
        '!file' => '<code>settings.php</code>',
      ));
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
      'name' => $directory_name,
      'label' => isset($backup_settings['label']) ? $backup_settings['label'] : $directory_name,
      'description' => isset($backup_settings['description']) ? $backup_settings['description'] : '',
      'backup_directory' => $backup_directory,
      'targets' => array(),
    );
  }

  // Add information about this specific backup to the directory, excluding the
  // values that are shared.
  $backup['settings'] = array_diff_key($backup['settings'], $backup_info);
  $backup_info['targets'][$backup_target_name] = $backup;

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
 * @param string $backup_target_name
 *   The name of the backup target, used as the base name of the created backup
 *   file, such as "db_default" or "config_active".
 * @param string $backup_target
 *   The target to be backed up (the source). This is usually an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param array $backup_settings
 *   An array of settings that are read by the Backup class, modifying its
 *   behavior. Each Backup class may have different settings. Check the
 *   defaultSettings() method within each class for a full list.
 *
 * @return BackupFile|false
 *   The BackupFile referencing the newly created backup.
 */
function backup_execute($backup_target_name, $backup_target, array $backup_settings) {
  // Safety check that backup target has an associate backup handler.
  $backup_class = backup_get_handler_name($backup_target);
  if (!is_subclass_of($backup_class, 'Backup')) {
    return FALSE;
  }

  // If no filepath is set, a backup cannot be created.
  if (empty($backup_settings['backup_directory'])) {
    return FALSE;
  }
  $backup_directory = $backup_settings['backup_directory'];

  // Create the BackupFile instance to hold the backup output.
  $file = new BackupFile($backup_target_name, $backup_directory);

  /** @var Backup $backup_instance */
  $backup_instance = new $backup_class($backup_target_name, $backup_target, $backup_settings);

  // Pre and post steps are separate to allow offloading compression and
  // decompression into separate requests (eventually).
  $backup_instance->preBackup($file);
  $success = $backup_instance->backup($file);
  $backup_instance->postBackup($file);

  // Set the "completed" flag indicating this backup was successful.
  if ($success) {
    $directory_name = basename($backup_directory);
    $backup_info_file = $backup_directory . '/' . $directory_name . '.backup.json';
    $backup_info = backdrop_json_decode(file_get_contents($backup_info_file));
    $backup_info['targets'][$backup_target_name]['completed'] = TRUE;
    file_put_contents($backup_info_file, backdrop_json_encode($backup_info, TRUE));
  }

  if (!$success) {
    return FALSE;
  }

  return $file;
}

/**
 * Delete excess backups that exceed a certain count. Oldest first.
 *
 * @param int $backup_limit
 *   The maximum number of backups to allow. Must be greater than 1. If deleting
 *   all backups entirely, use backup_delete() instead.
 *
 * @return int
 *   The number of backups removed.
 */
function backup_limit_cleanup($backup_limit) {
  $backups = backup_directory_list();

  if ($backup_limit <= 1) {
    return 0;
  }

  // Sort by reverse date order.
  backdrop_sort($backups, array('timestamp'), SORT_DESC);
  $backup_count = 0;
  $backups_deleted = 0;
  foreach ($backups as $backup_name => $backup) {
    if ($backup['valid']) {
      $backup_count++;
      if ($backup_count > $backup_limit) {
        if (backup_delete($backup_name)) {
          $backups_deleted++;
        }
      }
    }
  }
  return $backups_deleted;
}

/**
 * Delete a backup from the backup directory by its directory name.
 *
 * @param string $backup_name
 *   The name of the backup (same as the backup directory).
 *
 * @return bool
 *   TRUE if the backup was deleted, FALSE on failure.
 */
function backup_delete($backup_name) {
  // Validate that the backup exists within the backup directory. This prevents
  // deleting any kind of file with this function other than a backup.
  $backups = backup_directory_list();
  $success = FALSE;
  if (!isset($backups[$backup_name])) {
    return FALSE;
  }

  $backup = $backups[$backup_name];
  // Final safety check, only delete valid backups (those with correct JSON
  // files describing their contents). The "backup_directory" key is set
  // specifically in backup_directory_list(), it is not read from the JSON.
  if ($backup['valid'] && is_dir($backup['backup_directory'])) {
    $success = file_unmanaged_delete_recursive($backup['backup_directory']);
  }

  return $success;
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
function backup_restore_prepare($backup_file, $restore_target, array &$backup_settings, array &$errors) {
  // No operation here for now, but available and called if needed.
  return TRUE;
}

/**
 * Run a backup process.
 *
 * @param string $restore_target_name
 *   The name of the backup target, used as the base name of the created backup
 *   file, such as "db_default" or "config_active".
 * @param string $restore_target
 *   The target to which the backup should be restored. Typically an indicator
 *   such as "db:default" (for the database) or "config:active" (for config).
 * @param string $backup_directory
 *   The directory containing the backup file to restore.
 * @param array $backup_settings
 *   An array of settings that are read by the Backup class, modifying its
 *   behavior. Each Backup class may have different settings. Check the
 *   defaultSettings() method within each class for a full list.
 *
 * @return bool
 *   TRUE if the restore was successful, FALSE on failure.
 */
function backup_restore_execute($restore_target_name, $restore_target, $backup_directory, array $backup_settings) {
  // Safety check that restore target has an associate backup handler.
  $backup_class = backup_get_handler_name($restore_target);
  if (!is_subclass_of($backup_class, 'Backup', TRUE)) {
    return FALSE;
  }

  /** @var Backup $backup_instance */
  $backup_instance = new $backup_class($restore_target_name, $restore_target, $backup_settings);

  // Create the BackupFile instance to hold the backup output.
  $file = new BackupFile($restore_target_name, $backup_directory);
  // @todo: Determine these extensions in a more flexible way.
  if (strpos($restore_target, 'db:') === 0) {
    $file->pushExtension('sql');
    $file->pushExtension('gz');
  }
  elseif (strpos($restore_target, 'config:') === 0) {
    $file->pushExtension('tar');
    $file->pushExtension('gz');
  }

  // Pre and post steps are separate to allow offloading compression and
  // decompression into separate requests (eventually).
  $backup_instance->preRestore($file);
  $success = $backup_instance->restore($file);
  $backup_instance->postRestore($file);

  return $success;
}
