<?php
/**
 * @file
 * User page callbacks for the Filter module.
 */

/**
 * Page callback: Displays a page with long filter tips.
 *
 * @return string
 *   An HTML-formatted string.
 *
 * @see filter_menu()
 * @see theme_filter_tips()
 */
function filter_tips_long() {
  $format_id = arg(2);
  if ($format_id) {
    $output = theme('filter_tips', array('tips' => _filter_tips($format_id, TRUE), 'long' => TRUE));
  }
  else {
    $output = theme('filter_tips', array('tips' => _filter_tips(-1, TRUE), 'long' => TRUE));
  }
  return $output;
}
