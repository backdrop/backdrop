<?php
$info = array(
  // Available colors and color labels used in theme.
  'fields' => array(
    'header' => t('Header background'),
    'base' => t('Mobile site name background'),
    'slogan' => t('Slogan background'),
    'titleslogan' => t('Site name and slogan'),
    'hovermenu' => t('Main menu hover background'),
    'menutoggle' => t('Responsive menu toggle'),

    'bg' => t('Main background'),
    'text' => t('Text color'),
    'link' => t('Link color'),
    'borders' => t('Borders'),
    'formfocusborder' => t('Form element borders when selected'),

    'primarytabs' => t('Primary Tabs Background'),
    'primarytabstext' => t('Primary Tabs Text'),
    'buttons' => t('Buttons and Breadcrumb'),

    'footer' => t('Footer background'),
    'footertext' => t('Footer text'),
    'footerborder' => t('Footer Border'),
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
        'hovermenu' => '#114a75',
        'menutoggle' => '#bcbbbb',

        'bg' => '#ffffff',
        'text' => '#000001',
        'link' => '#0073bd',
        'borders' => '#bbbbbb',
        'formfocusborder' => '#43afe4',

        'primarytabs' => '#586172',
        'primarytabstext' => '#010000',
        'buttons' => '#dee2ea',

        'footer' => '#fffeff',
        'footertext' => '#000100',
        'footerborder' => '#bbbbbc',
      ),
    ),
    'ice' => array(
      'title' => t('Ice'),
      'colors' => array(
        'header' => '#d0d0d0',
        'base' => '#999999',
        'slogan' => '#d0d0d1',
        'titleslogan' => '#000000',
        'hovermenu' => '#ffffff',
        'menutoggle' => '#24272c',

        'bg' => '#ffffff',
        'text' => '#2a2a2a',
        'link' => '#00809c',
        'borders' => '#24272c',
        'formfocusborder' => '#ffffff',

        'primarytabs' => '#888889',
        'primarytabstext' => '#d0d0d0',
        'buttons' => '#014958',

        'footer' => '#ffffff',
        'footertext' => '#000100',
        'footerborder' => '#c5c5c5',
      ),
    ),
    'plum' => array(
      'title' => t('Plum'),
      'colors' => array(
        'header' => '#4c1c58',
        'base' => '#000100',
        'slogan' => '#4c1c59',
        'titleslogan' => '#ffffff',
        'hovermenu' => '#9d408e',
        'menutoggle' => '#2c2c28',

        'bg' => '#fffdf7',
        'text' => '#301313',
        'link' => '#9d408d',
        'borders' => '#2c2c28',
        'formfocusborder' => '#edede7',

        'primarytabs' => '#2b2b2b',
        'primarytabstext' => '#301313',
        'buttons' => '#c9d9e3',

        'footer' => '#e7e7e7',
        'footertext' => '#000100',
        'footerborder' => '#89297a',
      ),
    ),
    'goldfish' => array(
      'title' => t('Goldfish'),
      'colors' => array(
        'header' => '#e0e4cc',
        'base' => '#fffeff',
        'slogan' => '#e0e4cd',
        'titleslogan' => '#4c5037',
        'hovermenu' => '#eda766',
        'menutoggle' => '#4c5036',

        'bg' => '#ffffff',
        'text' => '#3b3b3b',
        'link' => '#fa6900',
        'borders' => '#4c5036',
        'formfocusborder' => '#f3f4e9',

        'primarytabs' => '#eda766',
        'primarytabstext' => '#e0e4cc',
        'buttons' => '#4c5038',

        'footer' => '#f0f1e3',
        'footertext' => '#000100',
        'footerborder' => '#cfd7b2',
      ),
    ),
    'mocha' => array(
      'title' => t('Mocha'),
      'colors' => array(
        'header' => '#c63d0f',
        'base' => '#3b3738',
        'slogan' => '#c63d0e',
        'titleslogan' => '#ffffff',
        'hovermenu' => '#7e8f7d',
        'menutoggle' => '#dec0b4',

        'bg' => '#faf8f8',
        'text' => '#3b3738',
        'link' => '#c63d0f',
        'borders' => '#dec0b4',
        'formfocusborder' => '#ffffff',

        'primarytabs' => '#2b2b2b',
        'primarytabstext' => '#3b3738',
        'buttons' => '#dec0b4',

        'footer' => '#7e8f7c',
        'footertext' => '#000100',
        'footerborder' => '#6b7d68',
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
