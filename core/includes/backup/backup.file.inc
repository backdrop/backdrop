<?php
/**
 * @file Contains the BackupFile class.
 */

/**
 * Provides a utility object for reading and writing local backup files.
 */
class BackupFile {
  protected $backupName = "";
  protected $directory = "";
  protected $fileExtensions = array();
  protected $handle = NULL;
  protected $log = array();

  /**
   * Construct a file object used to save or load a backup from disk.
   *
   * @param string $backup_name
   *   The name of the backup file without the extension. The extension will be
   *   determined by the backup execution (depending on compression).
   * @param string $directory
   *   The URI or local path to the directory that will hold the backup. Stream
   *   wrappers may be used but they may not be reliable in restoration
   *   operations, if the database is not in a working state. Do not include
   *   a trailing slash.
   */
  public function __construct($backup_name, $directory) {
    $this->directory = $directory;
    $this->backupName = $backup_name;
  }

  /**
   * Get the backup name.
   */
  public function backupName() {
    return $this->backupName;
  }

  /**
   * Get the file directory;
   */
  public function directory() {
    return $this->directory;
  }

  /**
   * Get the full current filepath.
   */
  public function filePath() {
    return $this->directory . '/' . $this->fileName();
  }

  /**
   * Get the final filename.
   */
  public function fileName() {
    $extension_str = '.' . $this->extension();
    return $this->backupName . $extension_str;
  }

  /**
   * Get the file extension.
   */
  public function extension() {
    return implode('.', $this->fileExtensions);
  }

  /**
   * Get the file mimetype.
   */
  public function mimetype() {
    return file_get_mimetype($this->fileName());
  }

  /**
   * Add to the extension list.
   */
  public function pushExtension($extension) {
    $this->fileExtensions[] = $extension;
  }

  /**
   * Remove rom the extension list.
   */
  public function popExtension() {
    return array_pop($this->fileExtensions);
  }

  /**
   * Get the last file extension.
   */
  public function lastExtension() {
    return end($this->fileExtensions);
  }

  /**
   * Open a file for reading or writing.
   */
  public function open($write = FALSE, $binary = FALSE) {
    if (!$this->handle) {
      $path = $this->filepath();

      // Check if the file can be read/written.
      if ($write && ((file_exists($path) && !is_writable($path)) || !is_writable(dirname($path)))) {
        $this->log('The file %path is not writable.', array('%path' => $path), 'error');
        return FALSE;
      }
      if (!$write && !is_readable($path)) {
        $this->log('The file %path cannot be read.', array('%path' => $path), 'error');
        return FALSE;
      }

      // Open the file.
      $mode = ($write ? "w" : "r") . ($binary ? "b" : "");
      $this->handle = fopen($path, $mode);
      return $this->handle;
    }
    return NULL;
  }

  /**
   * Close a file when we're done reading/writing.
   */
  public function close() {
    fclose($this->handle);
    $this->handle = NULL;
  }

  /**
   * Write a line to the file.
   */
  public function write($data) {
    if (!$this->handle) {
      $this->handle = $this->open(TRUE);
    }
    if ($this->handle) {
      fwrite($this->handle, $data);
    }
  }

  /**
   * Read a line from the file.
   */
  public function read($size = NULL) {
    if (!$this->handle) {
      $this->handle = $this->open();
    }
    if ($this->handle && !feof($this->handle)) {
      return $size ? fread($this->handle, $size) : fgets($this->handle);
    }
    return NULL;
  }

  /**
   * Write the entire contents of the file.
   */
  public function putContents($data) {
    file_put_contents($this->filePath(), $data);
  }

  /**
   * Read the entire contents of the file.
   */
  public function getContents() {
    return file_get_contents($this->filePath());
  }

  /**
   * Get the file size.
   */
  public function getFileSize() {
    return filesize($this->filePath());
  }

  /**
   * Log any issues with reading or writing files.
   */
  protected function log($message, $replacements, $log_type) {
    $this->log[] = array($message, $replacements, $log_type);
  }

  /**
   * Return the array of current log messages.
   *
   * @return array
   */
  public function getLog() {
    return $this->log;
  }

  /**
   * Clear the current log messages.
   */
  public function clearLog() {
    $this->log = array();
  }

}
