<?php

/**
 * @file
 * Describe hooks provided by the Entity Reference module.
 */

 /**
  * Register a new behavior plugin.
  *
  * @return
  *   Array of plugins keyed by the plugin name, and the following as value:
  *   - title: The title of the plugin.
  *   - description: The description of the plugin.
  *   - class: The class for the plugin.
  *   - behavior type: the type of behavior plugin.
  *   - access callback: the name of the function. Defaults to FALSE.
  *   - force enabled: used for disabled property of element.
  */
function hook_entityreference_behavior_plugins() {
  $plugins['views'] = array(
    'title' => t('Render Views filters as select list'),
    'description' => t('Provides a select list for Views filters on this field. This should not be used when there are over 100 entities, as it might cause an out of memory error.'),
    'class' => 'EntityReferenceBehaviorHandlerViewsFilterSelect',
    'behavior type' => 'field',
    'access callback' => FALSE,
    'force enabled' => FALSE,
  );
  return $plugins;
}

/**
 * Register a new selection plugin.
 *
 * @return
 *   Array of plugins keyed by the plugin name, and the following as value:
 *   - title: The title of the plugin.
 *   - class: The class for the plugin.
 *   - weight: a value to rank which classes get called first.
 */
function hook_entityreference_selection_plugins() {
  $plugins['base'] = array(
    'title' => t('Simple (with optional filter by bundle)'),
    'class' => 'EntityReferenceSelectionHandlerGeneric',
    'weight' => -100,
  );
  return $plugins;
}
