<?php
/**
 * @file
 * Theme functions for the Locale module.
 */

/**
 * Returns HTML for the language negotiation configuration form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_language_negotiation_configure_form($variables) {
  $form = $variables['form'];
  $output = '';

  foreach ($form['#language_types'] as $type) {
    $rows = [];
    $info = $form['#language_types_info'][$type];
    $title = '<label>' . $form[$type]['#title'] . '</label>';
    $description = '<div class="description">' . $form[$type]['#description'] . '</div>';

    foreach (element_children($form[$type]) as $id) {
      if (array_key_exists('title', $form[$type][$id])) {
        $row = [
          'data' => [
            '<strong>' . backdrop_render($form[$type][$id]['title']) . '</strong>',
            backdrop_render($form[$type][$id]['description']),
            backdrop_render($form[$type][$id]['enabled']),
            backdrop_render($form[$type][$id]['weight']),
          ],
          'class' => ['draggable'],
        ];
        if ($form[$type]['#show_operations']) {
          $row['data'][] = backdrop_render($form[$type][$id]['operation']);
        }
        $rows[] = $row;
      }
    }

    $header = [
      ['data' => t('Detection method')],
      ['data' => t('Description')],
      ['data' => t('Enabled')],
      ['data' => t('Weight')],
    ];

    // If there is at least one operation enabled show the operation column.
    if ($form[$type]['#show_operations']) {
      $header[] = ['data' => t('Operations')];
    }

    $variables = [
      'header' => $header,
      'rows' => $rows,
      'attributes' => ['id' => "language-negotiation-providers-$type"],
    ];
    $table  = theme('table', $variables);
    $table .= backdrop_render_children($form[$type]);

    backdrop_add_tabledrag("language-negotiation-providers-$type", 'order', 'sibling', "language-provider-weight-$type");

    $output .= '<div class="form-item">' . $title . $description . $table . '</div>';
  }

  $output .= backdrop_render_children($form);
  return $output;
}

/**
 * Returns themed HTML for the locale_date_format_form() form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_locale_date_format_form($variables) {
  $form = $variables['form'];
  $header = [
    'machine_name' => t('Language'),
    'pattern' => t('Format'),
  ];

  foreach (element_children($form['locales']) as $key) {
    $row = [];
    $row[] = $form['locales'][$key]['#title'];
    unset($form['locales'][$key]['#title']);
    $row[] = ['data' => backdrop_render($form['locales'][$key])];
    $rows[] = $row;
  }

  $output = backdrop_render($form['help']);
  $output .= theme('table', ['header' => $header, 'rows' => $rows]);
  $output .= backdrop_render_children($form);

  return $output;
}

