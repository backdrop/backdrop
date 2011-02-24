<?php
// $Id: ctools_custom_content.inc,v 1.2 2010/10/11 22:18:23 sdboyer Exp $

$plugin = array(
  'schema' => 'views_view',
  'access' => 'administer views',

  'menu' => array(
    'menu item' => 'views',
    'menu title' => 'Views',
    'menu description' => 'Manage customized lists of content.',
  ),

  'title singular' => t('view'),
  'title singular proper' => t('View'),
  'title plural' => t('views'),
  'title plural proper' => t('Views'),

  'handler' => 'views_ui',

  'strings' => array(
    'confirmation' => array(
      'revert' => array(
        'information' => t('This action will permanently remove any customizations made to this view.'),
        'success' => t('The view has been reverted.'),
      ),
      'delete' => array(
        'information' => t('This action will permanently remove the view from your database.'),
        'success' => t('The view has been deleted.'),
      ),
    ),
  ),
);

