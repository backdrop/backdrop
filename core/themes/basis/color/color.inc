<?php

$info = array(
  // Available colors and color labels used in theme.
  'fields' => array(
    'header' => t('Header background'),
    'base' => t('Site name background'),
    'slogan' => t('Slogan background'),
    'titleslogan' => t('Site name and slogan'),
    'bg' => t('Main background'),
    'footer' => t('Footer background'),
    'text' => t('Text color'),
    'link' => t('Link color'),
    'hovermenu' => t('Main menu hover background'),
    'primarytabs' => t('Primary Tabs background'),
    'borders' => t('Borders'),
    'formfocusborder' => t('Form element borders when selected'),
    'buttons' => t('Buttons and Breadcrumb'),
  ),
  // Pre-defined color schemes.
  'schemes' => array(
    'default' => array(
      'title' => t('Basis (default)'),
      'colors' => array(
        'header' => '#20252e',
        'base' => '#0074bd',
        'slogan' => '#000000',
        'titleslogan' => '#fffffe',
        'bg' => '#ffffff',
        'footer' => '#fffeff',
        'text' => '#000001',
        'link' => '#0073bd',
        'hovermenu' => '#114a75',
        'primarytabs' => '#586172',
        'borders' => '#bbbbbb',
        'formfocusborder' => '#43afe4',
        'buttons' => '#dee2ea',
      ),
    ),
  ),

  'blend_target' => '#ffffff',

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
