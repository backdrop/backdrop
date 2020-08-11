<?php
/**
 * @file
 * Theme functions and preprocessing for Link module output.
 */

/**
 * Formats a link field widget.
 */
function theme_link_field($variables) {
  $element = $variables['element'];
  // Prefix single value link fields with the name of the field.
  if (empty($element['#field']['multiple'])) {
    if (isset($element['url']) && !isset($element['title'])) {
      $element['url']['#title_display'] = 'invisible';
    }
  }

  $output = '';
  $output .= '<div class="link-field-subrow">';
  if (isset($element['title'])) {
    $output .= '<div class="link-field-title link-field-column">' . backdrop_render($element['title']) . '</div>';
  }
  $output .= '<div class="link-field-url' . (isset($element['title']) ? ' link-field-column' : '') . '">' . backdrop_render($element['url']) . '</div>';
  $output .= '</div>';
  if (!empty($element['attributes']['target'])) {
    $output .= '<div class="link-attributes">' . backdrop_render($element['attributes']['target']) . '</div>';
  }
  if (!empty($element['attributes']['title'])) {
    $output .= '<div class="link-attributes">' . backdrop_render($element['attributes']['title']) . '</div>';
  }
  if (!empty($element['attributes']['class'])) {
    $output .= '<div class="link-attributes">' . backdrop_render($element['attributes']['class']) . '</div>';
  }
  $output .= backdrop_render_children($element);
  return $output;
}

/**
 * Formats a link.
 */
function theme_link_formatter_link_default($variables) {
  $output = '';
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);

  if (isset($link_options['attributes']['class'])) {
    $link_options['attributes']['class'] = array($link_options['attributes']['class']);
  }
  // Display a normal link if both title and URL are available.
  if (!empty($variables['element']['title']) && !empty($variables['element']['url'])) {
    $output = l($variables['element']['title'], $variables['element']['url'], $link_options);
  }
  // If only a title, display the title.
  elseif (!empty($variables['element']['title'])) {
    $output = $link_options['html'] ? $variables['element']['title'] : check_plain($variables['element']['title']);
  }
  elseif (!empty($variables['element']['url'])) {
    $output = l($variables['element']['title'], $variables['element']['url'], $link_options);
  }
  return $output;
}

/**
 * Formats a link (or its title) as plain text.
 */
function theme_link_formatter_link_plain($variables) {
  $link_options = $variables['element'];
  if (isset($link_options['title'])) {
    unset($link_options['title']);
  }
  else {
    $variables['element']['title'] = '';
  }
  unset($link_options['url']);
  return empty($variables['element']['url']) ? check_plain($variables['element']['title']) : url($variables['element']['url'], $link_options);
}

/**
 * Formats a link as an absolute URL.
 */
function theme_link_formatter_link_absolute($variables) {
  $absolute = array('absolute' => TRUE);
  return empty($variables['element']['url']) ? '' : url($variables['element']['url'], $absolute + $variables['element']);
}

/**
 * Formats a link using the URL's domain for it's link text.
 */
function theme_link_formatter_link_domain($variables) {
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);
  $domain = parse_url($variables['element']['display_url'], PHP_URL_HOST);
  if (!empty($variables['display']['settings']['strip_www'])) {
    $domain = str_replace('www.', '', $domain);
  }
  return $variables['element']['url'] ? l($domain, $variables['element']['url'], $link_options) : '';
}

/**
 * Formats a link's title as plain text.
 */
function theme_link_formatter_link_title_plain($variables) {
  return empty($variables['element']['title']) ? '' : check_plain($variables['element']['title']);
}

/**
 * Formats a link using an alternate display URL for its link text.
 */
function theme_link_formatter_link_url($variables) {
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);
  return $variables['element']['url'] ? l($variables['element']['display_url'], $variables['element']['url'], $link_options) : '';
}

/**
 * Formats a link using "Link" as the link text.
 */
function theme_link_formatter_link_short($variables) {
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);
  return $variables['element']['url'] ? l(t('Link'), $variables['element']['url'], $link_options) : '';
}

/**
 * Formats a link using the field's label as link text.
 */
function theme_link_formatter_link_label($variables) {
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);
  return $variables['element']['url'] ? l($variables['field']['label'], $variables['element']['url'], $link_options) : '';
}

/**
 * Formats a link as separate title and URL elements.
 */
function theme_link_formatter_link_separate($variables) {
  $class = empty($variables['element']['attributes']['class']) ? '' : ' ' . $variables['element']['attributes']['class'];
  unset($variables['element']['attributes']['class']);
  $link_options = $variables['element'];
  unset($link_options['title']);
  unset($link_options['url']);
  $title = empty($variables['element']['title']) ? '' : check_plain($variables['element']['title']);

  $url_parts = _link_parse_url($variables['element']['url']);
  $output = '';
  $output .= '<div class="link-item ' . $class . '">';
  if (!empty($title)) {
    $output .= '<div class="link-title">' . $title . '</div>';
  }
  $output .= '<div class="link-url">' . l($url_parts['url'], $variables['element']['url'], $link_options) . '</div>';
  $output .= '</div>';
  return $output;
}
