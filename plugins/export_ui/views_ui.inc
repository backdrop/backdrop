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
);

