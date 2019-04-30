<?php
/**
 * @file
 * Theme functions for Dashboard module.
 */


/**
 * Theme function for content overview.
 */
function theme_dashboard_overview_content($variables) {
  $overview = $variables['overview'];
  $output  = '<div class="dashboard-content-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';
  return $output;
}
/**
 * Theme function for user overview.
 *
 * Available variables:
 *   - overview: A structured array of items containing user data, in the
 *     following format:
 *
 *     @code
 *     $items = array(
 *       'all' => 16,
 *       'active' => 9,
 *       'blocked' => 2,
 *       'roles' => array(
 *         3 => array(
 *           'name' => 'Administrator',
 *           'count' => 4,
 *         ),
 *       ),
 *     );
 *     @endcode
 *
 */
function theme_dashboard_overview_user($variables) {
  $overview = $variables['overview'];
  $output  = '<div class="dashboard-user-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';
  return $output;
}

/**
 * Theme function for create content Block.
 */
function theme_dashboard_create($variables) {
  $create = $variables['create'];
  $output  = '<div class="dashboard-create">';
  $output .= theme('item_list', array('items' => $create));
  $output .= '</div>';
  return $output;
}
/**
 * Theme function for dashboard blocks.
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
