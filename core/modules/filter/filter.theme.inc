<?php
/**
 * @file
 * Theme functions for the Filter module.
 */

/**
 * Returns HTML for a text format-enabled form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element containing #children and #description.
 *
 * @ingroup themeable
 */
function theme_text_format_wrapper($variables) {
  $element = $variables['element'];
  $output = '<div class="text-format-wrapper">';
  $output .= $element['#children'];
  if (!empty($element['#description'])) {
    $output .= '<div class="description">' . $element['#description'] . '</div>';
  }
  $output .= "</div>\n";

  return $output;
}

/**
 * Returns HTML for a captioned item, usually an image.
 */
function theme_filter_caption($variables) {
  $output = '';
  $output .= '<figure' . backdrop_attributes($variables['attributes']) . '>';
  $output .= $variables['item'];
  $output .= '<figcaption>' . $variables['caption'] . '</figcaption>';
  $output .= '</figure>';
  return $output;
}

/**
 * Returns HTML for a link to the more extensive filter tips.
 *
 * @ingroup themeable
 */
function theme_filter_tips_more_info() {
  return '<p>' . l(t('More information about text editors'), 'filter/tips', array('attributes' => array('target' => '_blank'))) . '</p>';
}

/**
 * Returns HTML for guidelines for a text format.
 *
 * @param $variables
 *   An associative array containing:
 *   - format: An object representing a text format.
 *
 * @ingroup themeable
 */
function theme_filter_guidelines($variables) {
  $format = $variables['format'];
  $attributes['class'][] = 'filter-guidelines-item';
  $attributes['class'][] = 'filter-guidelines-' . $format->format;
  $output = '<div' . backdrop_attributes($attributes) . '>';
  $output .= '<h3>' . check_plain($format->name) . '</h3>';
  $output .= theme('filter_tips', array('tips' => _filter_tips($format->format, FALSE)));
  $output .= '</div>';
  return $output;
}

/**
 * Returns HTML for the text format administration overview form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_filter_admin_overview($variables) {
  $form = $variables['form'];
  $formats = filter_formats(NULL, TRUE);

  $header = array(
    array('data' => t('Name'), 'class' => array('format-name')),
    array('data' => t('Editor'), 'class' => array('format-editor', RESPONSIVE_PRIORITY_LOW)),
    array('data' => t('Roles'), 'class' => array('format-roles', RESPONSIVE_PRIORITY_LOW)),
    array('data' => t('Weight'), 'class' => array('format-weight')),
    array('data' => t('Operations'), 'class' => array('format-operations')),
  );
  // Table to hold disabled text formats.
  $disabled_header = array(
    array(
      'data' => t('Disabled text formats'),
      'colspan' => count($header),
    ),
  );

  $rows = array();
  $disabled_rows = array();

  foreach (element_children($form['formats']) as $id) {
    $form['formats'][$id]['weight']['#attributes']['class'] = array('text-format-order-weight');
    $row = array();

    $row[] = array(
      'data' => theme('filter_format_name', array('format' => $formats[$id])),
      'class' => array('format-name'),
      'data-label' => t('Name'),
    );
    $row[] = array(
      'data' => backdrop_render($form['formats'][$id]['editor']),
      // Responsive classes normally need only be specified in the header, and
      // they are then automatically inherited by the rest of the cells in the
      // same table column. We need to specifically add them here though, as the
      // disabled section has a single cell that spans across the entire header;
      // so responsive classes cannot be specified there that way.
      'class' => array('format-editor', RESPONSIVE_PRIORITY_LOW),
      'data-label' => t('Editor'),
    );
    $row[] = array(
      'data' => backdrop_render($form['formats'][$id]['roles']),
      'class' => array('format-roles', RESPONSIVE_PRIORITY_LOW),
      'data-label' => t('Roles'),
    );

    // If the format is enabled, add classes to make it draggable, and to keep
    // it in the enabled formats section; also add a cell to the table row for
    // its weight.
    if ($formats[$id]->status) {
      $row_classes = array('draggable', 'enabled');

      $row[] = array(
        'data' => backdrop_render($form['formats'][$id]['weight']),
        'class' => array('format-weight'),
        'data-label' => t('Weight'),
      );
    }
    // Otherwise, add a class to move it to the disabled formats section.
    else {
      $row_classes = array('disabled');
    }

    $row[] = array(
      'data' => backdrop_render($form['formats'][$id]['operations']),
      'class' => array('format-operations'),
      'data-label' => t('Operations'),
    );

    $rows[] = array(
      'data' => $row,
      'class' => $row_classes,
    );
  }

  // Move disabled text formats to the "Disabled text formats" table.
  foreach ($rows as $id => $row) {
    if (in_array('disabled', $row['class'])) {
      $disabled_rows[] = $row;
      unset($rows[$id]);
    }
  }

  $output = backdrop_render($form['help']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'text-format-order')));
  // Only render the disabled formats section if there are disabled formats.
  if (!empty($disabled_rows)) {
    $output .= theme('table', array('header' => $disabled_header, 'rows' => $disabled_rows, 'attributes' => array('id' => 'disabled-text-formats')));
  }

  // Do not render the 'formats' child element of the form.
  unset($form['formats']);

  $output .= backdrop_render_children($form);

  backdrop_add_tabledrag('text-format-order', 'order', 'sibling', 'text-format-order-weight');

  return $output;
}

/**
 * Display the format name and its machine name.
 */
function theme_filter_format_name($variables) {
  $format = $variables['format'];

  $output = '';
  $output .= '<div class="filter-format-name">' . check_plain($format->name) . '</div>';
  $output .= '<div class="filter-format-machine-name">(<span>' . check_plain($format->format) . '</span>)</div>';

  return $output;
}

/**
 * Returns HTML for a text format's filter order form.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_filter_admin_format_filter_order($variables) {
  $element = $variables['element'];

  // Filter order (tabledrag).
  $rows = array();
  foreach (element_children($element, TRUE) as $name) {
    $element[$name]['weight']['#attributes']['class'][] = 'filter-order-weight';
    $rows[] = array(
      'data' => array(
        backdrop_render($element[$name]['filter']),
        backdrop_render($element[$name]['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $output = backdrop_render_children($element);
  $output .= theme('table', array('rows' => $rows, 'attributes' => array('id' => 'filter-order')));
  backdrop_add_tabledrag('filter-order', 'order', 'sibling', 'filter-order-weight', NULL, NULL, TRUE);

  return $output;
}

/**
 * Returns HTML for a set of filter tips.
 *
 * @param $variables
 *   An associative array containing:
 *   - tips: An array containing descriptions and a CSS ID in the form of
 *     'module-name/filter-id' (only used when $long is TRUE) for each
 *     filter in one or more text formats. Example:
 *     @code
 *       array(
 *         'Full HTML' => array(
 *           0 => array(
 *             'tip' => 'Web page addresses and e-mail addresses turn into links automatically.',
 *             'id' => 'filter/2',
 *           ),
 *         ),
 *       );
 *     @endcode
 *   - long: (optional) Whether the passed-in filter tips contain extended
 *     explanations, i.e. intended to be output on the path 'filter/tips'
 *     (TRUE), or are in a short format, i.e. suitable to be displayed below a
 *     form element. Defaults to FALSE.
 *
 * @see _filter_tips()
 * @ingroup themeable
 */
function theme_filter_tips($variables) {
  $tips = $variables['tips'];
  $long = $variables['long'];
  $output = '';

  $multiple = count($tips) > 1;
  if ($multiple) {
    $output = '<h2>' . t('Text Formats') . '</h2>';
  }

  if (count($tips)) {
    if ($multiple) {
      $output .= '<div class="compose-tips">';
    }
    foreach ($tips as $name => $tiplist) {
      if ($multiple) {
        $output .= '<div class="filter-type filter-' . backdrop_html_class($name) . '">';
        $output .= '<h3>' . check_plain($name) . '</h3>';
      }

      if (count($tiplist) > 0) {
        $output .= '<ul class="tips">';
        foreach ($tiplist as $tip) {
          $output .= '<li' . ($long ? ' id="filter-' . str_replace("/", "-", $tip['id']) . '">' : '>') . $tip['tip'] . '</li>';
        }
        $output .= '</ul>';
      }

      if ($multiple) {
        $output .= '</div>';
      }
    }
    if ($multiple) {
      $output .= '</div>';
    }
  }

  return $output;
}

function theme_filter_format_editor_image_form($variables) {
  $form = $variables['form'];

  $library = backdrop_render($form['library_view']);
  $actions = backdrop_render($form['actions']);

  $output = '';
  $output .= '<div class="image-form-wrapper">';

  $output .= '<div class="editor-image-fields">';
  $output .= backdrop_render_children($form);
  $output .= '</div>';

  $output .= '<div class="editor-image-library">';
  $output .= $library;
  $output .= '</div>';

  $output .= '</div>';

  $output .= $actions;
  return $output;
}
