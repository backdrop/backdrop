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
  $table = array();
  $js_settings = array();

  // Add table headers and attributes.
  foreach (array('header', 'attributes') as $key) {
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
      $table['rows'][] = array(
        'class' => array('region-title', 'region-' . $region_name_class . '-title'),
        'no_striping' => TRUE,
        'data' => array(
          array('data' => $region['title'], 'colspan' => $columns_count),
        ),
      );
    }
    if (isset($region['message'])) {
      $class = (empty($region['rows_order']) ? 'region-empty' : 'region-populated');
      $table['rows'][] = array(
        'class' => array('region-message', 'region-' . $region_name_class . '-message', $class),
        'no_striping' => TRUE,
        'data' => array(
          array('data' => $region['message'], 'colspan' => $columns_count),
        ),
      );
    }

    // Add form rows, in the order determined at pre-render time.
    foreach ($region['rows_order'] as $name) {
      $element = $elements[$name];

      $row = array('data' => array());
      if (isset($element['#attributes'])) {
        $row += $element['#attributes'];
      }

      // Render children as table cells.
      foreach (element_children($element) as $cell_key) {
        $child = &$element[$cell_key];
        // Do not render a cell for children of #type 'value'.
        if (!(isset($child['#type']) && $child['#type'] == 'value')) {
          $cell = array('data' => backdrop_render($child));
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
  $bundle_label = $element['#bundle_label'];
  $path = $element['#admin_path'] . '/display';
  $rows = array();

  $header = array(
    array('data' => t('Display modes')),
    array('data' => t('Description'), 'class' => array('priority-low')),
    array('data' => t('Operations'), 'class' => array('operations')),
  );

  $customized_count = 0;
  $default_count = 0;
  foreach ($view_modes as $view_mode_name => $view_mode_info) {
    $row_classes = array('view-mode--' . str_replace('_', '-', $view_mode_name));

    $operations = array();
    if ($view_mode_name == 'default') {
      $row_classes[] = 'default-view-mode';
      $operations['manage'] = array(
        'title' => t('Manage display'),
        'href' => "$path/default",
      );
    }
    elseif ($view_mode_info['custom settings']) {
      $customized_count++;
      $row_classes[] = 'customized-view-mode';
      $operations['manage'] = array(
        'title' => t('Manage display'),
        'href' => "$path/$view_mode_name",
        'query' => array('destination' => $path),
      );
      $operations['reset'] = array(
        'title' => t('Reset to default'),
        'href' => "$path/{$view_mode_name}/reset",
      );
    }
    else {
      $default_count++;
      $row_classes[] = 'uses-default';
      if (user_access('administer view modes')) {
        $operations['enable'] = array(
          'title' => t('Customize'),
          'href' => "$path/{$view_mode_name}/enable",
          'query' => array(
            'token' => backdrop_get_token('view_mode_enable'),
          ),
        );
      }
    }

    if ($view_mode_info['storage'] == 'Custom' && user_access('administer view modes')) {
      $operations['configure'] = array(
        'title' => t('Configure'),
        'href' => "$path/{$view_mode_name}/configure",
      );
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => "$path/{$view_mode_name}/delete",
      );
    }

    $bundle_name = field_extract_bundle($entity_type, $element['#bundle']);
    $row['label'] = theme('label_machine_name__node_display__' . $bundle_name, array(
      'label' => $view_mode_info['label'],
      'machine_name' => $view_mode_name,
    ));
    $row['description'] = '<div class="description">' . $view_mode_info['status'] . '</div>';
    $row['operations'] = array(
      'data' => array(
        '#type' => 'dropbutton',
        '#links' => $operations,
      ),
    );

    // Add descriptive headers.
    if ($default_count == 1) {
      $text = t('Not customized for !bundle', array('!bundle' => $bundle_label));
      $header_row = array($text, '', '');
      $rows[] = array('data' => $header_row, 'class' => array('header'));
      // Ensure header is printed only once.
      $default_count++;
    }
    if ($customized_count == 1) {
      $text = t('Customized for !bundle', array('!bundle' => $bundle_label));
      $header_row = array($text, '', '');
      $rows[] = array('data' => $header_row, 'class' => array('header'));
      // Ensure header is printed only once.
      $customized_count++;
    }

    $rows[] = array('data' => $row, 'class' => $row_classes);
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('field-ui-view-modes'))));

  return $output;
}
