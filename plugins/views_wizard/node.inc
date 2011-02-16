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
  'form_wizard_class' => array(
    'file' => 'views_ui_node_views_wizard.class.php',
    'class' => 'ViewsUiNodeViewsWizard',
  ),
  'title' => t('Content'),
  'filters' => array(
    'status' => array(
      'value' => NODE_PUBLISHED,
      'table' => 'node',
      'field' => 'status',
    ),
  ),
);

if (module_exists('statistics')) {
  $plugin['available_sorts']['node_counter-totalcount:DESC'] = t('By number of hits');
}
