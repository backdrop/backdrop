<?php
/**
 * @file
 * Theme functions for field groups.
 */

/**
 * Returns HTML for an element's children fieldsets as horizontal tabs.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties and children of
 *     the fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_horizontal_tabs($variables) {
  $element = $variables['element'];
  // Add required JavaScript and Stylesheet.
  backdrop_add_library('field_group', 'horizontal-tabs');

  if (!empty($element['#title'])) {
    $label = $element['#title'];
  }
  else {
    $label = t('Horizontal Tabs');
  }

  $output = '<h2 class="element-invisible">' . $label . '</h2>';

  $output .= '<div class="horizontal-tabs-panes">';
  $output .= $element['#children'];
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for an element's children fieldsets as multipage.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties and children of
 *     the fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_multipage($variables) {
  $element = $variables['element'];
  // Add required JavaScript and Stylesheet.
  $element['#attached']['library'][] = array('field_group', 'multipage');

  if (!empty($element['#title'])) {
    $label = $element['#title'];
  }
  else {
    $label = t('Multipage');
  }

  $output = '<h2 class="element-invisible">' . $label . '</h2>';

  $output .= '<div class="multipage-panes">';
  $output .= $element['#children'];
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for multipage pane.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties and children of
 *     the fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_multipage_pane($variables) {
  $element = $variables['element'];
  $group = $variables['element']['#group_object'];
  $parent_group = $variables['element']['#parent_group_object'];
  $instance_settings = $parent_group->format_settings['instance_settings'];

  static $multipages;
  if (!isset($multipages[$group->parent_name])) {
    $multipages = array($group->parent_name => 0);
  }
  $multipages[$parent_group->group_name]++;

  if (isset($instance_settings['page_header'])) {
    $page_header = $instance_settings['page_header'];
  }
  else {
    $page_header = 3;
  }

  // Create a page title from the label.
  switch ($page_header) {
    case 1:
      $title = $element['#title'];
      break;
    case 2:
      $count = $multipages[$parent_group->group_name];
      $total = count($parent_group->children);
      $replacements = array(
        '%count' => $count,
        '%total' => $total,
      );
      $title = t('Step %count of %total', $replacements);
      break;
    case 3:
      $count = $multipages[$parent_group->group_name];
      $total = count($parent_group->children);
      $label = $element['#title'];
      $replacements = array(
        '%count' => $count,
        '%total' => $total,
        '!label' => $label,
      );
      $title = t('Step %count of %total !label', $replacements);
      break;
    case 0:
    default:
      $title = '';
      break;
  }

  element_set_attributes($element, array('id'));
  _form_set_class($element, array('form-wrapper'));

  $output = '<div' . backdrop_attributes($element['#attributes']) . '>';
  if (!empty($element['#title'])) {
    // Always wrap fieldset legends in a SPAN for CSS positioning.
    $span = '<span>' . $title . '</span>';
    $output .= '<h2 class="multipage-pane-title">' . $span . '</h2>';
  }
  $output .= '<div class="fieldset-wrapper multipage-pane-wrapper">';
  if (!empty($element['#description'])) {
    $output .= '<div class="fieldset-description">';
    $output .= $element['#description'];
    $output .= '</div>';
  }
  $output .= $element['#children'];
  if (isset($element['#value'])) {
    $output .= $element['#value'];
  }

  // Add a page counter if needed.
  $page_counter_format = 1;
  if (isset($instance_settings['page_counter'])) {
    $page_counter_format = $instance_settings['page_counter'];
  }
  $multipage_element['#page_counter_rendered'] = '';

  if ($page_counter_format == 1) { // t('Format 1 / 10')
    $count = $multipages[$parent_group->group_name];
    $total = count($parent_group->children;
    $message = '<span class="multipage-counter">%count / %total</span>';
    $output .= t($message, array('%count' => $count, '%total' => $total));
  }
  elseif ($page_counter_format == 2) { // t('The count number only')
    $count = $multipages[$parent_group->group_name];
    $message = '<span class="multipage-counter">%count</span>';
    $output .=  t($message, array('%count' => $count));
  }

  $output .= '</div>';
  $output .= "</div>\n";

  return $output;

}