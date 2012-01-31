<?php

/**
 * Additional behaviors for a Entity Reference field.
 *
 * Implementations that wish to provide an implementation of this should
 * register it using CTools' plugin system.
 */
interface EntityReference_BehaviorHandler {

  /**
   * Alter the field schema.
   *
   * @see hook_field_schema()
   */
  public function schema_alter(&$schema, $field);

  /**
   * Alter the properties information of a field instance.
   *
   * @see entity_hook_field_info()
   */
  public function property_info_alter(&$info, $entity_type, $field, $instance, $field_type);

  /**
   * Act on loading entity reference fields of entities.
   *
   * @see hook_field_load()
   */
  public function load($entity_type, $entities, $field, $instances, $langcode, &$items);

  /**
   * Act on validating an entity reference field.
   *
   * @see hook_field_validate()
   */
  public function validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors);

  /**
   * Act on presaving an entity reference field.
   *
   * @see hook_field_presave()
   */
  public function presave($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after inserting an entity reference field.
   *
   * @see hook_field_insert()
   */
  public function insert($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after updating an entity reference field.
   *
   * @see hook_field_update()
   */
  public function update($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after deleting entity reference field.
   *
   * @see hook_field_delete()
   */
  public function delete($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Generate a settings form for this handler.
   */
  public function settingsForm();

  /**
   * Determine if handler should appear.
   */
  public function access($selection_class);
}

/**
 * An abstract implementation of EntityReference_BehaviorHandler.
 */
abstract class EntityReference_BehaviorHandler_Abstract implements EntityReference_BehaviorHandler {

  public function __construct($behavior, array $field, array $instance = NULL) {
    ctools_include('plugins');
    $plugin = ctools_get_plugins('entityreference', 'behavior', $behavior);
    $this->plugin = $plugin;

    if ($plugin['behavior type'] == 'field') {
      $this->settings = !empty($field['settings']['handler_settings']['behavior']) ? $field['settings']['handler_settings']['behavior'] : array();
    }
    else {
      $this->settings = !empty($instance['settings']['behavior']) ? $instance['settings']['behavior'] : array();
    }

    $this->status = !empty($settings['status']) ? $settings['status'] : FALSE;
    unset($this->settings['status']);

    $this->field = $field;
    $this->instance = $instance;
  }

  public function schema_alter(&$schema, $field) {}

  public function property_info_alter(&$info, $entity_type, $field, $instance, $field_type) {}

  public function load($entity_type, $entities, $field, $instances, $langcode, &$items) {}

  public function validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {}

  public function presave($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function insert($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function update($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function delete($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function settingsForm() {}

  public function access($selection_class) {
    return TRUE;
  }
}

/**
 * A broken implementation of EntityReference_BehaviorHandler.
 */
class EntityReference_BehaviorHandler_Broken extends EntityReference_BehaviorHandler_Abstract {
  public function settingsForm() {
    $form['behavior_handler'] = array(
      '#markup' => t('The selected behavior handler is broken.'),
    );
    return $form;
  }

  public function access($selection_class) {
    return TRUE;
  }
}
