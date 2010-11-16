<?php
// $Id$

// Parent plugin.
$plugin = array(
  'name' => 'node',
  'get children' => 'views_ui_node_views_wizard_get_children',
  'get child' => 'views_ui_node_views_wizard_get_child',
  'classes' => array(
    'node_class',
  ),
  'node_class' => 'views_ui_node_views_wizard',
);

function views_ui_node_views_wizard_get_children($plugin, $name) {
  $plugins = array($name => $plugin);
  foreach (node_type_get_names() as $type => $name) {
    $child = $plugin;
    $child['name'] =  $plugin['name'] . ':' . $type;
    unset($child['get children']);
    $plugins[$child['name']] = $child;
  }
  return $plugins;
}


function views_ui_node_views_wizard_get_child($plugin, $parent, $type) {
  $child = $plugin;
  $child['name'] =  $plugin['name'] . ':' . $type;
  unset($child['get children']);
  return $child;
}