<?php
/**
 * @file
 * Support for processing entity reference fields in Migrate.
 */

/**
 * Implements hook_migrate_api().
 */
function entityreference_migrate_api() {
  return array(
    'api' => 2,
    'field handlers' => array('MigrateEntityReferenceFieldHandler'),
  );
}

/**
 * Extended class for handling entityreference fields.
 */
class MigrateEntityReferenceFieldHandler extends MigrateSimpleFieldHandler {

  /**
   * Constructor.
   */
  public function __construct() {
    parent::__construct(array(
      'value_key' => 'target_id',
      'skip_empty' => TRUE,
    ));

    $this->registerTypes(array('entityreference'));
  }
}
