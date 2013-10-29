<?php

/**
 * Implements hook_plugin_info
 */
function entityreference_plugin_info($type) {
  static $plugins = array();
  
  // We never need to run this more than once in a request, so if $plugins has data, we return it earlier.
  if (array_key_exists($type, $plugins)) return $plugins[$type];
  
  if (in_array(__FUNCTION__, variable_get('drupal_alter_allowed', array()))) {
    drupal_alter(__FUNCTION__, $plugins);
  }
  
  if (module_exists('taxonomy')) {
    $plugins['behavior']['taxonomy_index'] = array(
      'title' => t('Taxonomy index'),
      'description' => t('Include the term references created by instances of this field carried by node entities in the core {taxonomy_index} table. This will allow various modules to handle them like core term_reference fields.'),
      'class' => 'EntityReferenceBehavior_TaxonomyIndex',
      'behavior type' => 'instance',
      'force enabled' => TRUE,
    );
  }
  
  if (module_exists('views')) {
    $plugins['selection']['views'] = array(
      'title' => t('Views: Filter by an entity reference view'),
      'class' => 'EntityReference_SelectionHandler_Views',
      'weight' => 0,
    );
      
    $plugins['behavior']['views_select'] = array(
      'title' => t('Render Views filters as select list'),
      'description' => t('Provides a select list for Views filters on this field. This should not be used when there are over 100 entities, as it might cause an out of memory error.'),
      'class' => 'EntityReferenceBehavior_ViewsFilterSelect',
      'behavior type' => 'field',
    );
  }

  $plugins['selection']['base'] = array(
    'title' => t('Simple (with optional filter by bundle)'),
    'class' => 'EntityReference_SelectionHandler_Generic',
    'weight' => -100,
  );
  
  // Behavior plugins need some defaults.
  foreach ($plugins['behavior'] as &$plugin) {
    $plugin += array(
      'description' => '',
      'behavior type' => 'field',
      'access callback' => FALSE,
      'force enabled' => FALSE,
    );
  }
  
  return $plugins[$type];
}

function entityreference_load_plugin($type, $handler, $fallback) {
  $plugins = entityreference_plugin_info($type);
  $module = array_key_exists('module', $plugins[$handler]) ? $plugins[$handler]['module'] : 'entityreference';
  $class = $plugins[$handler]['class'];
  module_load_include('class.php', $module, "plugins/$type/$class");
  $class = class_exists($class) ? $class : $fallback;
  return $class;
}