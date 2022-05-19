<?php
/**
 * @file
 * Hooks provided by the Field Group module.
 *
 * Fieldgroup is a module that will wrap fields and other fieldgroups on forms 
 * and on display modes.
 *
 * DEVELOPERS NOTES
 *
 * - Fieldgroup uses a '#fieldgroups' property to know what fieldgroups are to 
 *   be pre_rendered and rendered by the field_group module. #fieldgroups is 
 *   later merged with the normal #groups that can be used by any other module.
 */

/**
 * @addtogroup hooks
 * @{
 */


/**
 * Javascript hooks
 *
 * Backdrop.FieldGroup.Effects.processHook.execute()
 * See field_group.js for the examples for all implemented formatters.
 */


/**
 * Define the information on formatters. 
 *
 * The formatters are separated by dispalay mode type. Use "form" for all form 
 * elements and "display" will be the real display modes (full, teaser, etc).
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
 *       'default_formatter' => 'collapsible',
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
 *       'default_formatter' => 'collapsible',
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
        'description' => t('This fieldgroup renders the inner content in a fieldset with the title as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array('classes' => ''),
        'default_formatter' => 'collapsible',
      ),
    ),
    'display' => array(
      'div' => array(
        'label' => t('Div'),
        'description' => t('This fieldgroup renders the inner content in a simple div with the title as legend.'),
        'format_types' => array('open', 'collapsible', 'collapsed'),
        'instance_settings' => array('effect' => 'none', 'speed' => 'fast', 'classes' => ''),
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

  // Add the required formatter type selector.
  if (isset($formatter['format_types'])) {
    $form['formatter'] = array(
      '#title' => t('Fieldgroup settings'),
      '#type' => 'select',
      '#options' => backdrop_map_assoc($formatter['format_types']),
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
 * Create the wrapper element to contain the fields.
 *
 * In the example beneath, some variables are prepared and used when building the
 * actual wrapper element. All elements in backdrop fapi can be used.
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
 * @see field_group_pre_render_fieldset.
 */
function hook_field_group_pre_render(&$element, $group, &$form) {

  // You can prepare some variables to use in the logic.
  $view_mode = isset($form['#view_mode']) ? $form['#view_mode'] : 'form';
  $id = $form['#entity_type'] . '_' . $form['#bundle'] . '_' . $view_mode . '_' . $group->group_name;

  // Each formatter type can have whole different set of element properties.
  switch ($group->format_type) {

    // Normal or collapsible div.
    case 'div':
      $effect = isset($group->format_settings['instance_settings']['effect']) ? $group->format_settings['instance_settings']['effect'] : 'none';
      $speed = isset($group->format_settings['instance_settings']['speed']) ? $group->format_settings['instance_settings']['speed'] : 'none';
      $add = array(
        '#type' => 'markup',
        '#weight' => $group->weight,
        '#id' => $id,
      );
      $classes .= " speed-$speed effect-$effect";
      if ($group->format_settings['formatter'] != 'open') {
        $add['#prefix'] = '<div class="field-group-format ' . $classes . '">
          <span class="field-group-format-toggler">' . check_plain(t($group->label)) . '</span>
          <div class="field-group-format-wrapper" style="display: none;">';
        $add['#suffix'] = '</div></div>';
      }
      else {
        $add['#prefix'] = '<div class="field-group-format ' . $group->group_name . ' ' . $classes . '">';
        $add['#suffix'] = '</div>';
      }
      if (!empty($description)) {
        $add['#prefix'] .= '<div class="description">' . $description . '</div>';
      }
      $element += $add;

      if ($effect == 'blind') {
        backdrop_add_library('system', 'effects.blind');
      }

      break;
    break;
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
  $element['#attached']['js'][] = backdrop_get_path('module', 'field_group') . '/js/field_group.js';
  $element['#attached']['css'][] = backdrop_get_path('module', 'field_group') . '/css/field_group.css';
}

/**
 * Override or change default summary behavior.
 *
 * In most cases the implementation of field group itself will be enough.
 * 
 * @param Object $group 
 *   The Field group info.
 */
function hook_field_group_format_summary($group) {
  $output = '';
  // Create additional summary or change the default setting.
  return $output;
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
    $groups['group_issue_metadata|node|project_issue|form']->data['children'][] = 'taxonomy_vocabulary_9';
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
