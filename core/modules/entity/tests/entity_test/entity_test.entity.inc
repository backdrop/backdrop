<?php
/**
 * @file
 * Test entity classes.
 */

/**
 * Test class for the test_entity entity type.
 */
class TestEntity extends Entity {
  public $id;

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return $this->id;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'entity_test';
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return 'label';
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => 'test/' . $this->id,
    );
  }

  /**
   * Implements EntityInterface::createAccess().
   */
  public static function createAccess($bundle = NULL, $account = NULL) {
    return NULL;
  }

  /**
   * Implements EntityInterface::access().
   */
  public function access($op, $account = NULL) {
    return NULL;
  }

}
