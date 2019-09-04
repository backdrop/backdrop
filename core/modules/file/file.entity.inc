<?php
/**
 * @file
 * Entity controller and class for files.
 */

/**
 * Defines the file entity class.
 */
class File extends Entity {

  /**
   * The file ID.
   *
   * @var integer
   */
  public $fid;

  /**
   * The file type (bundle).
   *
   * @var string
   */
  public $type;

  /**
   * The file language code.
   *
   * @var string
   */
  public $langcode = LANGUAGE_NONE;

  /**
   * The uid of the user who is associated with the file.
   *
   * @var integer
   */
  public $uid;

  /**
   * Name of the file with no path components.
   *
   * This may differ from the basename of the URI if the file is renamed to
   * avoid overwriting an existing file.
   *
   * @var string
   */
  public $filename;

  /**
   * The URI to access the file (either local or remote).
   *
   * @var string
   */
  public $uri;

  /**
   * The file's MIME type.
   *
   * @var string
   */
  public $filemime;

  /**
   * The size of the file in bytes.
   *
   * @var integer
   */
  public $filesize;

  /**
   * A field indicating the status of the file.
   *
   * Two status are defined in core: temporary (0) and permanent (1).
   * Temporary files older than BACKDROP_MAXIMUM_TEMP_FILE_AGE will be removed
   * during a cron run.
   *
   * @var integer
   */
  public $status;

  /**
   * UNIX timestamp for when the file was last saved.
   *
   * @var integer
   */
  public $timestamp;

  /**
   * Constructor for file entities.
   */
  public function __construct(array $values = array()) {
    // Automatically detect filename if not set.
    if (!isset($values['filename']) && isset($values['uri'])) {
      $values['filename'] = backdrop_basename($values['uri']);
    }

    // Automatically detect filemime if not set.
    if (!isset($values['filemime']) && isset($values['uri'])) {
      $values['filemime'] = file_get_mimetype($values['uri']);
    }

    parent::__construct($values);
  }

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return isset($this->fid) ? $this->fid : NULL;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'file';
  }

  /**
   * Implements EntityInterface::bundle().
   */
  public function bundle() {
    return $this->type;
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return $this->filename;
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => $this->uri,
      'options' => array(),
    );
  }

  /**
   * Overrides Entity::access().
   *
   * The "view" operation is not supported on the File Entity itself because it
   * has no page callback (yet), but there is separately file_download_access()
   * to check if a file is downloadable.
   *
   * @param string $op
   *   The operation to be performed on the file. Possible values are:
   *   - create
   *   - update
   *   - delete
   * @param User|AnonymousUser|object $account
   *   (optional) The user to check for. Leave it to NULL to check for the
   *   global user.
   *
   * @return bool
   *   TRUE if access is granted, FALSE otherwise.
   */
  public function access($op, $account = NULL) {
    if ($op == 'create') {
      return self::createAccess(NULL, $account);
    }
    elseif (!in_array($op, array('update', 'delete'), TRUE)) {
      // If the $op was not one of the supported ones, we return access denied.
      return FALSE;
    }

    // If no user object is supplied, the access check is for the current user.
    if (empty($account)) {
      $account = $GLOBALS['user'];
    }

    if ($op == 'update') {
      return user_access('manage files', $account);
    }
    elseif ($op == 'delete') {
      return user_access('delete files', $account);
    }

    return FALSE;
  }

}

/**
 * File storage controller for files.
 */
class FileStorageController extends EntityDatabaseStorageController {
  /**
   * Overrides EntityDatabaseStorageController::presave().
   *
   * @param File $entity
   *   The file entity about to be saved.
   */
  protected function preSave(EntityInterface $entity) {
    $entity->timestamp = REQUEST_TIME;
    $entity->filesize = filesize($entity->uri);
    if (!isset($entity->langcode)) {
      // Default the file's language code to none, because files are language
      // neutral more often than language dependent.
      $entity->langcode = LANGUAGE_NONE;
    }

    // The file type is used as a bundle key, and therefore, must not be NULL.
    // It defaults to FILE_TYPE_NONE when loaded via file_load(), but in case
    // file_save() is called on a new file object, default it here too.
    if (!isset($entity->type)) {
      $entity->type = FILE_TYPE_NONE;
    }

    // If the file isn't already assigned a real type, determine what type should
    // be assigned to it.
    if ($entity->type === FILE_TYPE_NONE) {
      $type = file_get_type($entity);
      if (isset($type)) {
        $entity->type = $type;
      }
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::preDelete().
   *
   * @param File[] $entities
   *   The file entities about to be deleted.
   */
  public function preDelete($entities) {
    foreach ($entities as $entity) {
      // Delete the actual file. Failures due to invalid files and files that
      // were already deleted are logged to watchdog but ignored, the
      // corresponding file entity will be deleted.
      file_unmanaged_delete($entity->uri);
    }
    // Delete corresponding file usage entries.
    db_delete('file_usage')
      ->condition('fid', array_keys($entities), 'IN')
      ->execute();
  }

}
