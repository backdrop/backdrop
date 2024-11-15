<?php

/**
 * @file
 * Functions to handle the direct to database destination.
 */

/**
 * A destination type for saving to a database server.
 *
 * @ingroup backup_restore
 */
abstract class BackupDatabase extends Backup {
  const BACKUP_ID = 'db';

  /**
   * @var DatabaseConnection
   *
   * The database connection to be used when doing a backup or restore.
   */
  protected $connection = NULL;

  /**
   * @var array
   *
   * If a restore operation is happening as part of a batch, a copy of the batch
   * information from the "batch" database table. This is saved in preRestore()
   * and re-inserted in postRestore().
   */
  protected $restoreBatch = NULL;
  protected $restoreQueue = NULL;

  /**
   * {@inheritdoc}
   */
  function typeLabel() {
    return t('Database');
  }

  /**
   * Get the file type supported when backing-up or restoring to/from a file.
   */
  public function getFileTypeId() {
    return 'sql';
  }

  /**
   * Get the version info for the given database.
   */
  public function databaseInfo() {
    return array(
      'type' => FALSE,
      'version' => t('Unknown'),
    );
  }

  /**
   * Get the default settings for this object.
   *
   * @return array
   *   The default tables whose data can be ignored. These tables mostly
   *   contain info which can be easily reproduced (such as cache or search
   *   index) but also tables which can become quite bloated but are not
   *   necessarily extremely important to back up or migrate during development
   *   (such as access log and watchdog).
   */
  public function defaultSettings() {
    $settings = parent::defaultSettings();

    $all_tables = $this->getTableNames();

    // Basic modules that should be excluded.
    $basic = array(
      // Default core tables.
      'session',
      'accesslog',
      // Devel module.
      'devel_queries',
      'devel_times',
    );

    // Identify all cache tables.
    $cache = array('cache');
    foreach ($all_tables as $table_name) {
      if (strpos($table_name, 'cache_') === 0) {
        $cache[] = $table_name;
      }
    }

    // Simpletest can create a lot of tables that do not need to be backed up,
    // but all of them start with the string 'simpletest' so they can be easily
    // excluded.
    $simpletest = array();
    foreach ($all_tables as $table_name) {
      if (strpos($table_name, 'simpletest') === 0) {
        $simpletest[] = $table_name;
      }
    }

    $settings += array(
      'nodata_tables' => backdrop_map_assoc(array_merge($basic, $cache)),
      'exclude_tables' => $simpletest,
      'lock_tables' => TRUE,
      'compression' => self::COMPRESSION_GZIP,
    );

    return $settings;
  }

  /**
   * Get the form for the backup settings for this destination.
   */
  public function backupSettingsForm() {
    $objects = $this->getObjectNames();
    $settings = $this->settings;

    $form['#description'] = t('You may omit specific tables, or specific table data from the backup file. Only omit data that you know you will not need such as cache data, or tables from other applications. Excluding tables can break your Backdrop install, so <strong>do not change these settings unless you know what you\'re doing</strong>.');
    $form['exclude_tables'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Exclude the following tables altogether'),
      '#options' => $objects,
      '#default_value' => $settings['exclude_tables'],
      '#description' => t('The selected tables will not be added to the backup file.'),
    );
    $form['nodata_tables'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Exclude the data from the following tables'),
      '#options' => $objects,
      '#default_value' => $settings['nodata_tables'],
      '#description' => t('The selected tables will have their structure backed up but not their contents. This is useful for excluding cache data to reduce file size.'),
    );
    $form['lock_tables'] = array(
      '#type' => 'checkbox',
      '#title' => t('Lock tables during backup'),
      '#default_value' => !empty($settings['lock_tables']) ? $settings['lock_tables'] : NULL,
      '#description' => t('This can help reduce data corruption, but will make your site unresponsive.'),
    );
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function backup(BackupFile $file) {
    $file->pushExtension($this->getFileTypeId());

    //$this->lock_tables($settings);

    // Switch to a different db if specified.
    $success = $this->backupDatabaseToFile($file);

    //$this->unlock_tables($settings);

    // Pull any logged messages out of the file and up to this class.
    $file_log = $file->getLog();
    foreach ($file_log as $log) {
      call_user_func_array($this->log, $log);
    }
    $file->clearLog();

    return $success ? $file : FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function postBackup(BackupFile &$file) {
    parent::postBackup($file);
    // Compress the backup and delete the original once compressed.
    if ($this->settings['compression'] === self::COMPRESSION_GZIP) {
      // The returned value should overwrite $file by reference.
      $file = $this->compress($file, self::COMPRESSION_GZIP, TRUE);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function preRestore(BackupFile &$file) {
    parent::preRestore($file);
    // Decompress the backup but leave the original file.
    if ($file->lastExtension() === 'gz') {
      $this->decompress($file, self::COMPRESSION_GZIP, FALSE);
      $file->popExtension();
    }

    $batch_id = isset($_REQUEST['id']) ? (int) $_REQUEST['id'] : 0;
    if ($batch_id && $batch = batch_load($batch_id)) {
      $this->restoreBatch = $batch;
      $queue_item = db_select('queue', 'q')
        ->fields('q')
        ->condition('name', 'backdrop_batch:' . $batch_id . ':0')
        ->execute()
        ->fetchAssoc();
      $this->restoreQueue = $queue_item;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function restore(BackupFile $file) {
    $num = 0;
    // Open the file using the file wrapper.
    if ($file->extension() !== $this->getFileTypeId()) {
      $this->log('Unable to restore from file %file because a %type file can\'t be restored to this database.',
        array(
          '%file' => $file->filepath(),
          '%type' => $file->extension(),
        ),
        Backup::LOG_ERROR
      );
    }
    else {
      // Restore the database.
      $num = $this->restoreDatabaseFromFile($file);

      if ($this->settings['verbose']) {
        $this->log('%num SQL commands executed.', array('%num' => $num), self::LOG_INFO);
      }
    }

    // Pull any logged messages out of the file and up to this class.
    $file_log = $file->getLog();
    foreach ($file_log as $log) {
      call_user_func_array($this->log, $log);
    }
    $file->clearLog();

    return $num;
  }

  /**
   * {@inheritdoc}
   */
  public function postRestore(BackupFile &$file) {
    parent::postRestore($file);

    // If a compressed copy of this backup exists, delete the uncompressed copy.
    if ($file->lastExtension() === 'sql') {
      $uncompressed_path = $file->filePath();
      $file->pushExtension('gz');
      if (file_exists($file->filepath())) {
        file_unmanaged_delete($uncompressed_path);
      }
      else {
        $file->popExtension();
      }
    }

    // If a batch is in progress, put back the batch entry into the database.
    if ($this->restoreBatch) {
      $batch = $this->restoreBatch;
      db_insert('batch')
        ->fields(array(
          'bid' => $batch['id'],
          'timestamp' => REQUEST_TIME,
          'token' => backdrop_get_token($batch['id']),
          'batch' => serialize($batch),
        ))
        ->execute();

      // Save the current session into the restored database.
      $_SESSION['batches'][$batch['id']] = TRUE;
      backdrop_static_reset('backdrop_session_last_read');
      backdrop_session_commit();

      if ($this->restoreQueue) {
        db_merge('queue')
          ->key(array('item_id' => $this->restoreQueue['item_id']))
          ->fields($this->restoreQueue)
          ->execute();
      }
    }
  }

  /**
   * Get the db connection for the specified db.
   */
  protected function getDatabaseConnection() {
    if (!$this->connection) {
      $parts = explode(':', $this->getTarget());
      // One of the predefined databases (set in settings.php)
      if ($parts[0] == 'db') {
        $key = empty($parts[1]) ? 'default' : $parts[1];
        $target = empty($parts[2]) ? 'default' : $parts[2];
      }
      // An arbitrary database URL such as mysql://user:pass@localhost:3306/db.
      // This ability is not used directly by core, which only provides UIs for
      // backing up and restoring existing database connections.
      else {
        $url_parts = parse_url($this->getTarget());
        // If the url is specified build it into a connection info array.
        if ($url_parts) {
          $info = array(
            'driver' => empty($url_parts['scheme']) ? NULL : $url_parts['scheme'],
            'host' => empty($url_parts['host']) ? NULL : $url_parts['host'],
            'port' => empty($url_parts['port']) ? 3306 : $url_parts['port'],
            'username' => empty($url_parts['user']) ? NULL : $url_parts['user'],
            'password' => empty($url_parts['pass']) ? NULL : $url_parts['pass'],
            'database' => empty($url_parts['path']) ? NULL : $url_parts['path'],
          );
          $key = uniqid('backdrop_backup_tmp_');
          $target = 'default';
          Database::addConnectionInfo($key, $target, $info);
        }
        // No database selected. Assume the default.
        else {
          $key = $target = 'default';
        }
      }
      if ($target && $key) {
        $this->connection = Database::getConnection($target, $key);
      }
    }
    return $this->connection;
  }

  /**
   * Backup the databases to a file.
   *
   * @return bool
   *   TRUE is backup successful, FALSE otherwise.
   */
  abstract protected function backupDatabaseToFile(BackupFile $file);

  /**
   * Backup the databases to a file.
   *
   * @return bool
   *   TRUE is restore successful, FALSE otherwise.
   */
  abstract protected function restoreDatabaseFromFile(BackupFile $file);

  /**
   * Get a list of objects in the database.
   */
  protected function getObjectNames() {
    // Must be overridden.
    $out = $this->getTableNames();
    if (method_exists($this, 'getViewNames')) {
      $out += $this->getViewNames();
    }
    return $out;
  }

  /**
   * Get a list of tables in the database.
   *
   * @return string[]
   *   An array of all table names keyed by the table name.
   */
  abstract protected function getTableNames();

  /**
   * Get a list of views in the database.
   *
   * @return string[]
   *   An array of all view names keyed by the view name.
   */
  abstract protected function getViewNames();

  /**
   * Get a list of tables to be locked during the backup.
   *
   * @return array
   *   The list of tables to be locked, if any.
   */
  protected function getLockedTables() {
    $tables = array();
    if ($this->settings['lock_tables']) {
      foreach ($this->getTableNames() as $table) {
        // There's no need to lock excluded or structure only tables because it
        // doesn't matter if they change.
        if (empty($settings['exclude_tables']) || !in_array($table, (array) $settings['exclude_tables'])) {
          $tables[] = $table;
        }
      }
    }
    return $tables;
  }

  /**
   * Lock the database in anticipation of a backup.
   */
  abstract protected function lockTables();

  /**
   * Unlock any tables that have been locked.
   */
  abstract protected function unlockTables();

}
