<?php
/**
 * @file
 * Theme functions for the Field UI module.
 */

/**
 * Returns HTML for Field UI overview tables.
 *
 * @param $variables
 *   An associative array containing:
 *   - elements: An associative array containing a Form API structure to be
 *     rendered as a table.
 *
 * @ingroup themeable
 */
function theme_field_ui_table($variables) {
  $elements = $variables['elements'];
  $table = [];
  $js_settings = [];

  // Add table headers and attributes.
  foreach (['header', 'attributes'] as $key) {
    if (isset($elements["#$key"])) {
      $table[$key] = $elements["#$key"];
    }
  }

  // Determine the colspan to use for region rows, by checking the number of
  // columns in the headers.
  $columns_count = 0;
  foreach ($table['header'] as $header) {
    $columns_count += (is_array($header) && isset($header['colspan']) ? $header['colspan'] : 1);
  }

  // Render rows, region by region.
  foreach ($elements['#regions'] as $region_name => $region) {
    $region_name_class = backdrop_html_class($region_name);

    // Add region rows.
    if (isset($region['title'])) {
      $table['rows'][] = [
        'class' => ['region-title', 'region-' . $region_name_class . '-title'],
        'no_striping' => TRUE,
        'data' => [
          ['data' => $region['title'], 'colspan' => $columns_count],
        ],
      ];
    }
    if (isset($region['message'])) {
      $class = (empty($region['rows_order']) ? 'region-empty' : 'region-populated');
      $table['rows'][] = [
        'class' => ['region-message', 'region-' . $region_name_class . '-message', $class],
        'no_striping' => TRUE,
        'data' => [
          ['data' => $region['message'], 'colspan' => $columns_count],
        ],
      ];
    }

    // Add form rows, in the order determined at pre-render time.
    foreach ($region['rows_order'] as $name) {
      $element = $elements[$name];

      $row = ['data' => []];
      if (isset($element['#attributes'])) {
        $row += $element['#attributes'];
      }

      // Render children as table cells.
      foreach (element_children($element) as $cell_key) {
        $child = &$element[$cell_key];
        // Do not render a cell for children of #type 'value'.
        if (!(isset($child['#type']) && $child['#type'] == 'value')) {
          $cell = ['data' => backdrop_render($child)];
          if (isset($child['#cell_attributes'])) {
            $cell += $child['#cell_attributes'];
          }
          $row['data'][] = $cell;
        }
      }
      $table['rows'][] = $row;
    }
  }

  return theme('table', $table);
}

/**
 * Returns HTML for the entity display mode table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing a Form API structure to be
 *     rendered as a table.
 *
 * @ingroup themeable
 */
function theme_field_ui_view_modes($variables) {
  $element = $variables['element'];
  $view_modes = $element['#view_modes'];
  $entity_type = $element['#entity_type'];
  $entity_bundle = $element['#bundle_label'];
  $bundle_machine_name = $element['#bundle'];
  $path = $element['#admin_path'] . '/display';

  $rows = [];

  $header = [
    ['data' => t('Display modes')],
    ['data' => t('Description'), 'class' => ['priority-low']],
    ['data' => t('Operations'), 'class' => ['operations']],
  ];

  // Add the row for the default display mode.
  $row = [];
  $row['label'] = theme('label_machine_name__node_display__' . $bundle_machine_name, [
    'label' => t('Default'),
    'machine_name' => 'default',
  ]);
  $row['description'] = t('Used for all display modes which are not customized');
  $row['operations'] = [
    'data' => [
      '#type' => 'dropbutton',
      '#links' => [
        'manage' => [
          'title' => t('Manage display'),
          'href' => "$path/default",
        ],
      ],
    ],
  ];
  $rows[] = [
    'data' => $row,
    'class' => ['default-view-mode'],
  ];

  $customized_count = 0;
  $default_count = 0;
  foreach ($view_modes as $view_mode_name => $view_mode_info) {
    $row_classes = ['view-mode--' . str_replace('_', '-', $view_mode_name)];
    $row = [];

    $row['label'] = theme('label_machine_name__node_display__' . $bundle_machine_name, [
      'label' => $view_mode_info['label'],
      'machine_name' => $view_mode_name,
    ]);

    $operations = [];
    if ($view_modes[$view_mode_name]['custom settings']) {
      $row[] = '';
      $customized_count++;
      $operations['manage'] = [
        'title' => t('Manage display'),
        'href' => "$path/$view_mode_name",
        'query' => ['destination' => $path],
      ];
      $operations['reset'] = [
        'title' => t('Reset to default'),
        'href' => "$path/{$view_mode_name}/reset",
      ];
    }
    else {
      $default_count++;
      $row_classes[] = 'uses-default';
      $row['description'] = '<em>' . t('Uses the Default display mode') . '</em>';
      if (user_access('administer view modes')) {
        $operations['enable'] = [
          'title' => t('Customize'),
          'href' => "$path/{$view_mode_name}/enable",
          'query' => [
            'token' => backdrop_get_token('view_mode_enable'),
          ],
        ];
      }
    }

    if ($view_mode_info['storage'] == 'Custom' && user_access('administer view modes')) {
      $operations['configure'] = [
        'title' => t('Configure'),
        'href' => "$path/{$view_mode_name}/configure",
      ];
      $operations['delete'] = [
        'title' => t('Delete'),
        'href' => "$path/{$view_mode_name}/delete",
      ];
    }

    $row['operations'] = [
      'data' => [
        '#type' => 'dropbutton',
        '#links' => $operations,
      ],
    ];

    // Add descriptive headers.
    if ($default_count == 1) {
      $text = t('Not customized for !bundle', ['!bundle' => $element['#bundle']]);
      $header_row = [$text, '', ''];
      $rows[] = ['data' => $header_row, 'class' => ['header']];
      $default_count++; // Ensure header only prints once.
    }
    if ($customized_count == 1) {
      $text = t('Customized for !bundle', ['!bundle' => $element['#bundle']]);
      $header_row = [$text, '', ''];
      $rows[] = ['data' => $header_row, 'class' => ['header']];
      $customized_count++; // Ensure header only prints once.
    }

    $rows[] = ['data' => $row, 'class' => $row_classes];
  }

  $output = theme('table', ['header' => $header, 'rows' => $rows, 'attributes' => ['class' => ['field-ui-view-modes']]]);

  return $output;
}
