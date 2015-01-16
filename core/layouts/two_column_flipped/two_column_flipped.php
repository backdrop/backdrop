<?php
/**
 * @file
 * PHP functions for the 2 column flipped layout.
 */

/**
 * Process variables for the 2 column flipped layout.
 */
function template_preprocess_layout__two_column_flipped(&$variables) {
  if (!$variables['content']['sidebar']) {
    $variables['classes'][] = 'layout-no-sidebar';
  }
}
