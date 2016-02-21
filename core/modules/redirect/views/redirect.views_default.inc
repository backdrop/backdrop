<?php

/**
 * Implements hook_views_default_views().
 */
function redirect_views_default_views() {
  $views = array();
  $files = file_scan_directory(backdrop_get_path('module', 'redirect') . '/views', '/\.view$/');
  foreach ($files as $path => $file) {
    require $path;
    if (isset($view)) {
      $views[$view->name] = $view;
    }
  }
  return $views;
}
