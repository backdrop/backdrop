<?php
/**
 * @file
 * Basis preprocess functions and theme function overrides.
 */

/**
 * Prepares variables for page templates.
 *
 * Many themes provide their own copy of page.tpl.php. The default is located at
 * "core/modules/system/templates/page.tpl.php". The full list of variables is 
 * documented in that file.
 *
 * @param $variables
 *   An array containing (but not limited to) the following:
 *   - css: The array of CSS files to be used for this page.
 *   - page: The rendered page content, as output from Layout module.
 *   - page_bottom: Final closing markup from any modules that have altered the
 *   page. This variable should always be output last, after all other dynamic
 *   content.
 * 
 * @see page.tpl.php
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
 * Prepares variables for maintenance page templates.
 *
 * Many themes provide their own copy of maintenance-page.tpl.php. The default is located at
 * "core/modules/system/templates/maintenance-page.tpl.php". The full list of variables is 
 * documented in that file or files that it references.
 *
 * @param $variables
 *   An array containing (but not limited to) the following:
 *   - css: The array of CSS files to be used for this page.
 *   - page: The rendered page content, as output from Layout module.
 *   - page_bottom: Final closing markup from any modules that have altered the
 *   page. This variable should always be output last, after all other dynamic
 *   content.
 * 
 * @see maintenance-page.tpl.php
 */
function basis_preprocess_maintenance_page(&$variables) {
  $css_path = backdrop_get_path('theme', 'basis') . '/css/component/maintenance.css';
  backdrop_add_css($css_path);
}

/**
 * Prepares variables for layout templates.
 *
 * This is the theme specific layout method for single column layouts. 
 * The default is located at "core/modules/layout/templates/layout.tpl.php". 
 * The full list of variables is documented in that file.
 *
 * @param $variables
 *   An array containing (but not limited to) the following:
 *   - title: The page title, for use in the actual HTML content.
 *   - classes: Array of classes to be added to the layout wrapper.
 *   - content: An array of content, each item in the array is keyed to one
 * 
 * @see layout.tpl.php
 */
function basis_preprocess_layout(&$variables) {
  if ($variables['is_front']) {
    // Add a special front-page class.
    $variables['classes'][] = 'layout-front';
    // Add a special front-page template suggestion.
    $original = $variables['theme_hook_original'];
    $variables['theme_hook_suggestions'][] = $original . '__front';
    $variables['theme_hook_suggestion'] = $original . '__front';
  }
}

/**
 * Prepares variables for node templates.
 *
 * Some themes provide their own copy of node.tpl.php. The default is located at
 * "core/modules/node/templates/node.tpl.php". The full list of variables is 
 * documented in that file.
 *
 * @param $variables
 *   An array containing (but not limited to) the following:
 *   - title: the (sanitized) title of the node.
 *   - content: An array of node items. 
 *   - classes: Array of classes that can be used to style contextually through
 *   CSS. 
 * 
 * @see node.tpl.php
 */
function basis_preprocess_node(&$variables) {
  if ($variables['status'] == NODE_NOT_PUBLISHED) {
    $name = node_type_get_name($variables['type']);
    $variables['title_suffix']['unpublished_indicator'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="unpublished-indicator">' . t('This @type is unpublished.', array('@type' => $name)) . '</div>',
    );
  }
}

/**
 * Prepares variables for header templates.
 *
 * Some themes provide their own copy of header.tpl.php. The default is located at
 * "core/modules/system/templates/header.tpl.php". The full list of variables is 
 * documented in that file.
 *
 * @param $variables
 *   An array containing (but not limited to) the following:
 *   - front_page: The URL of the front page. Use this instead of $base_path, when
 *   linking to the front page. This includes the language domain or prefix.
 *   - site_name: The name of the site, empty when display has been disabled.
 * 
 * @see header.tpl.php
 */
function basis_preprocess_header(&$variables) {
  $logo = $variables['logo'];
  $logo_attributes = $variables['logo_attributes'];

  // Add classes and height/width to logo.
  if ($logo) {
    $logo_wrapper_classes = array();
    $logo_wrapper_classes[] = 'header-logo-wrapper';
    if ($logo_attributes['width'] <= $logo_attributes['height']) {
      $logo_wrapper_classes[] = 'header-logo-tall';
    }

    $variables['logo_wrapper_classes'] = $logo_wrapper_classes;
  }
}

/**
 * Overrides theme_breadcrumb(). Removes &raquo; from markup.
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
