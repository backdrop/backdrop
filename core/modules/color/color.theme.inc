<?php
/**
 * @file
 * Theme functions for the Color module.
 */

/**
 * Returns HTML for a theme's color preview.
 *
 * @param $variables
 *   An associative array containing settings for the preview.
 *
 * @ingroup themeable
 */
function theme_color_preview($variables) {

  $preview_link = l(t('Open preview in a new window'), $variables['preview_url'], array(
      'attributes' => array(
        'target' => '_blank'
      )
    ));

  $output  = '<div class="color-preview">';
  $output .= $preview_link;
  $output .= '<object id="preview" data="' . $variables['preview_url'] . '"></object>';
  $output .= '</div>';

  return $output;
}
