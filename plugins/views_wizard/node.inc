<?php
// $Id$

// Parent plugin.
$plugin = array(
  'name' => 'node',
  'base_table' => 'node',
  'get children' => 'views_ui_node_views_wizard_get_children',
  'get child' => 'views_ui_node_views_wizard_get_child',
  'form_wizard_class' => array(
    'file' => 'views_ui_node_views_wizard.class.php',
    'class' => 'ViewsUiNodeViewsWizard',
  ),
  'title' => t('All posts'),
  'filters' => array(
    'status' => array(
      'operator' => '=',
      'value' => '1',
      'table' => 'node',
      'field' => 'status',
    ),
  ),
);

function views_ui_node_views_wizard_get_children($plugin, $name) {
  $plugins = array($name => $plugin);
  foreach (node_type_get_names() as $type => $name) {
    $child = $plugin;
    $child['name'] =  $plugin['name'] . ':' . $type;
    $child['title'] = t('!label posts', array('!label' => $name));
    $child['filters']['type'] = array(
      'operator' => 'in',
      'value' => array(
        'site' => 'site',
      ),
      'table' => 'node',
      'field' => 'type',
    );
    unset($child['get children']);
    unset($child['get child']);
    $plugins[$child['name']] = $child;
  }
  return $plugins;
}

function views_ui_node_views_wizard_get_child($plugin, $parent, $type) {
  $child = $plugin;
  $child['name'] =  $plugin['name'] . ':' . $type;
  unset($child['get children']);
  unset($child['get child']);
  return $child;
}
