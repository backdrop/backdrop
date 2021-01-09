<?php
/**
 * @file
 * Provides an interface and a base class for entities.
 */

/**
 * Defines a common interface for all entity objects.
 */
interface EntityInterface {

  /**
   * Constructs a new entity object.
   *
   * @param $values
   *   An array of values to set, keyed by property name. If the entity type
   *   has bundles, the bundle key has to be specified.
   */
  public function __construct(array $values);

  /**
   * Returns the entity identifier (the entity's machine name or numeric ID).
   *
   * @return
   *   The identifier of the entity, or NULL if the entity does not yet have
   *   an identifier.
   */
  public function id();

  /**
   * Returns whether the entity is new.
   *
   * Usually an entity is new if no ID exists for it yet. However, entities may
   * be enforced to be new with existing IDs too.
   *
   * @return
   *   TRUE if the entity is new, or FALSE if the entity has already been saved.
   */
  public function isNew();

  /**
   * Returns the type of the entity.
   *
   * @return
   *   The type of the entity.
   */
  public function entityType();

  /**
   * Returns the bundle of the entity.
   *
   * @return
   *   The bundle of the entity. Defaults to the entity type if the entity type
   *   does not make use of different bundles.
   */
  public function bundle();

  /**
   * Returns the label of the entity.
   *
   * @return
   *   The label of the entity, or NULL if there is no label defined.
   */
  public function label();

  /**
   * Returns the URI elements of the entity.
   *
   * @return
   *   An array containing the 'path' and 'options' keys used to build the URI
   *   of the entity, and matching the signature of url(). NULL if the entity
   *   has no URI of its own.
   */
  public function uri();

  /**
   * Determines whether the given user has access to create this type of entity.
   *
   * @param string $bundle
   *   (optional) The entity's bundle to check create access against.
   * @param User|AnonymousUser|object $account
   *   (optional) The user to check for. Leave it to NULL to check for the
   *   global user.
   *
   * @return bool
   *   Whether access is allowed or not. If the entity type does not specify any
   *   access information, NULL is returned.
   */
  public static function createAccess($bundle = NULL, $account = NULL);

  /**
   * Determines whether the given user has access to this entity.
   *
   * @param string $op
   *   The operation being performed. One of 'view', 'update', 'create' or
   *   'delete'.
   * @param User|AnonymousUser|object $account
   *   (optional) The user to check for. Leave it to NULL to check for the
   *   global user.
   *
   * @return bool|NULL
   *   Whether access is allowed or not. If the entity type does not specify any
   *   access information, NULL is returned.
   */
  public function access($op, $account = NULL);

  /**
   * Saves an entity permanently.
   *
   * @return
   *   Either SAVED_NEW or SAVED_UPDATED, depending on the operation performed.
   *
   * @throws EntityStorageException
   *   In case of failures an exception is thrown.
   */
  public function save();

  /**
   * Deletes an entity permanently.
   *
   * @throws EntityStorageException
   *   In case of failures an exception is thrown.
   */
  public function delete();

  /**
   * Creates a duplicate of the entity.
   *
   * @return EntityInterface
   *   A clone of the current entity with all identifiers unset, so saving
   *   it inserts a new entity into the storage system.
   */
  public function createDuplicate();

  /**
   * Returns a single value from a field on the entity.
   *
   * @param string $field_name
   *   The machine name of the field.
   * @param string $value_key
   *   The key indicating the primary data value for the field. Defaults to
   *   'value' which is used in both text and number fields. For fields which do
   *   not have a 'value' key such as terms (which use a 'tid' key) or files
   *   (which use a 'fid' key), the key must be specified.
   * @param string $langcode (optional)
   *   The language code for the requested language. Defaults to the
   *   current language.
   *
   * @return (array)
   *   All field values, or an empty array if the field is empty, if the field does not
   *   contain the $value_key specified, or if the entity does not contain the
   *   $field_name specified.
   *
   * @since 1.13.0 Method added.
   */
  public function getFieldValue($field_name, $value_key = 'value', $langcode = NULL);

  /**
   * Returns all values from a field on the entity.
   *
   * @param string $field_name
   *   The machine name of the field.
   * @param string $value_key
   *   The key indicating the primary data value for the field. Defaults to
   *   'value' which is used in both text and number fields. For fields which do
   *   not have a 'value' key such as terms (which use a 'tid' key) or files
   *   (which use a 'fid' key), the key must be specified.
   * @param string $langcode (optional)
   *   The language code for the requested language. Defaults to the
   *   current language.
   *
   * @return (array)
   *   All field values, or an empty array if the field is empty, if the field does not
   *   contain the $value_key specified, or if the entity does not contain the
   *   $field_name specified.
   *
   * @since 1.13.0 Method added.
   */
  public function getFieldValues($field_name, $value_key = 'value', $langcode = NULL);

  /**
   * Returns the revision identifier of the entity.
   *
   * @return
   *   The revision identifier of the entity, or NULL if the entity does not
   *   have a revision identifier.
   *
   * @since 1.16.0 Method added.
   */
  public function getRevisionId();

  /**
   * Checks if this entity is the active revision.
   *
   * @return bool
   *   TRUE if the entity is the active revision, FALSE otherwise.
   *
   * @since 1.16.0 Method added.
   */
  public function isActiveRevision();

  /**
   * Sets this revision as the active revision.
   *
   * @since 1.16.0 Method added.
   */
  public function setIsActiveRevision();
}

/**
 * Defines a base entity class.
 */
abstract class Entity extends stdClass implements EntityInterface {

  /**
   * Indicates whether this is the active revision.
   *
   * @var bool
   */
  public $is_active_revision = TRUE;

  /**
   * Constructs a new entity object.
   */
  public function __construct(array $values = array()) {
    // Set initial values.
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }
  }

  /**
   * Implements EntityInterface::isNew().
   */
  public function isNew() {
    // We support creating entities with pre-defined IDs to ease migrations.
    // For that the "is_new" property may be set to TRUE.
    return !empty($this->is_new) || !$this->id();
  }

  /**
   * Implements EntityInterface::bundle().
   *
   * Entity types that do not support bundles return the entity type as their
   * bundle. Subclasses should override this method if supporting bundles.
   */
  public function bundle() {
    return $this->entityType();
  }

  /**
   * Implements EntityInterface::createAccess().
   */
  public static function createAccess($bundle = NULL, $account = NULL) {
    return TRUE;
  }

  /**
   * Implements EntityInterface::access().
   */
  public function access($op, $account = NULL) {
    return TRUE;
  }

  /**
   * Implements EntityInterface::save().
   */
  public function save() {
    return entity_get_controller($this->entityType())->save($this);
  }

  /**
   * Implements EntityInterface::delete().
   */
  public function delete() {
    if (!$this->isNew()) {
      entity_get_controller($this->entityType())->delete(array($this->id()));
    }
  }

  /**
   * Implements EntityInterface::createDuplicate().
   */
  public function createDuplicate() {
    $entity_info = entity_get_info($this->entityType());
    $duplicate = clone $this;
    $duplicate->{$entity_info['entity keys']['id']} = NULL;
    if (isset($entity_info['entity_keys']['revision'])) {
      $duplicate->{$entity_info['entity keys']['revision']} = NULL;
    }
    return $duplicate;
  }

  /**
   * Implements EntityInterface::getFieldValue().
   */
  public function getFieldValue($field_name, $value_key = 'value', $langcode = NULL) {
    $values = $this->getFieldValues($field_name, $value_key, $langcode);
    return empty($values) ? NULL : reset($values);
  }

  /**
   * Implements EntityInterface::getFieldValues().
   */
  public function getFieldValues($field_name, $value_key = 'value', $langcode = NULL) {
    $values = array();
    if ($field_items = field_get_items($this->entityType(), $this, $field_name, $langcode)) {
      foreach ($field_items as $item) {
        if (isset($item[$value_key])) {
          $values[] = $item[$value_key];
        }
      }
    }
    return $values;
  }

  /**
   * Implements Backdrop\entity\EntityInterface::getRevisionId().
   */
  public function getRevisionId() {
    return NULL;
  }

  /**
   * Implements Backdrop\entity\EntityInterface::isActiveRevision().
   */
  public function isActiveRevision() {
    return $this->is_active_revision;
  }

  /**
   * Implements Backdrop\entity\EntityInterface::setActiveRevision().
   */
  public function setIsActiveRevision() {
    $this->is_active_revision = TRUE;
  }
}

/**
 * Defines an exception thrown when a malformed entity is passed.
 */
class EntityMalformedException extends Exception { }
