<?php
// $Id$

// Parent plugin.
// @todo Is the module_exists() necessary?
if (module_exists('comment')) {
  $plugin = array(
    'name' => 'comment',
    'base_table' => 'comment',
    'form_wizard_class' => array(
      'file' => 'views_ui_comment_views_wizard.class.php',
      'class' => 'ViewsUiCommentViewsWizard',
    ),
    'title' => t('Comments'),
    'filters' => array(
      'status' => array(
        'operator' => '=',
        'value' => COMMENT_PUBLISHED,
        'table' => 'comment',
        'field' => 'status',
      ),
    ),
  );
}
