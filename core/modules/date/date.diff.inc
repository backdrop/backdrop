<?php

/**
 * @file
 * Provide diff field functions for the Date module.
 */

/**
 * Diff field callback for parsing date fields comparative values.
 */
function date_field_diff_view($items, $context) {
  $diff_items = array();
  $display = $context['display'];
  $display['settings']['format_type'] = $context['settings']['format_type'];
  $display['settings']['fromto'] = $context['settings']['fromto'];
  foreach ($items as $delta => $item) {
    $date = date_formatter_process('date_default', $context['entity_type'], $context['entity'], $context['field'], $context['instance'], $context['language'], $item, $display);
    switch ($display['settings']['fromto']) {
      case 'both':
        if ($date['value']['formatted'] != $date['value2']['formatted']) {
          $diff_items[$delta] = t('@from to @to', array(
            '@from' => $date['value']['formatted'],
            '@to' => $date['value2']['formatted'],
          ));
        }
        else {
          $diff_items[$delta] = $date['value']['formatted'];
        }
        break;

      case 'value':
      case 'value2':
        $diff_items[$delta] = $date[$display['settings']['fromto']]['formatted'];
        break;

    }
  }
  return $diff_items;
}

/**
 * Provide default field comparison options.
 */
function date_field_diff_default_options($field_type) {
  return array(
    'format_type' => 'long',
    'fromto' => 'both',
  );
}

/**
 * Provide a form for setting the field comparison options.
 */
function date_field_diff_options_form($field_type, $settings) {
  $options_form = array();

  $form['format_type'] = array(
    '#title' => t('Choose how render dates and times'),
    '#type' => 'select',
    '#options' => date_format_type_options(),
    '#default_value' => $settings['format_type'],
    '#description' => t('To add or edit options, visit <a href="@date-time-page">Date and time settings</a>.', array('@date-time-page' => url('admin/config/regional/date-time'))),
    '#weight' => 0,
  );

  $form['fromto'] = array(
    '#title' => t('Display'),
    '#type' => 'select',
    '#options' => array(
      'both' => t('Both Start and End dates'),
      'value' => t('Start date only'),
      'value2' => t('End date only'),
    ),
    '#default_value' => $settings['fromto'],
    '#weight' => 1,
  );

  return $options_form;
}
