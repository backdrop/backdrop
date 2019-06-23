<?php
/**
 * @file
 * Theme functions for the Menu module.
 */

/**
 * Returns HTML for the menu name and its machine name.
 *
 * @param $variables
 *   An associative array containing:
 *   - 'title': The menu's human-readable title.
 *   - 'menu_name': The menu's machine name.
 *   - 'description': The menu's description.
 *
 * @see menu_overview_page()
 * @ingroup themeable
 */
function theme_menu_name($variables) {
  $menu = $variables['menu'];

  $output = '';
  $output .= '<div class="menu-label">' . check_plain($menu['title']) . '</div>';
  $output .= '<div class="menu-machine-name">(<span>' . check_plain($menu['menu_name']) . '</span>)</div>';
  return $output;
}

/**
 * Returns HTML for the menu description.
 *
 * @param $variables
 *   An associative array containing:
 *   - 'title': The menu's human-readable title.
 *   - 'menu_name': The menu's machine name.
 *   - 'description': The menu's description.
 *
 * @see menu_overview_page()
 * @ingroup themeable
 */
function theme_menu_description($variables) {
  $menu = $variables['menu'];

  $output = '';
  if (!empty($menu['description'])) {
    $output = '<div class="menu-description">' . filter_xss_admin($menu['description']) . '</div>';
  }
  return $output;
}

/**
 * Returns HTML for the menu overview form into a table.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_menu_overview_form($variables) {
  $form = $variables['form'];

  $output = '';

  $form['langcode']['#field_suffix'] = backdrop_render($form['langcode_filter']);
  $output .= backdrop_render($form['langcode']);

  backdrop_add_tabledrag('menu-overview', 'match', 'parent', 'menu-plid', 'menu-plid', 'menu-mlid', TRUE, MENU_MAX_DEPTH - 1);
  backdrop_add_tabledrag('menu-overview', 'order', 'sibling', 'menu-weight');

  $header = array();
  $header[] = t('Menu link');
  if (module_exists('language')) {
    $header[] = t('Language');
  }
  $header[] = array('data' => t('Enabled'), 'class' => array('checkbox'));
  $header[] = t('Weight');
  $header[] = t('Operations');

  $rows = array();
  foreach (element_children($form) as $mlid) {
    if (isset($form[$mlid]['hidden'])) {
      $element = &$form[$mlid];

      // Add special classes to be used for tabledrag.js.
      $element['plid']['#attributes']['class'] = array('menu-plid');
      $element['mlid']['#attributes']['class'] = array('menu-mlid');
      $element['weight']['#attributes']['class'] = array('menu-weight');

      // Change the parent field to a hidden. This allows any value but hides the field.
      $element['plid']['#type'] = 'hidden';

      $row = array();
      $row[] = theme('indentation', array('size' => $element['#item']['depth'] - 1)) . backdrop_render($element['title']);
      if (module_exists('language')) {
        $row[] = backdrop_render($element['langcode']);
      }
      $row[] = array('data' => backdrop_render($element['hidden']), 'class' => array('checkbox', 'menu-enabled'));
      $row[] = backdrop_render($element['weight']) . backdrop_render($element['plid']) . backdrop_render($element['mlid']);
      $row[] = backdrop_render($element['operations']);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'][] = 'draggable';
      $rows[] = $row;
    }
  }

  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => $form['#empty_text'],
        'colspan' => count($header),
      ),
    );
  }
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'menu-overview'),
  ));
  $output .= backdrop_render_children($form);
  return $output;
}

