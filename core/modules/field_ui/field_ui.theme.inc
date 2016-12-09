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


function theme_field_ui_modes($variables) {
  $element = $variables['element'];
  $view_modes = $element['#view_modes'];
  $entity_type = $element['#entity_type'];
  $custom_view_modes = config_get('entity.view_modes', 'view_modes');

  $header = array(
    t('Enabled view modes'),
    t('Storage state'),
    t('Operations'),
  );

  $rows = array();
  foreach (element_children($element) as $view_mode_name) {
    $row = array();

    $row['enabled'] = backdrop_render($element[$view_mode_name]);
    $row['type'] = !empty($custom_view_modes[$entity_type][$view_mode_name]) ? t('In configuration') : t('In code');

    $operations = array();
    if (isset($custom_view_modes[$entity_type][$view_mode_name])) {
      $operations['edit'] = array(
        'title' => t('Edit'),
        'href' => "admin/structure/view-modes/edit/{$entity_type}/{$view_mode_name}",
        'query' => array('destination' => current_path()),
      );
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => "admin/structure/view-modes/delete/{$entity_type}/{$view_mode_name}",
        'query' => array('destination' => current_path()),
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

  if (user_access('administer view modes')) {
    $rows[] = array(array(
      'data' => l(t('+ Add view mode'), 'admin/structure/view-modes/add/' . $entity_type, array('query' => array('destination' => current_path()))),
      'colspan' => '3',
    ));;
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'menu-overview')));
  $output .= backdrop_render_children($form);
  return $output;
}
