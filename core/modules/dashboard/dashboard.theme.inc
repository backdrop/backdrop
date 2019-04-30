<?php
/**
 * @file
 * Theme functions for Dashboard module.
 */

/**
 * Returns HTML for the content overview block.
 *
 * @param $variables
 *   An associative array containing:
 *   - overview: An array of items for the overview.
 *
 * @ingroup themeable
 */
function theme_dashboard_overview_content($variables) {
  $overview = $variables['overview'];

  $output  = '<div class="dashboard-content-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for the user overview block.
 *
 * Available variables:
 * - overview: A structured array of items containing user data, in the
 *   following format:
 *   @code
 *   $items = array(
 *     'all' => 16,
 *     'active' => 9,
 *     'blocked' => 2,
 *     'roles' => array(
 *       3 => array(
 *         'name' => 'Administrator',
 *         'count' => 4,
 *       ),
 *     ),
 *   );
 *   @endcode
 *
 * @ingroup themeable
 */
function theme_dashboard_overview_user($variables) {
  $overview = $variables['overview'];

  $output  = '<div class="dashboard-user-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for the create content Block.
 *
 * @param $variables
 *   An associative array containing:
 *   - create: An array of content creation links.
 *
 * @ingroup themeable
 */
function theme_dashboard_create($variables) {
  $create = $variables['create'];

  $output  = '<div class="dashboard-create">';
  $output .= theme('item_list', array('items' => $create));
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for the tables in dashboard blocks.
 *
 * @param $variables
 *   An associative array containing:
 *   - header: An array of items for the table header.
 *   - rows: An array of items for the table rows.
 *   - link: An administration link.
 *
 * @ingroup themeable
 */
function theme_dashboard_admin_table($variables) {
  $header = $variables['header'];
  $rows = $variables['rows'];
  $link = $variables['link'];

  $output  = '<div class="dashboard-blocks">';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('dashboard-block-manage'))));
  $output .= '  <div class="admin-page">' . $link . '</div>';
  $output .= '</div>';

  return $output;
}
