<?php
// $Id$

// Parent plugin.
if (module_exists('comment')) {
  $plugin = array(
    'name' => 'comment',
    'base_table' => 'comment',
    'created_column' => 'created',
    'form_wizard_class' => array(
      'file' => 'views_ui_comment_views_wizard.class.php',
      'class' => 'ViewsUiCommentViewsWizard',
    ),
    'title' => t('Comments'),
    'filters' => array(
      'status' => array(
        'value' => COMMENT_PUBLISHED,
        'table' => 'comment',
        'field' => 'status',
      ),
      'status_node' => array(
        'value' => NODE_PUBLISHED,
        'table' => 'node',
        'field' => 'status',
        'relationship' => 'nid',
      ),
    ),
  );
}
