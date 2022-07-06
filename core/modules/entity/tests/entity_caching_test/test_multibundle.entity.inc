<?php
/**
 * @file
 * Test entity classes.
 */

/**
 * Test class for the test_multibundle entity type.
 */
class TestMultiBundle extends Entity {
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
    return 'test_multibundle';
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return 'label';
  }

  /**
   * Implements EntityInterface::bundle().
   */
  public function bundle() {
    return $this->type;
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => 'test_multibundle/' . $this->id,
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
