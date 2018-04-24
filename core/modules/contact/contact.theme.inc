<?php
/**
 * @file
 * Theme functions for the Taxonomy module.
 */

/**
 * Returns HTML for the vocabulary overview form as a sortable list of vocabularies.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see taxonomy_overview_vocabularies()
 * @ingroup themeable
 */
function theme_contact_category_list($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form['categories']) as $key) {
    $category = &$form['categories'][$key];

    $row = array();
    $category['weight']['#attributes']['class'] = array('contact-category-weight');
    $row[] = backdrop_render($category['name']);
    $row[] = backdrop_render($category['recipients']);
    $row[] = backdrop_render($category['default']);
    $row[] = backdrop_render($category['weight']);
    $row[] = backdrop_render($category['operations']);

    $rows[] = array(
      'data' => $row,
      'class' => array('draggable')
    );
  }

  $header = array(
    t('Category'),
    t('Recipients'),
    t('Default'),
    t('Weight'),
    t('Operations'),
  );


  if (isset($form['actions'])) {
    backdrop_add_tabledrag('contact', 'order', 'sibling', 'contact-category-weight');
  }

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No categories available.'),
    'attributes' => array('id' => 'contact'))
  );
  $output .= backdrop_render_children($form);

  return $output;
}
