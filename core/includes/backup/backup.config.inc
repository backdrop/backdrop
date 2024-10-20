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
    $target = $this->getTarget();
    if ($target != 'config:active' && $target != 'config:staging') {
      return FALSE;
    }
    list(,$config_type) = explode(':', $target);

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

  }

  /**
   * {@inheritdoc}
   */
  function typeLabel() {
    // TODO: Implement typeLabel() method.
  }

  /**
   * {@inheritdoc}
   */
  public function applies() {
    // TODO: Implement applies() method.
  }

  public function backupSettingsForm() {

  }
}
