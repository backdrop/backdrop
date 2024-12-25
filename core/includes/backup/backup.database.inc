<?php
/**
 * @file
 * Functions to handle the direct to/from database backup source.
 */

/**
 * A destination type for saving to a database server.
 *
 * @ingroup backup_restore
 */
abstract class BackupDatabase extends Backup {
  /**
   * @var DatabaseConnection
   *
   * The database connection to be used when doing a backup or restore.
   */
  protected $connection = NULL;


  /**
   * {@inheritdoc}
   */
  public function __construct($backup_name, $backup_target, array $settings) {
    parent::__construct($backup_name, $backup_target, $settings);
  }

  /**
   * {@inheritdoc}
   */
  public function typeLabel() {
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
    $connection = $this->getDatabaseConnection();
    $prefixes = $connection->getPrefixes();

    // Check if a default database prefix is set. If in use, only back up the
    // tables within that prefix. If prefixes exist for some tables but there
    // is no default prefix, maintain the normal behavior of just allowing all
    // tables.
    $prefixed_tables = array();
    $default_prefix = isset($prefixes['default']) ? $prefixes['default'] : NULL;
    if ($default_prefix) {
      unset($prefixes['default']);
      foreach ($all_tables as $table) {
        foreach ($prefixes as $non_prefixed_table => $prefix) {
          if ($table == $prefix . $non_prefixed_table) {
            $prefixed_tables[] = $table;
          }
        }
        if (strpos($table, $default_prefix) === 0) {
          $prefixed_tables[] = $table;
        }
      }
    }

    // Identify all cache tables as no-data tables.
    $nodata_tables = array('cache');
    foreach ($all_tables as $table_name) {
      if (strpos($table_name, 'cache_') === 0) {
        $nodata_tables[] = $table_name;
      }
    }

    // Prefix all the hard-coded no-data tables.
    foreach ($nodata_tables as $index => $table) {
      $nodata_tables[$index] = $connection->tablePrefix($table) . $table;
    }

    // Simpletest can create a lot of tables that do not need to be backed up,
    // but all of them start with the string 'simpletest' so they can be easily
    // excluded.
    $simpletest_tables = array();
    $valid_simpletest_tables = array(
      'simpletest',
      'simpletest_prefix',
    );
    foreach ($valid_simpletest_tables as $index => $table) {
      $valid_simpletest_tables[$index] = $connection->tablePrefix($table) . $table;
    }

    // When assembling the simpletest tables to exclude, make sure to not
    // exclude a prefixed table (meaning this backup is happening as part of a
    // SimpleTest execution).
    foreach ($all_tables as $table_name) {
      if (strpos($table_name, 'simpletest') === 0 &&
        !in_array($table_name, $valid_simpletest_tables) &&
        !in_array($table_name, $prefixed_tables)) {
        $simpletest_tables[] = $table_name;
      }
    }

    $included_tables = array();
    $excluded_tables = array();
    if ($prefixed_tables) {
      $included_tables = $prefixed_tables;
    }
    elseif ($simpletest_tables) {
      $excluded_tables = $simpletest_tables;
    }

    $settings += array(
      'nodata_tables' => $nodata_tables,
      'exclude_tables' => $excluded_tables,
      'include_tables' => $included_tables,
      'lock_tables' => FALSE,
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

    $form['help'] = array(
      '#type' => 'help',
      '#markup' => t('You may omit specific tables, or specific table data from the backup file. Only omit data that you know you will not need such as cache data, or tables from other applications. Restoring a partial backup can break your Backdrop install. It is usually best to leave the defaults.'),
    );
    $form['include_exclude'] = array(
      '#type' => 'select',
      '#title' => t('Include/exclude tables'),
      '#options' => array(
        'exclude' => t('Exclude specific tables'),
        'include' => t('Include specific tables'),
      ),
      '#default_value' => $settings['include_tables'] ? 'include' : 'exclude',
      '#attributes' => array(
        'class' => array('backup-include-exclude-mode'),
      ),
    );
    $form['include_tables'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Include only the following tables'),
      '#options' => backdrop_map_assoc($objects),
      '#default_value' => $settings['include_tables'],
      '#description' => t('Only the selected tables will be added to the backup.'),
      '#states' => array(
        'visible' => array(
          ':input.backup-include-exclude-mode' => array(
            'value' => 'include',
          ),
        ),
      ),
    );
    $form['exclude_tables'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Exclude the following tables altogether'),
      '#options' => backdrop_map_assoc($objects),
      '#default_value' => $settings['exclude_tables'],
      '#description' => t('The selected tables will not be added to the backup.'),
      '#states' => array(
        'visible' => array(
          ':input.backup-include-exclude-mode' => array(
            'value' => 'exclude',
          ),
        ),
      ),
    );
    $form['nodata_tables'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Exclude the data from the following tables'),
      '#options' => backdrop_map_assoc($objects),
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

    if ($this->settings['lock_tables']) {
      $this->lockTables();
    }

    // Switch to a different db if specified.
    $success = $this->backupDatabaseToFile($file);

    if ($this->settings['lock_tables']) {
      $this->unlockTables();
    }

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
      $compressed_file = $this->compress($file, self::COMPRESSION_GZIP, TRUE);
      if ($compressed_file) {
        $file = $compressed_file;
      }
      else {
        $this->log('The backup file could not be compressed. Check that the zlib PHP extension is installed and enough disk space is available.', array(), self::LOG_ERROR);
      }
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
  }

  /**
   * {@inheritdoc}
   */
  public function restore(BackupFile $file) {
    $lines_executed = 0;
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
      $lines_executed = $this->restoreDatabaseFromFile($file);

      if ($this->settings['verbose']) {
        $this->log('@count SQL commands executed.', array('@count' => $lines_executed), self::LOG_INFO);
      }
    }

    // Pull any logged messages out of the file and up to this class.
    $file_log = $file->getLog();
    foreach ($file_log as $log) {
      call_user_func_array($this->log, $log);
    }
    $file->clearLog();

    return $lines_executed;
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
      $this->connection = Database::getConnection($target, $key);
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
   * @return int|false
   *   The number of commands executed on restore. FALSE on failure.
   */
  abstract protected function restoreDatabaseFromFile(BackupFile $file);

  /**
   * Get a list of objects in the database.
   */
  protected function getObjectNames() {
    return array_merge($this->getTableNames(), $this->getViewNames());
  }

  /**
   * Get a list of tables in the database.
   *
   * @return string[]
   *   An array of all table names, numerically indexed.
   */
  abstract protected function getTableNames();

  /**
   * Get a list of views in the database.
   *
   * @return string[]
   *   An array of all view names, numerically indexed.
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
        // There's no need to lock excluded tables because it doesn't matter if
        // they are updated during the backup. If the "include_tables" option is
        // used, it takes precedence.
        if ($this->settings['include_tables'] && in_array($table, $this->settings['include_tables'])) {
          $tables[] = $table;
        }
        elseif (!in_array($table, $this->settings['exclude_tables'])) {
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
