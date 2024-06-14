<?php
/**
 * @file
 * Test entity classes.
 */

/**
 * Test class for the test_cacheable entity type.
 */
class TestCacheable extends Entity {
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
    return 'test_cacheable';
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
      'path' => 'test_cacheable/' . $this->id,
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
