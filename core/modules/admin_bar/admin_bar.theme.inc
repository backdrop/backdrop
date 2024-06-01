<?php
/**
 * @file
 * Theme functions for the Admin Bar module.
 */

/**
 * Render a themed list of links.
 *
 * @param $variables
 *   - elements: A renderable array of links using the following keys:
 *     - #attributes: Optional array of attributes for the list item, processed
 *       via backdrop_attributes().
 *     - #title: Title of the link, passed to l().
 *     - #href: Optional path of the link, passed to l(). When omitted, the
 *       element's '#title' is rendered without link.
 *     - #description: Optional alternative text for the link, passed to l().
 *     - #options: Optional alternative text for the link, passed to l().
 *     The array key of each child element itself is passed as path for l().
 */
function theme_admin_bar_links($variables) {
  $destination = &backdrop_static('admin_bar_destination');
  $elements = $variables['elements'];

  if (!isset($destination)) {
    $destination = backdrop_get_destination();
    $destination = $destination['destination'];
  }

  // The majority of items in the menu are sorted already, but since modules
  // may add or change arbitrary items anywhere, there is no way around sorting
  // everything again. element_sort() is not sufficient here, as it
  // intentionally retains the order of elements having the same #weight,
  // whereas menu links are supposed to be ordered by #weight and #title.
  backdrop_sort($elements, array('#weight' => SORT_NUMERIC, '#title' => SORT_STRING));
  $elements['#sorted'] = TRUE;

  $output = '';
  foreach (element_children($elements) as $path) {
    // Early-return nothing if user does not have access.
    if (isset($elements[$path]['#access']) && !$elements[$path]['#access']) {
      continue;
    }
    $elements[$path] += array(
      '#attributes' => array(),
      '#options' => array(),
      '#title' => NULL,
    );

    // Increment the level for children items.
    $elements[$path]['#level'] = $elements['#level'] + 1;

    // Render children to determine whether this link is expandable.
    if (isset($elements[$path]['#type']) || isset($elements[$path]['#theme']) || isset($elements[$path]['#pre_render'])) {
      $elements[$path]['#children'] = backdrop_render($elements[$path]);
    }
    else {
      $elements[$path]['#children'] = theme('admin_bar_links', array('elements' => $elements[$path]));
      if (!empty($elements[$path]['#children'])) {
        $elements[$path]['#attributes']['class'][] = 'expandable';
      }
      if (isset($elements[$path]['#attributes']['class'])) {
        // Only add additional classes to top-level links.
        if (substr_count($path, '/') <= 1 ) {
          $class = backdrop_clean_css_identifier($path);
          $elements[$path]['#options']['attributes']['class'][] = $class;
        }
      }
    }

    // Prepare an icon string if any for this menu item. Only allow icons on
    // the first two tiers of items.
    $icon = '';
    if ($elements[$path]['#level'] < 2) {
      if (isset($elements[$path]['#options']['icon'])) {
        // If 'icon' is set to an empty string or FALSE, do not add an icon.
        if ($elements[$path]['#options']['icon']) {
          $icon = icon($elements[$path]['#options']['icon'], array('immutable' => TRUE));
        }
      }
      else {
        $arrow_direction = $GLOBALS['language']->direction ? 'left' : 'right';
        $icon = icon('caret-circle-' . $arrow_direction . '-fill', array('immutable' => TRUE));
      }
    }

    // Prepare the title as well.
    if (!empty($elements[$path]['#options']['html'])) {
      $title = $elements[$path]['#title'];
    }
    else {
      $title = check_plain($elements[$path]['#title']);
    }

    $link_text = '<span class="admin-bar-link-icon">' . $icon . '</span>';
    $link_text .= '<span class="admin-bar-link-text">' . $title . '</span>';

    $link = '';
    // Handle menu links.
    if (isset($elements[$path]['#href'])) {
      // Strip destination query string from href attribute and apply a CSS class
      // for our JavaScript behavior instead.
      if (isset($elements[$path]['#options']['query']['destination']) && $elements[$path]['#options']['query']['destination'] == $destination) {
        unset($elements[$path]['#options']['query']['destination']);
        $elements[$path]['#options']['attributes']['class'][] = 'admin-bar-destination';
      }

      // Append the icon to the sanitized title.
      $options = $elements[$path]['#options'];
      $options['html'] = TRUE;
      $link = l($link_text, $elements[$path]['#href'], $options);
    }
    // Handle plain text items, but do not interfere with menu additions.
    elseif (!isset($elements[$path]['#type']) && isset($elements[$path]['#title'])) {
      $attributes = '';
      if (isset($elements[$path]['#options']['attributes'])) {
        $attributes = backdrop_attributes($elements[$path]['#options']['attributes']);
      }
      $link = '<span' . $attributes . '>' . $link_text . '</span>';
    }

    $output .= '<li' . backdrop_attributes($elements[$path]['#attributes']) . '>';
    $output .= $link . $elements[$path]['#children'];
    $output .= '</li>';
  }
  if ($output) {
    $elements['#wrapper_attributes']['class'][] = 'dropdown';
    $attributes = backdrop_attributes($elements['#wrapper_attributes']);
    $output = '<ul' . $attributes . '>' . $output . '</ul>';
  }
  return $output;
}
