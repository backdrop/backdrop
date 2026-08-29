<?php
/**
 * @file
 * Basis preprocess functions and theme function overrides.
 */

/**
 * Implements hook_css_alter().
 */
function basis_css_alter(&$css) {
  // Remove the Basis css/component/menu-dropdown.css and add breakpoint files
  // if using a custom breakpoint.
  $config = config('menu.settings');
  $path = backdrop_get_path('theme', 'basis');
  if (isset($css[$path . '/css/component/menu-dropdown.css']) && $config->get('menu_breakpoint') == 'custom') {
    $dropdown_css = $css[$path . '/css/component/menu-dropdown.css'];
    unset($css[$path . '/css/component/menu-dropdown.css']);

    $weight = $dropdown_css['weight'];
    $weight += 0.0001;
    $css[$path . '/css/component/menu-dropdown.breakpoint.css'] = $dropdown_css;
    $css[$path . '/css/component/menu-dropdown.breakpoint.css']['weight'] = $weight;
    $css[$path . '/css/component/menu-dropdown.breakpoint.css']['data'] = $path . '/css/component/menu-dropdown.breakpoint.css';

    $weight += 0.0001;
    $css[$path . '/css/component/menu-dropdown.breakpoint-queries.css'] = $dropdown_css;
    $css[$path . '/css/component/menu-dropdown.breakpoint-queries.css']['weight'] = $weight;
    $css[$path . '/css/component/menu-dropdown.breakpoint-queries.css']['media'] = 'all and (min-width: ' . $config->get('menu_breakpoint_custom') . ')';
    $css[$path . '/css/component/menu-dropdown.breakpoint-queries.css']['data'] = $path . '/css/component/menu-dropdown.breakpoint-queries.css';
  }
}

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

  // The CSS update option can be one of the following:
  // - install: Calculate the CSS update version based on core install_version.
  // - all: Apply all CSS updates.
  // - version: Select a specific update version (and all updates prior to it).
  $update_preference = theme_get_setting('css_update');

  // Get the specified CSS update version.
  // The version must be one of the values from basis_updated_css_versions().
  // This may also be an empty string, to signify "No updates".
  $update_version = theme_get_setting('css_update_version');

  // Process supplemental CSS versions as body classes.
  $update_css_versions = basis_updated_css_versions();
  foreach ($update_css_versions as $update_css_version) {
    if ($update_preference === 'all' || version_compare($update_version, $update_css_version, '>=')) {
      $update_css_version_class = 'update-' . str_replace('.', '-', $update_css_version);
      $variables['classes'][] = $update_css_version_class;
    }
  }
}

/**
 * Returns the versions of Backdrop that contain updated CSS for Basis.
 *
 * Every time a new supplemental CSS update is added to core, the core version
 * should be added to this list. When a new version is added, the
 * "css_update_version" in basis.settings.json should match the added value.
 */
function basis_updated_css_versions() {
  return array('1.30');
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
    $output .= '<nav class="breadcrumb" aria-label="' . t('Website Orientation') . '">';
    $output .= '<ol><li>' . implode('</li><li>', $breadcrumb) . '</li></ol>';
    $output .= '</nav>';
  }
  return $output;
}
