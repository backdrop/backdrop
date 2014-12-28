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
   * Construction function.
   */
  public function __construct() {
    parent::__construct(array(
      'value_key' => 'target_id',
      'skip_empty' => TRUE,
    ));

    $this->registerTypes(array('entityreference'));
  }

  /**
   * Field-level prepare.
   *
   * Converts incoming data into the proper field arrays for entityreference
   * fields.
   *
   * @param object $entity
   *   The destination entity which will hold the field arrays.
   * @param array $field_info
   *   Metadata for the entityreference field being populated.
   * @param array $instance
   *   Metadata for this instance of the entityreference field being populated.
   * @param array $values
   *   Array of values for the entityreference field.
   *
   * @return array|null
   *   An array of entityreference fields.
   */
  public function prepare($entity, array $field_info, array $instance, array $values) {
    $arguments = (isset($values['arguments'])) ? $values['arguments'] : array();
    $language = $this->getFieldLanguage($entity, $field_info, $arguments);
    // Setup the standard Field API array for saving.
    $delta = 0;
    if (!is_array(reset($values))) {
      $values = array($values);
    }
    foreach (array_filter($values) as $value) {
      $return[$language][$delta]['target_id'] = reset($value);
      $delta++;
    }
    // Return NULL for no values in the field.
    return isset($return) ? $return : NULL;
  }
}
