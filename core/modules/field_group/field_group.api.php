<?php
/**
 * @file
 * Hooks provided by the Field Group module.
 *
 * Field Group is a module that will wrap fields and other field groups on forms
 * and on display modes.
 *
 * DEVELOPERS NOTES
 *
 * - Field Group uses a '#fieldgroups' property to know what field groups are to
 *   be pre_rendered and rendered by the field_group module. '#fieldgroups' is
 *   later merged with the normal #groups that can be used by any other module.
 */

/**
 * @addtogroup hooks
 * @{
 */


/**
 * JavaScript hooks.
 *
 * Backdrop.FieldGroup.Effects.processHook.execute()
 * See field_group.js for the examples for all implemented formatters.
 */


/**
 * Define the information on formatters.
 *
 * The formatters are separated by display mode type. Use "form" for all form
 * elements and "display" will be the real display modes (full, teaser, etc).
 *
 * Structure:
 * @code
 * array(
 *   'form' => array(
 *     'fieldset' => array(
 *       // required, String with the name of the formatter type.
 *       'label' => t('Fieldset'),
 *       // Optional; string description of the formatter type.
 *       'description' => t('This is field group that ...'),
 *       // Required; array of available formatter options.
 *       'format_types' => array('open', 'collapsible', 'collapsed'),
 *       // Required; string with default value of the style.
 *       'default_formatter' => 'collapsible',
 *       // Optional; array with key => default_value pairs.
 *       'instance_settings' => array('key' => 'value'),
 *     ),
 *   ),
 *   'display' => array(
 *     'fieldset' => array(
 *       // Required; string with the name of the formatter type.
 *       'label' => t('Fieldset'),
 *       // Optional; string description of the formatter type.
 *       'description' => t('This is field group that ...'),
 *       // Required; array of available formatter options.
 *       'format_types' => array('open', 'collapsible', 'collapsed'),
 *       // Required; string with default value of the style.
 *       'default_formatter' => 'collapsible',
 *       // Optional; array with key => default_value pairs.
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
        'description' => t('This field group renders the inner content in a fieldset with the title as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array(
          'required_fields' => 1,
          'description' => '',
          'classes' => '',
          'id' => '',
        ),
        'default_formatter' => 'collapsible',
      ),
    'display' => array(
      'fieldset' => array(
        'label' => t('Fieldset'),
        'description' => t('This field group renders the inner content in a fieldset with the title as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array(
          'description' => '',
          'classes' => '',
          'id' => '',
        ),
        'default_formatter' => 'collapsible',
      ),
    ),
  );
}

/**
 * Define the configuration widget for field group formatter settings.
 *
 * Each formatter can have different elements and storage.
 *
 * @param Object $group
 *   The group object.
 *
 * @return Array $form
 *   The form element for the format settings.
 */
function hook_field_group_format_settings($group) {
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
  $instance_settings = $group->format_settings['instance_settings'];

  // Add the required formatter type selector.
  if (isset($formatter['format_types'])) {
    if (isset($group->format_settings['formatter'])) {
      $formatter_default_value = $group->format_settings['formatter'];
    }
    else {
      $formatter_default_value = $formatter['default_formatter'];
    }
    $form['formatter'] = array(
      '#title' => t('Field group settings'),
      '#type' => 'select',
      '#options' => backdrop_map_assoc($formatter['format_types']),
      '#default_value' => $formatter_default_value,
      '#weight' => 1,
    );
  }

  if ($mode == 'form') {
    $required_default = '';
    if (isset($instance_settings['required_fields'])) {
      $required_default = $instance_settings['required_fields'];
    }
    elseif (isset($formatter['instance_settings']['required_fields'])) {
      $required_default = $formatter['instance_settings']['required_fields'];
    }
    $form['instance_settings']['required_fields'] = array(
      '#type' => 'checkbox',
      '#title' => t('Mark group for required fields.'),
      '#default_value' => $required_default,
      '#weight' => 2,
    );
  }

  $classes_default = '';
  if (isset($instance_settings['classes'])) {
    $classes_default = $instance_settings['classes'];
  }
  elseif (isset($formatter['instance_settings']['classes'])) {
    $classes_default = $formatter['instance_settings']['classes'];
  }
  $form['instance_settings']['classes'] = array(
    '#title' => t('Extra CSS classes'),
    '#type' => 'textfield',
    '#default_value' =>  $classes_default,
    '#weight' => 3,
    '#element_validate' => array('field_group_validate_css_class'),
  );

  $description_default = '';
  if (isset($instance_settings['description'])) {
    $description_default = $instance_settings['description'];
  }
  elseif (isset($formatter['instance_settings']['description'])) {
    $description_default = $formatter['instance_settings']['description'];
  }
  $form['instance_settings']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $description_default,
    '#weight' => 0,
  );

  // Add optional instance_settings.
  switch ($group->format_type) {
    case 'div':
      $effect_default = '';
      if (isset($instance_settings['effect'])) {
        $effect_default = $instance_settings['effect'];
      }
      else {
        $effect_default = $formatter['instance_settings']['effect'];
      }
      $form['instance_settings']['effect'] = array(
        '#title' => t('Effect'),
        '#type' => 'select',
        '#options' => array('none' => t('None'), 'blind' => t('Blind')),
        '#default_value' => $effect_default,
        '#weight' => 2,
      );
      $speed_default = '';
      if (isset($instance_settings['speed'])) {
        $speed_default =  $instance_settings['speed'];
      }
      else {
        $speed_default = $formatter['instance_settings']['speed'];
      }
      $form['instance_settings']['speed'] = array(
        '#title' => t('Speed'),
        '#type' => 'select',
        '#options' => array(
          'none' => t('None'),
          'slow' => t('Slow'),
          'fast' => t('Fast'),
        ),
        '#default_value' => $speed_default,
        '#weight' => 3,
      );
      break;

    case 'fieldset':
      $classes_default = '';
      if (isset($instance_settings['classes'])) {
        $classes_default = $instance_settings['classes'];
      }
      elseif (isset($formatter['instance_settings']['classes'])) {
        $classes_default = $formatter['instance_settings']['classes'];
      }
      $form['instance_settings']['classes'] = array(
        '#title' => t('Extra CSS classes'),
        '#type' => 'textfield',
        '#default_value' => $classes_default,
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
 * Create the wrapper element to contain the fields.
 *
 * In the example beneath, some variables are prepared and used when building
 * the actual wrapper element. All elements in the Form API can be used.
 *
 * Note that at this point, the field group has no notion of the fields in it.
 *
 * There is also an alternative way of handling this. The default implementation
 * within field_group calls "field_group_pre_render_<format_type>".
 *
 * @param Array $elements
 *   Elements by address.
 * @param Object $group
 *   The Field group info.
 * @param (array) $form
 * 
 * @see field_group_pre_render_fieldset().
 */
function hook_field_group_pre_render(&$element, $group, &$form) {
  $element += array(
    '#type' => 'fieldset',
    '#title' => check_plain(t($group->label)),
    '#collapsible' => $group->collapsible,
    '#collapsed' => $group->collapsed,
    '#pre_render' => array(),
    '#attributes' => array('class' => explode(' ', $group->classes)),
    '#description' => $group->description,
  );

  if ($group->collapsible || $group->collapsed) {
    $element['#attached']['library'][] = array('system', 'backdrop.collapse');
  }
}

/**
 * Alter the pre_render array.
 * 
 * @param Array $element
 *   Elements by address.
 * @param Object $group 
 *   The Field group info.
 * @param (array) $form
 */
function hook_field_group_pre_render_alter(&$element, $group, &$form) {
  if ($group->format_type == 'htab') {
    $element['#theme_wrappers'] = array('my_horizontal_tab');
  }
}

/**
 * Alter the pre_render build.
 *
 * When you need this function, you most likely have a very custom case.
 *
 * @param Array $element 
 *   Elements by address.
 */
function hook_field_group_build_pre_render_alter(&$element) {

  // Prepare variables.
  $display = isset($element['#view_mode']);
  $groups = array_keys($element['#groups']);

  // Example from field_group itself to unset empty elements.
  if ($display) {
    foreach (element_children($element) as $name) {
      if (in_array($name, $groups)) {
        if (field_group_field_group_is_empty($element[$name], $groups)) {
          unset($element[$name]);
        }
      }
    }
  }

  // You might include additional javascript files and stylesheets.
  $path = backdrop_get_path('module', 'field_group');
  $element['#attached']['js'][] = $path . '/js/field_group.js';
  $element['#attached']['css'][] = $path . '/css/field_group.css';
}

/**
 * Override or change default summary behavior.
 *
 * In most cases the implementation of field group itself will be enough.
 * 
 * @param Object $group 
 *   The field group info.
 */
function hook_field_group_format_summary($group) {
  // Create additional summary or change the default setting.
  return t('New summary');
}

/**
 * Provide definitions for the field group.
 */
function hook_field_group_info() {
  // Provide definitions.
}

/**
 * Alter the group definitions provided by other modules.
 *
 * @param array $groups
 *   Reference to an array of field group definition objects.
 */
function hook_field_group_info_alter(&$groups) {
  if (!empty($groups['group_issue_metadata|node|project_issue|form'])) {
    $info = &$groups['group_issue_metadata|node|project_issue|form'];
    $info->data['children'][] = 'taxonomy_vocabulary_9';
  }
}

/**
 * Respond to updates to a group.
 *
 * @param $object $group
 *   The FieldGroup object.
 */
function hook_field_group_update_field_group($group) {
  // Update data depending on the group.
}

/**
 * Respond when a group is deleted.
 *
 * @param $object $group
 *   The FieldGroup object.
 */
function hook_field_group_delete_field_group($group) {
  // Delete extra data depending on the group.
}

/**
 * Respond to group creation.
 *
 * @param $object $group
 *   The FieldGroup object.
 */
function hook_field_group_create_field_group($group) {
  // Create extra data depending on the group.
}


/**
 * @} End of "addtogroup hooks".
 */
