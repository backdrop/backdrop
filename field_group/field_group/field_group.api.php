<?php
// $Id$

/**
 * @file
 * Hooks provided by the Field group module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Implements hook_field_group_format_settings().
 *
 *
 *
 * @params Object $group The group object.
 * @return Array $form The form element for the format settings.
 */
function hook_group_field_group_format_settings($group) {
  // Add a wrapper for extra settings to use by others.
  $form = array(
    'instance_settings' => array(
      '#tree' => TRUE,
      '#weight' => 2,
    ),
  );

  $field_group_types = field_group_formatter_info();
  $mode = $group->mode == 'form' ? 'form' : 'display';
  $formatter = $field_group_types[$mode][$group->format_type];

  // Add the required formatter type selector.
  if (isset($formatter['format_types'])) {
    $form['formatter'] = array(
      '#title' => t('Fieldgroup settings'),
      '#type' => 'select',
      '#options' => drupal_map_assoc($formatter['format_types']),
      '#default_value' => isset($group->format_settings['formatter']) ? $group->format_settings['formatter'] : $formatter['default_formatter'],
      '#weight' => 1,
    );
  }
  if ($mode == 'form') {
    $form['instance_settings']['required_fields'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mark group for required fields.'),
      '#default_value' => isset($group->format_settings['instance_settings']['required_fields']) ? $group->format_settings['instance_settings']['required_fields'] : (isset($formatter['instance_settings']['required_fields']) ? $formatter['instance_settings']['required_fields'] : ''),
      '#weight' => 2,
    );
  }
  $form['instance_settings']['classes'] = array(
    '#title' => t('Extra CSS classes'),
    '#type' => 'textfield',
    '#default_value' => isset($group->format_settings['instance_settings']['classes']) ? $group->format_settings['instance_settings']['classes'] : (isset($formatter['instance_settings']['classes']) ? $formatter['instance_settings']['classes'] : ''),
    '#weight' => 3,
    '#element_validate' => array('field_group_validate_css_class'),
  );
  $form['instance_settings']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => isset($group->format_settings['instance_settings']['description']) ? $group->format_settings['instance_settings']['description'] : (isset($formatter['instance_settings']['description']) ? $formatter['instance_settings']['description'] : ''),
    '#weight' => 0,
  );

  // Add optional instance_settings.
  switch ($group->format_type) {
    case 'div':
     $form['instance_settings']['effect'] = array(
        '#title' => t('Effect'),
        '#type' => 'select',
        '#options' => array('none' => t('None'), 'blind' => t('Blind')),
        '#default_value' => isset($group->format_settings['instance_settings']['effect']) ? $group->format_settings['instance_settings']['effect'] : $formatter['instance_settings']['effect'],
        '#weight' => 2,
      );
       $form['instance_settings']['speed'] = array(
        '#title' => t('Speed'),
        '#type' => 'select',
        '#options' => array('none' => t('None'), 'slow' => t('Slow'), 'fast' => t('Fast')),
        '#default_value' => isset($group->format_settings['instance_settings']['speed']) ? $group->format_settings['instance_settings']['speed'] : $formatter['instance_settings']['speed'],
        '#weight' => 3,
      );
      break;
    case 'fieldset':
      $form['instance_settings']['classes'] = array(
        '#title' => t('Extra CSS classes'),
        '#type' => 'textfield',
        '#default_value' => isset($group->format_settings['instance_settings']['classes']) ? $group->format_settings['instance_settings']['classes'] : $formatter['instance_settings']['classes'],
        '#weight' => 3,
        '#element_validate' => array('field_group_validate_css_class'),
      );
      break;
    case 'tabs':
    case 'htabs':
    case 'accordion':
      unset($form['instance_settings']['description']);
      if (isset($form['instance_settings']['required_fields'])) {
        unset($form['instance_settings']['required_fields']);
      }
      break;
    case 'tab':
    case 'htab':
    case 'accordion-item':
    default:
  }

  return $form;
}

/**
 * Implements hook_field_group_formatter_info().
 *
 * You can use all keys you want although there are some required ones.
 *
 * structure:
 * @code
 * array(
 *   'form' => array(
 *     'fieldset' => array(
 *       // required, String with the name of the formatter type.
 *       'label' => t('Fieldset'),
 *       // optional, String description of the formatter type.
 *       'description' => t('This is field group that ...'),
 *       // required, Array of available formatter options.
 *       'format_types' => array('open', 'collapsible', 'collapsed'),
 *       // required, String with default value of the style.
        'default_formatter' => 'collapsible',
 *       // optional, Array with key => default_value pairs.
 *       'instance_settings' => array('key' => 'value'),
 *     ),
 *   ),
 *   'display' => array(
 *     'fieldset' => array(
 *       // required, String with the name of the formatter type.
 *       'label' => t('Fieldset'),
 *       // optional, String description of the formatter type.
 *       'description' => t('This is field group that ...'),
 *       // required, Array of available formatter options.
 *       'format_types' => array('open', 'collapsible', 'collapsed'),
 *       // required, String with default value of the style.
        'default_formatter' => 'collapsible',
 *       // optional, Array with key => default_value pairs.
 *       'instance_settings' => array('key' => 'value'),
 *     ),
 *   ),
 * ),
 * @endcode
 *
 * @return $formatters
 *   A collection of available formatting html controls for form
 *   and display overview type.
 *
 * @see field_group_field_group_formatter_info()
 */
function hook_field_group_formatter_info() {
  return array(
    'form' => array(
      'fieldset' => array(
        'label' => t('Fieldset'),
        'description' => t('This fieldgroup renders the inner content in a fieldset with the titel as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array('classes' => ''),
        'default_formatter' => 'collapsible',
      ),
    ),
    'display' => array(
      'div' => array(
        'label' => t('Div'),
        'description' => t('This fieldgroup renders the inner content in a simple div with the titel as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array('effect' => 'none', 'speed' => 'fast', 'classes' => ''),
        'default_formatter' => 'collapsible',
      ),
    ),
  );
}

/**
 * @} End of "addtogroup hooks".
 */
