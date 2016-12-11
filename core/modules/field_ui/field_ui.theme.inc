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
function theme_field_ui_modes($variables) {
  $element = $variables['element'];
  $view_modes = $element['#view_modes'];
  $entity_type = $element['#entity_type'];
  $view_mode_settings = $element['#view_mode_settings'];
  $path = current_path();

  $custom_view_modes = config_get('entity.view_modes', 'view_modes');

  $header = array(
    t('Enabled'),
    t('Machine name'),
    t('Storage state'),
    array('data' => t('Operations'), 'class' => array('operations')),
  );

  $default_checkbox = array(
    '#type' => 'checkbox',
    '#title' => t('Default'),
    '#default_value' => TRUE,
    '#attributes' => array('disabled' => 'true', 'checked' => 'checked'),
  );
  $default_operations = array(
    'manage' => array(
      'title' => t('Manage display'),
      'href' => "$path/default",
    ),
  );
  $first_row = array();
  $row = array();
  $row['enabled'] = backdrop_render($default_checkbox);
  $row['machine'] = t('default');
  $row['storage'] = t('N/A');
  $row['operations'] = array(
    'data' => array(
      '#type' => 'dropbutton',
      '#links' => $default_operations,
    ),
  );
  $first_row = array('data' => $row, 'class' => array('default-view-mode'));

  $last_row = array();
  if (isset($element['new'])) {
    $row = array();
    $row['enabled'] = backdrop_render($element['new']['label']);
    $row['machine'] = backdrop_render($element['new']['machine_name']);
    $row['storage'] = array('data' => t('New'), 'colspan' => 2);
    unset($element['new']);

    $last_row = array('data' => $row, 'class' => array('add-new-view-mode'));
  }

  $storage_states = array();

  $rows = array($first_row);
  foreach (element_children($element) as $view_mode_name) {
    $row = array();

    $row['enabled'] = backdrop_render($element[$view_mode_name]);
    $row['machine'] = t($view_mode_name);
    $row['storage'] = !empty($custom_view_modes[$entity_type][$view_mode_name]) ? t('Custom') : t('Default (module-provided)');

    $operations = array();
    if ($view_mode_settings[$view_mode_name]['custom_settings']) {
      $operations['manage'] = array(
        'title' => t('Manage display'),
        'href' => "$path/$view_mode_name",
      );
    }
    if (isset($custom_view_modes[$entity_type][$view_mode_name])) {
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

    $rows[] = $row;
  }
  $rows[] = $last_row;

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('view-modes'))));
  $output .= backdrop_render_children($form);

  return $output;
}
