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
 *   - element: An associative array containing the properties and children of the
 *     fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_horizontal_tabs($variables)
{
  $element = $variables['element'];
  // Add required JavaScript and Stylesheet.
  backdrop_add_library('field_group', 'horizontal-tabs');

  $output = '<h2 class="element-invisible">' . (!empty($element['#title']) ? $element['#title'] : t('Horizontal Tabs')) . '</h2>';
  $output .= '<div class="horizontal-tabs-panes">' . $element['#children'] . '</div>';

  return $output;
}

/**
 * Returns HTML for an element's children fieldsets as multipage.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: An associative array containing the properties and children of the
 *     fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_multipage($variables)
{
  $element = $variables['element'];
  // Add required JavaScript and Stylesheet.
  $element['#attached']['library'][] = array('field_group', 'multipage');

  $output = '<h2 class="element-invisible">' . (!empty($element['#title']) ? $element['#title'] : t('Multipage')) . '</h2>';

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
 *   - element: An associative array containing the properties and children of the
 *     fieldset. Properties used: #children.
 *
 * @ingroup themeable
 */
function theme_multipage_pane($variables)
{

  $element = $variables['element'];
  $group = $variables['element']['#group_object'];
  $parent_group = $variables['element']['#parent_group_object'];

  static $multipages;
  if (!isset($multipages[$group->parent_name])) {
    $multipages = array($group->parent_name => 0);
  }
  $multipages[$parent_group->group_name]++;

  // Create a page title from the label.
  $page_header = isset($parent_group->format_settings['instance_settings']['page_header']) ? $parent_group->format_settings['instance_settings']['page_header'] : 3;
  switch ($page_header) {
    case 1:
      $title = $element['#title'];
      break;
    case 2:
      $title = t('Step %count of %total', array('%count' => $multipages[$parent_group->group_name], '%total' => count($parent_group->children)));
      break;
    case 3:
      $title = t('Step %count of %total !label', array('%count' => $multipages[$parent_group->group_name], '%total' => count($parent_group->children), '!label' => $element['#title']));
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
    $output .= '<h2 class="multipage-pane-title"><span>' . $title . '</span></h2>';
  }
  $output .= '<div class="fieldset-wrapper multipage-pane-wrapper">';
  if (!empty($element['#description'])) {
    $output .= '<div class="fieldset-description">' . $element['#description'] . '</div>';
  }
  $output .= $element['#children'];
  if (isset($element['#value'])) {
    $output .= $element['#value'];
  }

  // Add a page counter if needed.
  // counter array(0 => t('No'), 1 => t('Format 1 / 10'), 2 => t('The count number only'));
  $page_counter_format = isset($parent_group->format_settings['instance_settings']['page_counter']) ? $parent_group->format_settings['instance_settings']['page_counter'] : 1;
  $multipage_element['#page_counter_rendered'] = '';
  if ($page_counter_format == 1) {
    $output .= t('<span class="multipage-counter">%count / %total</span>', array('%count' => $multipages[$parent_group->group_name], '%total' => count($parent_group->children)));
  } elseif ($page_counter_format == 2) {
    $output .=  t('<span class="multipage-counter">%count</span>', array('%count' => $multipages[$parent_group->group_name]));
  }

  $output .= '</div>';
  $output .= "</div>\n";

  return $output;
}
