<?php
/**
 * @file
 * PHP functions for the 3/3/4 column layout.
 */

/**
 * Process variables for the 3/3/4 column layout.
 */
function template_preprocess_layout__three_three_four_column(&$variables) {
  if ($variables['content']['sidebar_first'] && $variables['content']['sidebar_second']) {
    $variables['classes_array'][] = 'layout-two-sidebars';
  }
  elseif ($variables['content']['sidebar_first'] || $variables['content']['sidebar_second']) {
    $variables['classes_array'][] = 'layout-one-sidebar';
    if ($variables['content']['sidebar_first']) {
      $variables['classes_array'][] = 'layout-sidebar-first';
    }
    else {
      $variables['classes_array'][] = 'layout-sidebar-second';
    }
  }
  else {
    $variables['classes_array'][] = 'layout-no-sidebars';
  }
}
