<?php

/**
 * @file
 * Views wizard for user views.
 */

$plugin = array(
  'name' => 'users',
  'base_table' => 'users',
  'created_column' => 'created',
  'form_wizard_class' => array(
    'file' => 'views_ui_users_views_wizard.class.php',
    'class' => 'ViewsUiUsersViewsWizard',
  ),
  'title' => t('Users'),
  'filters' => array(
    'status' => array(
      'value' => '1',
      'table' => 'users',
      'field' => 'status',
    ),
  ),
);

