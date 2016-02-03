<?php
/**
 * @file
 * PHP functions for the two column layout.
 */

/**
 * Process variables for the two column layout.
 */
function template_preprocess_layout__two_column(&$variables) {
  if ($variables['content']['sidebar']) {
    $variables['classes'][] = 'layout-with-sidebar';
  }
  else {
    $variables['classes'][] = 'layout-no-sidebar';
  }
}
