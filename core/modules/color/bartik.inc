<?php
/**
 * @file
 * Contains support for legacy Bartik Blue Lagoon colors.
 */

/**
 * Alters the theme settings form for Bartik legacy colors.
 *
 * @see color_form_system_theme_settings_alter().
 */
function bartik_form_system_theme_settings_alter(&$form, &$form_state) {
  // Add a checkbox to indicate legacy colors.
  $form['legacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Using legacy Bartik color palette'),
    '#default_value' => config_get('bartik.settings', 'legacy'),
    '#weight' => -100,
  );

  // Add form states based on legacy.
  $form['color']['scheme']['#default_value'] = '';
  $form['color']['scheme']['#states'] = array('disabled' => array(':input[name="legacy"]' => array("checked" => TRUE)));

  // Set colors back to legacy so prevuew is honest.
  $form['color']['palette']['top']['#default_value'] = '#0779bf';
  $form['color']['palette']['bottom']['#default_value'] = '#48a9e4';
  $form['color']['palette']['sidebar']['#default_value'] = '#f6f6f2';
  $form['color']['palette']['sidebarborders']['#default_value'] = '#f9f9f9';
  $form['color']['palette']['footer']['#default_value'] = '#292929';
  $form['color']['palette']['link']['#default_value'] = '#0071B3';
  $form['color']['palette']['menu']['#default_value'] = '#cccccc';
  $form['color']['palette']['activemenu']['#default_value'] = '#fefeff';

  // Disable colors selection tools.
  foreach ($form['color']['palette'] as $key => $color) {
    if ($key != '#tree') {
      $form['color']['palette'][$key]['#states'] = array('disabled' => array(':input[name="legacy"]' => array("checked" => TRUE)));
    }
  }

  // Lock menu tabs style.
  $form['tabs_wrapper']['main_menu_tabs']['#default_value'] = 'rounded-tabs';
  $form['tabs_wrapper']['main_menu_tabs']['#states'] = array('disabled' => array(':input[name="legacy"]' => array("checked" => TRUE)));

  // Add a new submit handler.
  array_unshift($form['#submit'], 'bartik_form_system_theme_settings_alter_submit');
}

/**
 * Submit handler for theme settings form.
 */
function bartik_form_system_theme_settings_alter_submit($form, &$form_state) {
  // Don't save any settings if the checkbox is checked.
  if ($form_state['values']['legacy'] === 1) {
    unset($form_state['values']['scheme']);
    unset($form_state['values']['palette']);
    $form_state['values']['color'] = FALSE;
    $form_state['values']['main_menu_tabs'] = NULL;
    return;
  }
  else {
    $legacy_palette = array(
      'top' => '#0779bf',
      'bottom' => '#48a9e4',
      'bg' => '#ffffff',
      'sidebar' => '#f6f6f2',
      'sidebarborders' => '#f9f9f9',
      'footer' => '#292929',
      'titleslogan' => '#fffeff',
      'text' => '#3b3b3b',
      'link' => '#0071B3',
      'menu' => '#cccccc',
      'activemenu' => '#fefeff',
    );
    // Don't save any settings if the colors are still legacy.
    if (implode(',', $form_state['values']['palette']) == implode(',', $legacy_palette)) {
      // Set legacy flag back to TRUE.
      $form_state['values']['legacy'] = TRUE;
      // Don't save settings.
      unset($form_state['values']['scheme']);
      unset($form_state['values']['palette']);
      $form_state['values']['color'] = FALSE;
      $form_state['values']['main_menu_tabs'] = NULL;
      return;
    }
  }
}
