<?php

/**
 * @file
 * Entity classes and controllers for User module.
 */

/**
 * Defines the user entity class.
 */
class User extends Entity {

  /**
   * The user ID.
   *
   * @var integer
   */
  public $uid;

  /**
   * The unique user name.
   *
   * @var string
   */
  public $name = '';

  /**
   * The user's password (hashed).
   *
   * @var string
   */
  public $pass;

  /**
   * The user's email address.
   *
   * @var string
   */
  public $mail = '';

  /**
   * The user's default theme.
   *
   * @var string
   */
  public $theme;

  /**
   * The user's signature.
   *
   * @var string
   */
  public $signature;

  /**
   * The user's signature format.
   *
   * @var string
   */
  public $signature_format;

  /**
   * The timestamp when the user was created.
   *
   * @var integer
   */
  public $created;

  /**
   * The timestamp when the user last accessed the site. A value of 0 means the
   * user has never accessed the site.
   *
   * @var integer
   */
  public $access = 0;

  /**
   * The timestamp when the user last logged in. A value of 0 means the user has
   * never logged in.
   *
   * @var integer
   */
  public $login = 0;

  /**
   * Whether the user is active (1) or blocked (0).
   *
   * @var integer
   */
  public $status = 0;

  /**
   * The user's timezone.
   *
   * @var string
   */
  public $timezone;

  /**
   * The user's langcode.
   *
   * @var string
   */
  public $langcode = LANGUAGE_NONE;

  /**
   * The user's preferred langcode for receiving emails and viewing the site.
   *
   * @var string
   */
  public $preferred_langcode = LANGUAGE_NONE;

  /**
   * The file ID of the user's picture.
   *
   * @var integer
   */
  public $picture = 0;

  /**
   * The email address used for initial account creation.
   *
   * @var string
   */
  public $init = '';

  /**
   * The user's roles.
   *
   * @var array
   */
  public $roles = array();

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return isset($this->uid) ? $this->uid : NULL;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'user';
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return user_format_name($this);
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    // Anonymous users do not have a URI.
    $uri = FALSE;
    if ($this->uid) {
      $uri = array(
        'path' => 'user/' . $this->uid,
        'options' => array(),
      );
    }
    return $uri;
  }
}


/**
 * Controller class for users.
 *
 * This extends the EntityDatabaseStorageController class, adding required
 * special handling for user objects.
 */
class UserStorageController extends EntityDatabaseStorageController {

  /**
   * Overrides EntityDatabaseStorageController::attachLoad().
   */
  function attachLoad(&$queried_users, $revision_id = FALSE) {
    // Build an array of user picture IDs so that these can be fetched later.
    $picture_fids = array();
    foreach ($queried_users as $key => $record) {
      if ($record->picture) {
        $picture_fids[] = $record->picture;
      }
      $queried_users[$key]->data = unserialize($record->data);
      $queried_users[$key]->roles = array();
      if ($record->uid) {
        $queried_users[$record->uid]->roles[DRUPAL_AUTHENTICATED_RID] = 'authenticated user';
      }
      else {
        $queried_users[$record->uid]->roles[DRUPAL_ANONYMOUS_RID] = 'anonymous user';
      }
    }

    // Add any additional roles from the database.
    $result = db_query('SELECT r.rid, r.name, ur.uid FROM {role} r INNER JOIN {users_roles} ur ON ur.rid = r.rid WHERE ur.uid IN (:uids)', array(':uids' => array_keys($queried_users)));
    foreach ($result as $record) {
      $queried_users[$record->uid]->roles[$record->rid] = $record->name;
    }

    // Add the full file objects for user pictures if enabled.
    if (!empty($picture_fids) && variable_get('user_pictures', 1) == 1) {
      $pictures = file_load_multiple($picture_fids);
      foreach ($queried_users as $entity) {
        if (!empty($entity->picture) && isset($pictures[$entity->picture])) {
          $entity->picture = $pictures[$entity->picture];
        }
      }
    }
    // Call the default attachLoad() method. This will add fields and call
    // hook_user_load().
    parent::attachLoad($queried_users, $revision_id);
  }

  /**
   * Overrides EntityDatabaseStorageController::create().
   */
  public function create(array $values) {
    if (!isset($values['created'])) {
      $values['created'] = REQUEST_TIME;
    }
    // Users always have the authenticated user role.
    $values['roles'][DRUPAL_AUTHENTICATED_RID] = 'authenticated user';

    return parent::create($values);
  }

  /**
   * Overrides EntityDatabaseStorageController::save().
   */
  public function save(EntityInterface $entity) {
    if (empty($entity->uid)) {
      $entity->uid = db_next_id(db_query('SELECT MAX(uid) FROM {users}')->fetchField());
      $entity->is_new = TRUE;
    }
    return parent::save($entity);
  }

  /**
   * Overrides EntityDatabaseStorageController::preSave().
   */
  protected function preSave(EntityInterface $entity) {
    // Update the user password if it has changed.
    if ($entity->isNew() || (!empty($entity->pass) && $entity->pass != $entity->original->pass)) {
      // Allow alternate password hashing schemes.
      require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'core/includes/password.inc');
      $entity->pass = user_hash_password(trim($entity->pass));
      // Abort if the hashing failed and returned FALSE.
      if (!$entity->pass) {
        throw new EntityMalformedException('The entity does not have a password.');
      }
    }

    if (!empty($entity->picture_upload)) {
      $entity->picture = $entity->picture_upload;
    }
    // Delete the picture if the submission indicates that it should be deleted
    // and no replacement was submitted.
    elseif (!empty($entity->picture_delete)) {
      $entity->picture = 0;
      file_usage_delete($entity->original->picture, 'user', 'user', $entity->uid);
      file_delete($entity->original->picture->fid);
    }

    if (!$entity->isNew()) {
      // Process picture uploads.
      if (!empty($entity->picture->fid) && (!isset($entity->original->picture->fid) || $entity->picture->fid != $entity->original->picture->fid)) {
        $picture = $entity->picture;
        // If the picture is a temporary file, move it to its final location
        // and make it permanent.
        if (!$picture->status) {
          $info = image_get_info($picture->uri);
          $picture_directory =  file_default_scheme() . '://' . variable_get('user_picture_path', 'pictures');

          // Prepare the pictures directory.
          file_prepare_directory($picture_directory, FILE_CREATE_DIRECTORY);
          $destination = file_stream_wrapper_uri_normalize($picture_directory . '/picture-' . $entity->uid . '-' . REQUEST_TIME . '.' . $info['extension']);

          // Move the temporary file into the final location.
          if ($picture = file_move($picture, $destination, FILE_EXISTS_RENAME)) {
            $entity->picture = $picture;
            file_usage_add($picture, 'user', 'user', $entity->uid);
          }
        }
        // Delete the previous picture if it was deleted or replaced.
        if (!empty($entity->original->picture->fid)) {
          file_usage_delete($entity->original->picture, 'user', 'user', $entity->uid);
          file_delete($entity->original->picture->fid);
        }
      }
      $entity->picture = empty($entity->picture->fid) ? 0 : $entity->picture->fid;

      // If the password is empty, that means it was not changed, so use the
      // original password.
      if (empty($entity->pass)) {
        $entity->pass = $entity->original->pass;
      }
    }

    // Prepare user roles.
    if (isset($entity->roles)) {
      $entity->roles = array_filter($entity->roles);
    }

    // Move account cancellation information into $entity->data.
    foreach (array('user_cancel_method', 'user_cancel_notify') as $key) {
      if (isset($entity->{$key})) {
        $entity->data[$key] = $entity->{$key};
      }
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::postSave().
   */
  protected function postSave(EntityInterface $entity, $update) {

    if ($update) {
      // If the password has been changed, delete all open sessions for the
      // user and recreate the current one.
      if ($entity->pass != $entity->original->pass) {
        drupal_session_destroy_uid($entity->uid);
        if ($entity->uid == $GLOBALS['user']->uid) {
          drupal_session_regenerate();
        }
      }

      // Remove roles that are no longer enabled for the user.
      $entity->roles = array_filter($entity->roles);

      // Reload user roles if provided.
      if ($entity->roles != $entity->original->roles) {
        db_delete('users_roles')
          ->condition('uid', $entity->uid)
          ->execute();

        $query = db_insert('users_roles')->fields(array('uid', 'rid'));
        foreach (array_keys($entity->roles) as $rid) {
          if (!in_array($rid, array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
            $query->values(array(
              'uid' => $entity->uid,
              'rid' => $rid,
            ));
          }
        }
        $query->execute();
      }

      // If the user was blocked, delete the user's sessions to force a logout.
      if ($entity->original->status != $entity->status && $entity->status == 0) {
        drupal_session_destroy_uid($entity->uid);
      }

      // Send emails after we have the new user object.
      if ($entity->status != $entity->original->status) {
        // The user's status is changing; conditionally send notification email.
        $op = $entity->status == 1 ? 'status_activated' : 'status_blocked';
        _user_mail_notify($op, $entity);
      }
    }
    else {
      // Save user roles.
      if (count($entity->roles) > 1) {
        $query = db_insert('users_roles')->fields(array('uid', 'rid'));
        foreach (array_keys($entity->roles) as $rid) {
          if (!in_array($rid, array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
            $query->values(array(
              'uid' => $entity->uid,
              'rid' => $rid,
            ));
          }
        }
        $query->execute();
      }
    }
  }
}
