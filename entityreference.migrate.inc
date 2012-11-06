<?php

/**
 * @file
 * Support for processing entity reference fields in Migrate.
 */

/**
 * Implement hook_migrate_api().
 */
function entityreference_migrate_api() {
  return array(
    'api' => 2,
    'field_handlers' => array('MigrateEntityReferenceFieldHandler'),
  );
}

class MigrateEntityReferenceFieldHandler extends MigrateSimpleFieldHandler {
  public function __construct() {
    parent::__construct(array(
      'value_key' => 'target_id',
      'skip_empty' => TRUE,
    ));

    $this->registerTypes(array('entityreference'));
  }
}
