<?php
/**
 * @file
 * Basis preprocess functions and theme function overrides.
 */

/**
 * Implements hook_preprocess_page().
 *
 * @see maintenance_page.tpl.php
 */
function basis_preprocess_page(&$variables) {
  $node = menu_get_object();

  // Add the OpenSans font from core on every page of the site.
  backdrop_add_library('system', 'opensans', TRUE);

  // To add a class 'page-node-[nid]' to each page.
  if ($node) {
    $variables['classes'][] = 'page-node-' . $node->nid;
  }

  // To add a class 'view-name-[name]' to each page.
  $view = views_get_page_view();
  if ($view) {
    $variables['classes'][] = 'view-name-' . $view->name;
  }
}

/**
 * Implements template_preprocess_page().
 *
 * @see layout.tpl.php
 */
function basis_preprocess_layout(&$variables) {
  if ($variables['is_front']) {
    $variables['classes'][] = 'layout-front';
  }
}

/**
 * Implements template_preprocess_header().
 *
 * @see header.tpl.php
 */
function basis_preprocess_header(&$variables) {
  $logo = $variables['logo'];

  // Add classes and height/width to logo.
  if ($logo) {
    $logo_attributes = array();
    $logo_wrapper_classes = array();
    $logo_wrapper_classes[] = 'header-logo-wrapper';
    $logo_size = getimagesize($logo);
    if (!empty($logo_size)) {
      if ($logo_size[0] < $logo_size[1]) {
        $logo_wrapper_classes[] = 'header-logo-tall';
      }
      $logo_attributes['width'] = $logo_size[0];
      $logo_attributes['height'] = $logo_size[1];
    }

    $variables['logo_wrapper_classes'] = $logo_wrapper_classes;
    $variables['logo_attributes'] = $logo_attributes;
  }
}

/**
 * Overrides theme_breadcrumb().
 *
 * Removes &raquo; from markup.
 *
 * @see theme_breadcrumb().
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
