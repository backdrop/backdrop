<?php
/**
 * @file Contains the Backup base class.
 */

/**
 * Base class for creating backups.
 *
 * @ingroup backup_restore
 */
abstract class Backup {

  const LOG_SUCCESS = 0;
  const LOG_ERROR = 1;
  const LOG_INFO = 2;

  const COMPRESSION_NONE = 'none';
  const COMPRESSION_GZIP = 'gzip';

  /**
   * An array of log entries. The array is unindexed and contains nested,
   * unindexed arrays, each with the following keys:
   * - 0: The log message.
   * - 1: The log variables for substitution.
   * - 2: The log message type (LOG_SUCCESS, LOG_ERROR, or LOG_INFO).
   *
   * @var array
   */
  protected $log = array();

  /**
   * The name of this backup. This is used as the basename of the backup file.
   *
   * The backup name should not include an extension, and should contain only
   * file-system safe characters (no slashes, colons, or periods).
   *
   * @var string
   */
  protected $backupName = '';

  /**
   * Indicates the item key to be backed up.
   *
   * This may be "db:default" in the case of the database, or "config:active" in
   * the case of config.
   *
   * @var string
   */
  protected $backupTarget = '';

  /**
   * The settings array to configure the backup and restore process.
   *
   * @var array
   */
  protected $settings = array();

  /**
   * Constructor for creating a new backup of any type.
   *
   * @param string $backup_name
   *   The name for the backup. This will be used as the base name for the file,
   *   not including any extensions. Extensions are determined by the backup
   *   handler class and the values in $settings, including if compression is
   *   enabled or not.
   * @param string $backup_target
   *   This key is very important, as it indicates the type of Backup to be
   *   performed. For example the name "db:default" will backup the primary
   *   database, and "config:active" will backup the active configuration.
   * @param array $settings
   *   An array of additional settings to be passed to the backup instance.
   *   These will be merged with the defaults provided by the defaultSettings()
   *   method.
   */
  public function __construct($backup_name, $backup_target, array $settings) {
    $this->backupName = $backup_name;
    $this->backupTarget = $backup_target;
    $this->settings = $settings + $this->defaultSettings();
  }

  /**
   * Get the name of the backup.
   *
   * @return string
   */
  public function getName() {
    return $this->backupName;
  }

  /**
   * Get the target of the backup.
   *
   * @return string
   */
  public function getTarget() {
    return $this->backupTarget;
  }

  /**
   * The translated name of the backup type.
   *
   * Examples are t("Database") or t("Configuration").
   *
   * @return string
   */
  abstract public function typeLabel();

  /**
   * An array of settings for this backup source.
   *
   * @return array
   */
  public function defaultSettings() {
    return array(
      'backup_max_time' => 1200,
      'backup_timeout_buffer' => 5,
      'memory_limit' => '256M',
      'verbose' => FALSE,
    );
  }

  /**
   * Determine if this backup source should be used for a particular target.
   *
   * This method needs to be implemented by all Backup classes. Up until PHP 7,
   * static methods could not be made abstract. Once PHP 7 is the minimum
   * requirement, this should be made abstract and the function body removed.
   *
   * @param string $target
   *   The target from which a backup will be created (or restored to). Examples
   *   are "db:default" or "config:active".
   *
   * @return boolean
   */
  public static function applies($target) {
    return FALSE;
  }

  /**
   * Run the full backup process, including generating the backup and saving it.
   *
   * @param BackupFile $file
   *   A BackupFile instance, used to write to the file handle.
   *
   * @return boolean
   *   TRUE if the backup was successful, FALSE on failure.
   */
  abstract public function backup(BackupFile $file);

  /**
   * Prepare a file to receive a backup.
   *
   * This step is not used in core but available for completeness.
   *
   * @param BackupFile $file
   *   The file to receive the backup. This is explicitly passed by reference
   *   to allow it to be reassigned to a new file if needed.
   *
   * @return void
   */
  public function preBackup(BackupFile &$file) {
    $this->prepareEnvironment();
  }

  /**
   * Change a file after a backup is complete.
   *
   * This step is usually used to compress a backup file.
   *
   * @param BackupFile $file
   *   The file that received the backup. This is explicitly passed by reference
   *   to allow it to be reassigned to a new file if needed.
   *
   * @return void
   */
  public function postBackup(BackupFile &$file) {
    // No-op.
  }

  /**
   * Restore a backup from a file.
   *
   * @param BackupFile $file
   *   A BackupFile instance, used to read from the file handle.
   *
   * @return boolean
   *   TRUE if the restore was successful, FALSE on failure.
   */
  abstract public function restore(BackupFile $file);

  /**
   * Prepare a file for restoration.
   *
   * This step is usually utilized to decompress a backup file.
   *
   * @param BackupFile $file
   *   The file containing the backup to be restored. This is explicitly passed
   *   by reference to allow it to be reassigned to a new file if needed.
   *
   * @return void
   */
  public function preRestore(BackupFile &$file) {
    $this->prepareEnvironment();
  }

  /**
   * Change a file after restoration.
   *
   * This step is usually used to delete up the decompressed backup file.
   *
   * @param BackupFile $file
   *   The file containing the backup that was restored. This is explicitly
   *   passed by reference to allow it to be reassigned to a new file if needed.
   *
   * @return void
   */
  public function postRestore(BackupFile &$file) {
    // No-op.
  }

  /**
   * Get the form for the backup settings for this destination.
   */
  public function backupSettingsForm() {
    return array();
  }

  /**
   * Log a message that occurs during a backup or migration process.
   *
   * Often a log will not be able to be saved anywhere, since Backdrop may not
   * be installed or in a working state, so log messages are held and can be
   * retrieved and cleared as needed.
   */
  protected function log($message, $replacements, $log_type) {
    $this->log[] = array($message, $replacements, $log_type);
  }

  /**
   * Return the log for this backup instance.
   *
   * @return array
   */
  public function getLog() {
    return $this->log;
  }

  /**
   * Clear the log for this backup instance.
   */
  public function clearLog() {
    $this->log = array();
  }

  /**
   * Determines if time for a page execution has run out.
   */
  protected function timeoutCheck() {
    // Max execution of 0 means unlimited.
    if (ini_get('max_execution_time') == 0) {
      return FALSE;
    }
    // Figure out when we should stop execution.
    $backup_max_time = max(ini_get('max_execution_time'), $this->settings['backup_max_time']);
    $timeout = (!empty($_SERVER['REQUEST_TIME']) ? $_SERVER['REQUEST_TIME'] : time()) + $backup_max_time - $this->settings['backup_timeout_buffer'];

    return (time() > $timeout);
  }

  /**
   * Compress a backup file with gzip.
   *
   * @return bool|BackupFile
   *   The newly created compressed file as BackupFile object. FALSE if failure.
   */
  public function compress(BackupFile $file, $format = self::COMPRESSION_GZIP, $delete_original = FALSE) {
    // Only gzip compression supported currently.
    if ($format !== self::COMPRESSION_GZIP) {
      return FALSE;
    }

    // If the file is already compressed, nothing to do here.
    if ($file->lastExtension() === 'gz') {
      return $file;
    }

    $success = FALSE;
    $compressed_file = clone $file;
    $compressed_file->pushExtension('gz');

    $source = $file->filePath();
    $destination = $compressed_file->filePath();
    $level = 9;

    if (function_exists('gzopen')) {
      $fp_out = gzopen($destination, 'wb'. $level);
      $fp_in = fopen($source, 'rb');
      if ($fp_out && $fp_in) {
        while (!feof($fp_in)) {
          gzwrite($fp_out, fread($fp_in, 1024 * 512));
        }
        $success = TRUE;
      }
      @fclose($fp_in);
      @gzclose($fp_out);
    }

    if ($success && $delete_original) {
      file_unmanaged_delete($source);
    }

    return $success ? $compressed_file : FALSE;
  }

  /**
   * Decompress a backup file with gzip.
   *
   * @return bool|BackupFile
   *   The newly decompressed file as BackupFile object. FALSE if failure.
   */
  public function decompress(BackupFile $file, $format = self::COMPRESSION_GZIP, $delete_original = FALSE) {
    // Only gzip compression supported currently.
    if ($format !== self::COMPRESSION_GZIP) {
      return FALSE;
    }

    // If the file is not compressed, nothing to do here.
    if ($file->lastExtension() !== 'gz') {
      return $file;
    }

    $success = FALSE;
    $decompressed_file = clone $file;
    $decompressed_file->popExtension();

    $source = $file->filePath();
    $destination = $decompressed_file->filePath();

    if (function_exists('gzopen')) {
      $fp_out = fopen($destination, 'wb');
      $fp_in = gzopen($source, 'rb');
      if ($fp_out && $fp_in) {
        while (!feof($fp_in)) {
          fwrite($fp_out, gzread($fp_in, 1024 * 512));
        }
        $success = TRUE;
      }
      @gzclose($fp_in);
      @fclose($fp_out);
    }

    if ($success && $delete_original) {
      file_unmanaged_delete($source);
    }

    return $success ? $decompressed_file : FALSE;
  }

  /**
   * Prepare the PHP environment for performing a backup or restore.
   */
  protected function prepareEnvironment() {
    // If not in 'safe mode', increase the maximum execution time.
    $backup_max_time = $this->settings['backup_max_time'];
    if (!ini_get('safe_mode') && strpos(ini_get('disable_functions'), 'set_time_limit') === FALSE && ini_get('max_execution_time') < $backup_max_time) {
      backdrop_set_time_limit($backup_max_time);
    }

    // Conditionally increase the memory limit.
    // Get the current PHP memory limit.
    $php_memory_limit = ini_get('memory_limit');

    // If the memory limit is set to -1, there's no need to do anything.
    if ($php_memory_limit != '-1') {
      // Convert the PHP memory limit to bytes.
      $php_memory_limit_bytes = parse_size($php_memory_limit);
      // Convert the instance settings memory limit to bytes.
      $backup_memory_limit_bytes = parse_size($this->settings['memory_limit']);

      // Increase the memory limit.
      if ($php_memory_limit_bytes < $backup_memory_limit_bytes) {
        // Ensure ini_set() is not disabled.
        if (!strpos(ini_get('disable_functions'), 'ini_set')) {
          // Convert bytes to megabytes, add the 'M' unit.
          $backup_memory_limit = round($backup_memory_limit_bytes / 1024 / 1024) . 'M';

          // Update the PHP memory limit.
          ini_set('memory_limit', $backup_memory_limit);
        }
      }
    }

    if ($this->settings['verbose']) {
      $this->log('Memory limit: %limit', array('%limit' => ini_get('memory_limit')), self::LOG_SUCCESS);
    }
  }
}

