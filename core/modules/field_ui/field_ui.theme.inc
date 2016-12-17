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
 * Returns HTML for the entity view mode table.
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
  $path = current_path();

  $header = array(
    array('data' => t('View modes')),
    array('data' => t('Machine name'), 'class' => array('priority-low')),
    array('data' => t('Status'), 'class' => array('priority-medium')),
    array('data' => t('Operations'), 'class' => array('operations')),
  );

  // Add the row for the default view mode.
  $row = array();
  $row['label'] = t('Default');
  $row['machine'] = t('default');
  $row['storage'] = t('N/A');
  $row['operations'] = array(
    'data' => array(
      '#type' => 'dropbutton',
      '#links' => array(
        'manage' => array(
          'title' => t('Manage display'),
          'href' => "$path/default",
        ),
      ),
    ),
  );
  $first_row = array(
    'class' => array('default-view-mode'),
    'data' => $row,
  );

  // Add the row for adding a new view mode. @todo, replace with action link?
  $last_row = array();
  if (isset($element['new'])) {
    $row = array();
    $row['label'] = backdrop_render($element['new']['label']);
    $row['machine'] = backdrop_render($element['new']['machine_name']);
    $row['status'] = array('data' => backdrop_render($element['new']['submit']), 'colspan' => 2);
    unset($element['new']);

    $last_row = array('data' => $row, 'class' => array('add-new-view-mode'));
  }

  $rows = array($first_row);
  foreach ($view_modes as $view_mode_name => $veiw_mode_info) {
    $class = '';
    $row = array();

    $row['label'] = t($veiw_mode_info['label']);
    $row['machine'] = t($view_mode_name);
    $row['status'] = t($veiw_mode_info['status']);

    $operations = array();
    if ($view_modes[$view_mode_name]['custom settings']) {
      $operations['manage'] = array(
        'title' => t('Manage display'),
        'href' => "$path/$view_mode_name",
        'query' => array('destination' => $path),
      );
      $operations['default'] = array(
        'title' => t('Reset to default'),
        'href' => "$path/view-modes/{$view_mode_name}/default",
      );
    }
    else {
      $class = 'disabled';
      $operations['customize'] = array(
        'title' => t('Customize'),
        'href' => "$path/view-modes/{$view_mode_name}/customize",
      );
    }

    if ($veiw_mode_info['storage'] == 'Custom') {
      $operations['configure'] = array(
        'title' => t('Configure'),
        'href' => "$path/view-modes/{$view_mode_name}/configure",
        'query' => array('destination' => $path),
      );
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => "$path/view-modes/{$view_mode_name}/delete",
        'query' => array('destination' => $path),
      );
    }

    $row['operations'] = array(
      'data' => array(
        '#type' => 'dropbutton',
        '#links' => $operations,
      ),
    );

    $rows[] = array('data' => $row, 'class' => array($class));
  }
  $rows[] = $last_row;

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('field-ui-view-modes'))));
  $output .= backdrop_render_children($form);

  return $output;
}
