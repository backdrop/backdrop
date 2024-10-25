<?php
/**
 * @file Contains the Backup base class.
 */

/**
 * Base class for creating backups.
 *
 * @ingroup backup_restore
 */
class BackupConfig extends Backup {
  const BACKUP_ID = 'config';

  /**
   * {@inheritdoc}
   */
  public function backup(BackupFile $file) {
    $config_type = $this->getConfigType();
    if (!$config_type) {
      return FALSE;
    }

    $file->pushExtension('tar');
    $file->pushExtension('gz');
    $file_path = $file->filePath();
    $config_storage = config_get_config_storage($config_type);
    try {
      $config_storage->exportArchive($file_path);
    }
    catch (ConfigStorageException $e) {
      $this->log($e->getMessage(), array(), 'error');
      return FALSE;
    }

    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function restore(BackupFile $file) {
    $config_type = $this->getConfigType();
    if (!$config_type) {
      return FALSE;
    }

    $config_storage = config_get_config_storage($config_type);
    try {
      $config_storage->importArchive($file->filePath());
    }
    catch (ConfigStorageException $e) {
      $this->log($e->getMessage(), array(), 'error');
      return FALSE;
    }

    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  function typeLabel() {
    return t('Configuration');
  }

  /**
   * {@inheritdoc}
   */
  public function applies() {
    // TODO: Implement applies() method.
  }

  public function backupSettingsForm() {

  }

  /**
   * Get the type of config being backed-up or restored.
   *
   * @return false|string
   *   Either "active", "staging", or FALSE.
   */
  protected function getConfigType() {
    $target = $this->getTarget();
    if ($target != 'config:active' && $target != 'config:staging') {
      return FALSE;
    }
    list(,$config_type) = explode(':', $target);
    return $config_type;
  }

}
