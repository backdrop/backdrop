<?php

/**
 * Additional behaviors for a Entity Reference field.
 *
 * Implementations that wish to provide an implementation of this should
 * register it using CTools' plugin system.
 */
interface EntityReference_BehaviorHandler {

  /**
   * Constructor for the behavior.
   *
   * @param $behavior
   *   The name of the behavior plugin.
   */
  public function __construct($behavior);

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
   * Alter the views data of a field.
   *
   * @see entityreference_field_views_data()
   */
  public function views_data_alter(&$data, $field);

  /**
   * Act on loading entity reference fields of entities.
   *
   * @see hook_field_load()
   */
  public function load($entity_type, $entities, $field, $instances, $langcode, &$items);

  /**
   * Alter the empty status of a field item.
   *
   * @see hook_field_is_empty()
   */
  public function is_empty_alter(&$empty, $item, $field);

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
   * Act before inserting an entity reference field.
   *
   * @see hook_field_insert()
   */
  public function insert($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after inserting an entity reference field.
   *
   * @see hook_field_attach_insert()
   */
  public function postInsert($entity_type, $entity, $field, $instance);

  /**
   * Act before updating an entity reference field.
   *
   * @see hook_field_update()
   */
  public function update($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after updating an entity reference field.
   *
   * @see hook_field_attach_update()
   */
  public function postUpdate($entity_type, $entity, $field, $instance);

  /**
   * Act before deleting an entity with an entity reference field.
   *
   * @see hook_field_delete()
   */
  public function delete($entity_type, $entity, $field, $instance, $langcode, &$items);

  /**
   * Act after deleting an entity with an entity reference field.
   *
   * @see hook_field_attach_delete()
   */
  public function postDelete($entity_type, $entity, $field, $instance);

  /**
   * Act after inserting an entity.
   *
   * @see hook_entity_insert()
   */
  public function entityPostInsert($entity_type, $entity, $field, $instance);

  /**
   * Act after updating an entity.
   *
   * @see hook_entity_update()
   */
  public function entityPostUpdate($entity_type, $entity, $field, $instance);

  /**
   * Act after deleting an entity.
   *
   * @see hook_entity_delete()
   */
  public function entityPostDelete($entity_type, $entity, $field, $instance);

  /**
   * Generate a settings form for this handler.
   */
  public function settingsForm($field, $instance);

  /**
   * Determine if handler should appear.
   */
  public function access($field, $instance);
}

/**
 * An abstract implementation of EntityReference_BehaviorHandler.
 */
abstract class EntityReference_BehaviorHandler_Abstract implements EntityReference_BehaviorHandler {

  /**
   * The name of the behavior plugin.
   */
  protected $behavior;

  /**
   * The plugin definition.
   */
  protected $plugin;

  public function __construct($behavior) {
    $this->behavior = $behavior;

    ctools_include('plugins');
    $plugin = ctools_get_plugins('entityreference', 'behavior', $behavior);
    $this->plugin = $plugin;
  }

  public function schema_alter(&$schema, $field) {}

  public function property_info_alter(&$info, $entity_type, $field, $instance, $field_type) {}

  public function views_data_alter(&$data, $field) {}

  public function load($entity_type, $entities, $field, $instances, $langcode, &$items) {}

  public function is_empty_alter(&$empty, $item, $field) {}

  public function validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {}

  public function presave($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function insert($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function postInsert($entity_type, $entity, $field, $instance) {}

  public function update($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function postUpdate($entity_type, $entity, $field, $instance) {}

  public function delete($entity_type, $entity, $field, $instance, $langcode, &$items) {}

  public function postDelete($entity_type, $entity, $field, $instance) {}

  public function entityPostInsert($entity_type, $entity, $field, $instance) {}

  public function entityPostUpdate($entity_type, $entity, $field, $instance) {}

  public function entityPostDelete($entity_type, $entity, $field, $instance) {}

  public function settingsForm($field, $instance) {}

  public function access($field, $instance) {
    return TRUE;
  }
}

/**
 * A broken implementation of EntityReference_BehaviorHandler.
 */
class EntityReference_BehaviorHandler_Broken extends EntityReference_BehaviorHandler_Abstract {
  public function settingsForm($field, $instance) {
    $form['behavior_handler'] = array(
      '#markup' => t('The selected behavior handler is broken.'),
    );
    return $form;
  }
}
