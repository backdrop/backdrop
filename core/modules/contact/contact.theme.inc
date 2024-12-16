<?php
/**
 * @file
 * Theme functions for the Taxonomy module.
 */

/**
 * Returns HTML for the contact form categories administrative table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the categories form elements.
 *
 * @ingroup themeable
 */
function theme_contact_category_list_table($variables) {
  $element = $variables['element'];

  $rows = [];
  foreach (element_children($element) as $key) {
    $category = &$element[$key];

    $row = [];
    $category['weight']['#attributes']['class'] = ['contact-category-weight'];
    $row[] = backdrop_render($category['name']);
    $row[] = backdrop_render($category['recipients']);
    $row[] = backdrop_render($category['default']);
    $row[] = backdrop_render($category['weight']);
    $row[] = backdrop_render($category['operations']);

    $rows[] = [
      'data' => $row,
      'class' => ['draggable']
    ];
  }

  $header = [
    t('Category'),
    t('Recipients'),
    t('Default'),
    t('Weight'),
    t('Operations'),
  ];

  backdrop_add_tabledrag('contact', 'order', 'sibling', 'contact-category-weight');

  $output = theme('table', [
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No categories available.'),
    'attributes' => ['id' => 'contact']]
  );
  $output .= backdrop_render_children($element);

  return $output;
}
