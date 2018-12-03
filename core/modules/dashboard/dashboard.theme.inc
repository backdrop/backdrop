<?php

/**
 * @file
 *
 * Theme functions used in total control panel panes.
 *
 */

/*
 * Theme function for site overview.
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_dashboard_overview($variables) {
  $content = $variables['content'];
  $users = $variables['users'];

  $output  = '<div class="total-control-site-overview">';
  $output .= '<strong>' . t('Content') . '</strong>';
  $output .= theme('item_list', array('items' => $content));
  $output .= '<strong>' . t('Users') . '</strong>';
  $output .= theme('item_list', array('items' => $users));
  $output .= '</div>';

  return $output;
}

/*
 * Theme function for content overview.
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_dashboard_overview_content($variables) {
  $overview = $variables['overview'];

  $output  = '<div class="total-control-content-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';

  return $output;
}

/*
 * Theme function for user overview.
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_dashboard_overview_user($variables) {
  $overview = $variables['overview'];

  $output  = '<div class="total-control-user-overview">';
  $output .= theme('item_list', array('items' => $overview));
  $output .= '</div>';

  return $output;
}

/*
 * Theme function for create content pane.
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_dashboard_create($variables) {
  $create = $variables['create'];

  $output  = '<div class="total-control-create">';
  $output .= theme('item_list', array('items' => $create));
  $output .= '</div>';

  return $output;
}

/*
 * Theme function for panel pages.
 */
/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_dashboard_admin_table($variables) {
  $header = $variables['header'];
  $rows = $variables['rows'];
  $link = $variables['link'];

  $output  = '<div class="total-control-panel-panes">';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('panels-manage'))));
  $output .= '  <div class="admin-page">' . $link . '</div>';
  $output .= '</div>';

  return $output;
}

