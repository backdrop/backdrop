<?php
// $Id$

// Parent plugin.
$plugin = array(
  'name' => 'node_revision',
  'base_table' => 'node_revision',
  'created_column' => 'timestamp',
  'form_wizard_class' => array(
    'file' => 'views_ui_node_revision_views_wizard.class.php',
    'class' => 'ViewsUiNodeRevisionViewsWizard',
  ),
  'title' => t('Node revision'),
  'filters' => array(
    'status' => array(
      'value' => '1',
      'table' => 'node', // @todo - unclear if this should be node or node_revision
      'field' => 'status',
    ),
  ),
);

