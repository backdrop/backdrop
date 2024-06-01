<?php
/**
 * @file
 * Theme functions for the Options module.
 */

/**
 * Returns HTML for the label for the empty value for options that are not required.
 *
 * The default theme will display N/A for a radio list and '- None -' for a select.
 *
 * @param $variables
 *   An associative array containing:
 *   - instance: An array representing the widget requesting the options.
 *
 * @ingroup themeable
 */
function theme_options_none($variables) {
  $instance = $variables['instance'];
  $option = $variables['option'];

  $output = '';
  switch ($instance['widget']['type']) {
    case 'options_buttons':
      $output = t('N/A');
      break;

    case 'options_select':
      $output = ($option == 'option_none' ? t('- None -') : t('- Select a value -'));
      break;
  }

  return $output;
}

/**
 * Theme an options element.
 */
function theme_options($variables) {
  $element = $variables['element'];

  element_set_attributes($element, array('id'));
  _form_set_class($element, array('form-options'));

  $classes = &$element['#attributes']['class'];
  $classes[] = 'options-key-type-'. $element['#key_type'];

  if ($element['#key_type_toggled']) {
    $classes[] = 'options-key-custom';
  }

  if (isset($element['#optgroups']) && $element['#optgroups']) {
    $classes[] = 'options-optgroups';
  }

  if (isset($element['#multiple']) && $element['#multiple']) {
    $classes[] = 'options-multiple';
  }

  // Replace the error class from wrapper div, which doesn't display well with
  // complex elements like Options.
  if ($key = array_search('error', $classes, TRUE)) {
    $classes[$key] = 'options-error';
  }

  $options = '';
  $options .= backdrop_render($element['options_field']);
  if (isset($element['default_value_field'])) {
    $options .= backdrop_render($element['default_value_field']);
  }
  if (isset($element['default_value_pattern'])) {
    $options .= backdrop_render($element['default_value_pattern']);
  }

  $settings = '';
  if (isset($element['custom_keys'])) {
    $settings .= backdrop_render($element['custom_keys']);
  }
  if (isset($element['multiple'])) {
    $settings .= backdrop_render($element['multiple']);
  }
  if (isset($element['option_settings'])) {
    $settings .= backdrop_render($element['option_settings']);
  }

  $output = '';
  $output .= '<div' . backdrop_attributes($element['#attributes']) . '>';
  $output .= theme('container', array('element' => array(
    '#title' => t('Options'),
    '#collapsible' => FALSE,
    '#children' => $options,
    '#attributes' => array('class' => array('options')),
  )));

  if (!empty($settings)) {
    $output .= theme('fieldset', array('element' => array(
      '#title' => t('Option settings'),
      '#collapsible' => FALSE,
      '#children' => $settings,
      '#attributes' => array('class' => array('option-settings')),
    )));
  }
  $output .= '</div>';

  return $output;
}
