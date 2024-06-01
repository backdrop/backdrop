<?php
/**
 * @file
 * Provides the interface for user entities.
 */

/**
 * Defines a common interface for user entities.
 *
 * @see User
 * @see AnonymousUser
 */
interface UserInterface {

  /**
   * Returns the entity identifier (the entity's machine name or numeric ID).
   */
  public function id();

  /**
   * Returns the label of the entity.
   */
  public function label();

  /**
   * Returns the URI elements of the entity.
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
   * @return bool
   *   Whether access is allowed or not. If the entity type does not specify any
   *   access information, NULL is returned.
   */
  public function access($op, $account = NULL);

}
