<?php
// $Id$

// Parent plugin.
$plugin = array(
  'name' => 'node',
  'base_table' => 'node',
  'created_column' => 'created',
  'available_sorts' => array(
    'title:DESC' => t('By title')
  ),
  'get children' => 'views_ui_node_views_wizard_get_children',
  'get child' => 'views_ui_node_views_wizard_get_child',
  'form_wizard_class' => array(
    'file' => 'views_ui_node_views_wizard.class.php',
    'class' => 'ViewsUiNodeViewsWizard',
  ),
  'title' => t('All content'),
  'filters' => array(
    'status' => array(
      'value' => NODE_PUBLISHED,
      'table' => 'node',
      'field' => 'status',
    ),
  ),
);

function _views_ui_node_views_wizard_build_child($parent, $type, $name) {
  $child = $parent;
  $child['name'] =  $parent['name'] . ':' . $type;
  $child['title'] = t('!label content', array('!label' => $name));
  $child['filters']['type'] = array(
    'value' => array(
      $type => $type,
    ),
    'table' => 'node',
    'field' => 'type',
  );
  unset($child['get children']);
  unset($child['get child']);

  // For nodes, each child corresponds to a content type, i.e. a specific field
  // API bundle that the view will be limited to.
  $child['bundles'] = array($type);

  return $child;
}

function views_ui_node_views_wizard_get_children($plugin, $name) {
  $plugins = array($name => $plugin);
  foreach (node_type_get_names() as $type => $name) {
    $child = _views_ui_node_views_wizard_build_child($plugin, $type, $name);
    $plugins[$child['name']] = $child;
  }
  return $plugins;
}

function views_ui_node_views_wizard_get_child($plugin, $parent, $type) {
  $name = node_type_get_name($type);
  return _views_ui_node_views_wizard_build_child($plugin, $type, $name);
}
