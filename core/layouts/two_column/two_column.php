<?php
/**
 * @file
 * PHP functions for the 2 column layout.
 */

/**
 * Process variables for the 2 column layout.
 */
function template_preprocess_layout__two_column(&$variables) {
  if (!$variables['content']['sidebar']) {
    $variables['classes'][] = 'layout-no-sidebar';
  }
}
