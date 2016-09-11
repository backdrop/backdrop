<?php

/**
 * Implements hook_preprocess_page()'
 * To add a class 'page-node-N' to each page.
 */
function basis_preprocess_page(&$variables) {
  $node = menu_get_object();

  // Add normalize.css from core as high up as possible in cascade
  backdrop_add_css('core/misc/normalize.css', array(
    'every_page' => true,
    'group' => CSS_SYSTEM,
    'weight' => -1000,
  ));

  // Add the OpenSans font from core on every page of the site.
  backdrop_add_library('system', 'opensans', TRUE);

  if ($node) {
    $variables['classes'][] = 'page-node-' . $node->nid;
  }


  // To add a class 'view-name-N' to each page.
  $view = views_get_page_view();
  if ($view) {
    $variables['classes'][] = 'view-name-' . $view->name;
  }
}

/**
 * Implements template_preprocess_page().
 */
function basis_preprocess_layout(&$variables) {
  if ($variables['is_front']) {
    $variables['classes'][] = 'layout-front';
  }
}

/**
 * Implements hook_preprocess_menu_local_tasks().
 */
function basis_preprocess_menu_local_tasks(&$variables) {
  $theme_path = backdrop_get_path('theme', 'basis');
  backdrop_add_css($theme_path . '/css/component/admin-tabs.css');
}

/**
 * Implements hook_preprocess_fieldset().
 */
function basis_preprocess_fieldset(&$variables) {
  if (isset($variables['element']['#collapsible']) && $variables['element']['#collapsible'] == true) {
    $seven_theme_path = backdrop_get_path('theme', 'seven');
    backdrop_add_js('core/misc/collapse.js');
  }
}

/**
 * Implements hook_preprocess_vertical_tabs().
 */
function basis_preprocess_vertical_tabs(&$variables) {
  $theme_path = backdrop_get_path('theme', 'basis');
  backdrop_add_css($theme_path . '/css/component/vertical-tabs.css');
}

/**
 * Implements template_preprocess_block().
 */
function basis_preprocess_block(&$variables) {
  $theme_path = backdrop_get_path('theme', 'basis');

  // Add component CSS if there's a hero on the page
  if (is_a($variables['block'], 'BlockHero')) {
    backdrop_add_css($theme_path . '/css/component/hero.css');
  }
}

/**
 * Overrides theme_breadcrumb().
 * Removing raquo from markup
 */
function basis_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];
  $output = '';
  if (!empty($breadcrumb)) {
    $output .= '<nav role="navigation" class="breadcrumb">';
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    $output .= '<h2 class="element-invisible">' . t('You are here') . '</h2>';
    $output .= '<ol><li>' . implode('</li><li>', $breadcrumb) . '</li></ol>';
    $output .= '</nav>';
  }
  return $output;
}
