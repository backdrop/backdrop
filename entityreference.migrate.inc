<?php

/**
 * @file
 * Support for processing entity reference fields in Migrate.
 */

/**
 * Implement hook_migrate_api().
 */
function entityreference_migrate_api() {
  return array('api' => 2);
}

class MigrateEntityReferenceFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('entityreference'));
  }

  public function prepare($entity, array $field_info, array $instance, array $values) {
    $migration = Migration::currentMigration();
    $arguments = (isset($values['arguments']))? $values['arguments']: array();
    $language = $this->getFieldLanguage($entity, $field_info, $arguments);
    // Setup the standard Field API array for saving.
    $delta = 0;
    foreach ($values as $value) {
      $return[$language][$delta]['target_id'] = $value;
      $delta++;
    }
    if (!isset($return)) {
      $return = NULL;
    }
    return $return;
  }
}
