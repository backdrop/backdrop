<?php

$info = array(
  // Available colors and color labels used in theme.
  'fields' => array(
    'header' => t('Header background'), // And active pager link.
    'sitename' => t('Site name background'),
    'slogan' => t('Slogan background'),
    'titleslogan' => t('Site name and slogan'), // And main menu links.
    'bg' => t('Main background'), // And submenu text, pager text, and drop down arrows
    'footer' => t('Footer background'),
    'text' => t('Text color'),
    'link' => t('Link color'),
    'hovermenu' => t('Hover menu link background'),
    'border_thingy' => t('Border thingies'), //These are all over the place.
    'form_focus_border' => t('Form element borders when selected'),
    'action_item_background' => t('Action item background'),
    'buttons' => t('Buttons'), // Action item link backgrounds, pager backgrounds, vertical tab backgrounds
    'buttons_hover' => t('Buttons Hover'), // Action item link backgrounds, vertical tab backgrounds
    'pager_hover' => t('Pager hover'), // Pager backgrounds
    'handles' => t('Handles'), // table drag handles, table sorts, and the plus/minus thing in the responsive menus.
  ),
  // Pre-defined color schemes.
  'schemes' => array(
    'default' => array(
      'title' => t('Basis (default)'),
      'colors' => array(
        'header' => '#20252e',
        'sitename' => '#0074bd',
        'slogan' => '#000',
        'titleslogan' => '#fffffe',
        'bg' => '#fff',
        'footer' => '#fffeff',
        'text' => '#000001',
        'link' => '#0073bd',
        'hovermenu' => '#114a75',
        'border_thingy' => '#bbb',
        'form_focus_border' => '#43afe4',
        'action_item_background' => '#586172',
        'buttons' => '#dee2ea',
        'buttons_hover' => '#d5d5d5',
        'pager_hover' => '#3c4350',
        'handles' => '#333',
      ),
    ),
  ),

  // CSS files (excluding @import) to rewrite with new color scheme.
  'css' => array(
    'css/base.css',
    'css/layout.css',
    'css/component/small-text-components.css',
    'css/component/header.css',
    'css/component/footer.css',
    'css/component/menu-dropdown.css',
    'css/component/menu-toggle.css',
    'css/component/backdrop-form.css',
    'css/component/tabledrag.css',
    'css/component/vertical-tabs.css',
    'css/component/fieldset.css',
    'css/component/dialog.css',
    'css/component/progress.css',
    'css/component/admin-tabs.css',
    'css/component/breadcrumb.css',
    'css/component/pager.css',
    'css/component/hero.css',
    'css/component/teasers.css',
    'css/component/comment.css',
    'css/component/caption.css',
    'css/skin.css',
  ),
);
