<?php
/**
 * @file
 * Defines some field group types.
 */

/**
 * Field group extra formatters
 *
 * @see field_group_field_group_formatter_info()
 */
function _field_group_extra_field_group_formatter_info() {

  return array(
    'form' => array(
      'htabs' => array(
        'label' => t('Horizontal tabs group'),
        'description' => t('This fieldgroup renders child groups in its own horizontal tabs wrapper.'),
        'instance_settings' => array('classes' => '', 'id' => ''),
      ),
      'htab' => array(
        'label' => t('Horizontal tab'),
        'format_types' => array('open', 'closed'),
        'description' => t('This fieldgroup renders the content in a fieldset, part of horizontal tabs group.'),
        'default_formatter' => 'closed',
        'instance_settings' => array('description' => '', 'classes' => '', 'required_fields' => 1),
      ),
      'multipage-group' => array(
        'label' => t('Multipage group'),
        'description' => t('This fieldgroup renders groups on separate pages.'),
        'instance_settings' => array('classes' => '', 'page_header' => 3, 'move_additional' => 1, 'page_counter' => 1, 'move_button' => 0),
      ),
      'multipage' => array(
        'label' => t('Multipage'),
        'format_types' => array('start', 'no-start'),
        'description' => t('This fieldgroup renders the content in a page.'),
        'default_formatter' => 'no-start',
        'instance_settings' => array('description' => '', 'classes' => '', 'required_fields' => 1),
      ),
      'accordion' => array(
        'label' => t('Accordion group'),
        'description' => t('This fieldgroup renders child groups as jQuery accordion.'),
        'instance_settings' => array('effect' => 'none', 'classes' => '', 'id' => ''),
      ),
      'accordion-item' => array(
        'label' => t('Accordion item'),
        'format_types' => array('open', 'closed'),
        'description' => t('This fieldgroup renders the content in a div, part of accordion group.'),
        'default_formatter' => 'closed',
        'instance_settings' => array('description' => '', 'classes' => '', 'required_fields' => 1),
      ),
    ),
    'display' => array(
      'htabs' => array(
        'label' => t('Horizontal tabs group'),
        'description' => t('This fieldgroup renders child groups in its own horizontal tabs wrapper.'),
        'instance_settings' => array('classes' => '', 'id' => ''),
      ),
      'htab' => array(
        'label' => t('Horizontal tab item'),
        'format_types' => array('open', 'closed'),
        'description' => t('This fieldgroup renders the content in a fieldset, part of horizontal tabs group.'),
        'instance_settings' => array('description' => '', 'classes' => '', 'id' => ''),
        'default_formatter' => 'closed',
      ),
      'accordion' => array(
        'label' => t('Accordion group'),
        'description' => t('This fieldgroup renders child groups as jQuery accordion.'),
        'instance_settings' => array('description' => '', 'classes' => '', 'effect' => 'bounceslide', 'id' => ''),
      ),
      'accordion-item' => array(
        'label' => t('Accordion item'),
        'format_types' => array('open', 'closed'),
        'description' => t('This fieldgroup renders the content in a div, part of accordion group.'),
        'instance_settings' => array('classes' => ''),
        'default_formatter' => 'closed',
      ),
    ),
  );
}

/**
 * Implements hook_theme().
 */
function field_group_theme() {
  return array(
    'horizontal_tabs' => array(
      'render element' => 'element',
      'file' => 'extra/field_group.theme.inc',
    ),
    'multipage' => array(
      'render element' => 'element',
      'file' => 'extra/field_group.theme.inc',
    ),
    'multipage_pane' => array(
      'render element' => 'element',
      'file' => 'extra/field_group.theme.inc',
    ),
  );
}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Accordion.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_accordion(&$element, $group, &$form) {

  // Add the jQuery UI accordion.
  $element['#attached']['library'][] = array('system', 'ui.accordion');

  $id = !empty($element['#id']) ? ' id="' . $element['#id'] . '"' : '';

  $element += array(
    '#type' => 'markup',
    '#prefix' => '<div class="' . $group->classes . '"' . $id .'>',
    '#suffix' => '</div>',
  );
}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Accordion item.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_accordion_item(&$element, $group, &$form) {

  $element += array(
    '#type' => 'markup',
    '#prefix' => '<h3 class="field-group-format-toggler ' . $group->format_type . ($group->collapsed ? '' : ' field-group-accordion-active') . '"><a href="#">' . check_plain(t($group->label)) . '</a></h3>
    <div class="field-group-format-wrapper ' . $group->classes . '">',
    '#suffix' => '</div>',
    //'#attributes' => array('class' => array($group->format_type)),
  );
  if (!empty($group->description)) {
    $element['#prefix'] .= '<div class="description">' . $group->description . '</div>';
  }

}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Horizontal tabs group.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_htabs(&$element, $group, &$form) {

  $classes = 'field-group-' . $group->format_type . '-wrapper';
  if (!empty($group->classes)) {
    $classes .= ' ' . $group->classes;
  }

  $id = !empty($element['#id']) ? ' id="' . $element['#id'] . '"' : '';

  $element += array(
    '#type' => 'horizontal_tabs',
    '#title' => check_plain(t($group->label)),
    '#theme_wrappers' => array('horizontal_tabs'),
    '#prefix' => '<div class="' . $classes . '"' . $id . '>',
    '#suffix' => '</div>',
  );

  // By default vertical_tabs don't have titles but you can override it in the theme.
  if (!empty($group->label)) {
    $element['#title'] = check_plain($group->label);
  }

  // Only add form.js on forms.
  if (!empty($form['#type']) && $form['#type'] == 'form') {
    $element['#attached']['library'][] = array('system', 'backdrop.form');
  }

  $element['#attached']['library'][] = array('field_group', 'horizontal-tabs');
}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Horizontal tab.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_htab(&$element, $group, &$form) {

  $element += array(
    '#type' => 'fieldset',
    '#title' => check_plain(t($group->label)),
    '#collapsible' => $group->collapsible,
    '#collapsed' => $group->collapsed,
    '#attributes' => array('class' => explode(" ", $group->classes)),
    '#group' => $group->parent_name,
    // very important. Cannot be added on the form!
    '#parents' => array($group->parent_name),
    '#description' => $group->description,
  );

}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Multipage group.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_multipage_group(&$element, &$group, &$form) {

  $multipage_element = array(
    '#type' => 'multipage',
    '#theme_wrappers' => array('multipage'),
    '#prefix' => '<div class="field-group-' . $group->format_type . '-wrapper ' . $group->classes . '">',
    '#suffix' => '</div>',
  );

  $element += $multipage_element;

  $move_additional = isset($group->format_settings['instance_settings']['move_additional']) ? ($group->format_settings['instance_settings']['move_additional'] && isset($form['additional_settings'])) : isset($form['additional_settings']);
  $move_button = isset($group->format_settings['instance_settings']['move_button']) ? $group->format_settings['instance_settings']['move_button'] : 0;

  backdrop_add_js(array(
    'field_group' => array(
      'multipage_move_submit' => (bool) $move_button,
      'multipage_move_additional' => (bool) $move_additional
    )
  ), 'setting');

}

/**
 * Implements field_group_pre_render_<format-type>.
 * Format type: Multipage.
 *
 * @param array $element
 *   The field group form element.
 * @param object $group
 *   The Field group object prepared for pre_render.
 * @param array $form
 *   The root element or form.
 */
function field_group_pre_render_multipage(&$element, $group, &$form) {

  $group->classes .= $group->format_settings['formatter'] == 'start' ? ' multipage-open' : ' multipage-closed';
  $element += array(
    '#type' => 'multipage_pane',
    '#title' => check_plain(t($group->label)),
    '#collapsible' => $group->collapsible,
    '#collapsed' => $group->collapsed,
    '#attributes' => array('class' => explode(" ", $group->classes)),
    '#group' => $group->parent_name,
    '#group_object' => $group,
    '#parent_group_object' => $form['#groups'][$group->parent_name],
    // very important. Cannot be added on the form!
    '#parents' => array($group->parent_name),
    '#description' => $group->description,
  );

  $element['#attached']['library'][] = array('field_group', 'multipage');
}

/**
 * Implements hook_element_info().
 */
function field_group_element_info() {
  $types['horizontal_tabs'] = array(
    '#theme_wrappers' => array('horizontal_tabs'),
    '#default_tab' => '',
    '#process' => array('form_process_horizontal_tabs'),
  );
  $types['multipage'] = array(
    '#theme_wrappers' => array('multipage'),
    '#default_tab' => '',
    '#process' => array('form_process_multipage'),
  );
  $types['multipage_pane'] = array(
    '#value' => NULL,
    '#process' => array('form_process_fieldset', 'ajax_process_form'),
    '#pre_render' => array('form_pre_render_fieldset'),
    '#theme_wrappers' => array('multipage_pane'),
  );
  return $types;
}

/**
 * Implements hook_library_info().
 */
function field_group_library_info() {

  $path = backdrop_get_path('module', 'field_group');
  // Horizontal Tabs.
  $libraries['horizontal-tabs'] = array(
    'title' => 'Horizontal Tabs',
    'website' => 'http://drupal.org/node/323112',
    'version' => '1.0',
    'js' => array(
      $path . '/extra/js/horizontal-tabs/horizontal-tabs.js' => array(),
    ),
    'css' => array(
      $path . '/extra/css/horizontal-tabs/horizontal-tabs.css' => array(),
    ),
  );
  // Multipage Tabs.
  $libraries['multipage'] = array(
    'title' => 'Multipage',
    'website' => 'http://drupal.org/node/323112',
    'version' => '1.0',
    'js' => array(
      $path . '/extra/js/multipage/multipage.js' => array(),
    ),
    'css' => array(
      $path . '/extra/css/multipage/multipage.css' => array(),
    ),
  );

  return $libraries;
}

/**
 * Creates a group formatted as horizontal tabs.
 * This function will never be callable from within field_group rendering. Other
 * modules using #type horizontal_tabs will have the benefit of this processor.
 *
 * @param array $element
 *   An associative array containing the properties and children of the
 *   fieldset.
 * @param array $form_state
 *   The $form_state array for the form this horizontal tab widget belongs to.
 *
 * @return array
 *   The processed element.
 */
function form_process_horizontal_tabs($element, &$form_state) {
  // Inject a new fieldset as child, so that form_process_fieldset() processes
  // this fieldset like any other fieldset.
  $element['group'] = array(
    '#type' => 'fieldset',
    '#theme_wrappers' => array(),
    '#parents' => $element['#parents'],
  );

  // The JavaScript stores the currently selected tab in this hidden
  // field so that the active tab can be restored the next time the
  // form is rendered, e.g. on preview pages or when form validation
  // fails.
  $name = implode('__', $element['#parents']);
  if (isset($form_state['values'][$name . '__active_tab'])) {
    $element['#default_tab'] = $form_state['values'][$name . '__active_tab'];
  }
  $element[$name . '__active_tab'] = array(
    '#type' => 'hidden',
    '#default_value' => $element['#default_tab'],
    '#attributes' => array('class' => array('horizontal-tabs-active-tab')),
  );

  return $element;
}

/**
 * Creates a group formatted as multipage.
 * This function will never be callable from within field_group rendering. Other
 * modules using #type multipage will have the benefit of this processor.
 *
 * @param array $element
 *   An associative array containing the properties and children of the
 *   fieldset.
 * @param array $form_state
 *   The $form_state array for the form this multipage tab widget belongs to.
 *
 * @return array
 *   The processed element.
 */
function form_process_multipage($element, &$form_state) {
  // Inject a new fieldset as child, so that form_process_fieldset() processes
  // this fieldset like any other fieldset.
  $element['group'] = array(
    '#type' => 'fieldset',
    '#theme_wrappers' => array(),
    '#parents' => $element['#parents'],
  );

  // The JavaScript stores the currently selected tab in this hidden
  // field so that the active control can be restored the next time the
  // form is rendered, e.g. on preview pages or when form validation
  // fails.
  $name = implode('__', $element['#parents']);
  if (isset($form_state['values'][$name . '__active_control'])) {
    $element['#default_tab'] = $form_state['values'][$name . '__active_control'];
  }
  $element[$name . '__active_control'] = array(
    '#type' => 'hidden',
    '#default_value' => $element['#default_control'],
    '#attributes' => array('class' => array('multipage-active-control')),
  );

  return $element;
}

/**
 * Field group extra format settings.
 *
 * @see field_group_field_group_format_settings_alter()
 */
function _field_group_extra_field_group_format_settings_alter(&$form, &$group) {
  $field_group_types = field_group_formatter_info();
  $mode = $group->mode == 'form' ? 'form' : 'display';
  $formatter = $field_group_types[$mode][$group->format_type];

  // Add optional instance_settings.
  switch ($group->format_type) {
    case 'multipage-group':
      $form['instance_settings']['page_header'] = array(
        '#title' => t('Format page title'),
        '#type' => 'select',
        '#options' => array(0 => t('None'), 1 => t('Label only'), 2 => t('Step 1 of 10'), 3 => t('Step 1 of 10 [Label]'),),
        '#default_value' => isset($group->format_settings['instance_settings']['page_header']) ? $group->format_settings['instance_settings']['page_header'] : $formatter['instance_settings']['page_header'],
        '#weight' => 1,
      );
      $form['instance_settings']['page_counter'] = array(
        '#title' => t('Add a page counter at the bottom'),
        '#type' => 'select',
        '#options' => array(0 => t('No'), 1 => t('Format 1 / 10'), 2 => t('The count number only')),
        '#default_value' => isset($group->format_settings['instance_settings']['page_counter']) ? $group->format_settings['instance_settings']['page_counter'] : $formatter['instance_settings']['page_counter'],
        '#weight' => 2,
      );
      $form['instance_settings']['move_button'] = array(
        '#title' => t('Move submit button to last multipage'),
        '#type' => 'select',
        '#options' => array(0 => t('No'), 1 => t('Yes')),
        '#default_value' => isset($group->format_settings['instance_settings']['move_button']) ? $group->format_settings['instance_settings']['move_button'] : $formatter['instance_settings']['move_button'],
        '#weight' => 3,
      );
      $form['instance_settings']['move_additional'] = array(
        '#title' => t('Move additional settings to last multipage (if available)'),
        '#type' => 'select',
        '#options' => array(0 => t('No'), 1 => t('Yes')),
        '#default_value' => isset($group->format_settings['instance_settings']['move_additional']) ? $group->format_settings['instance_settings']['move_additional'] : $formatter['instance_settings']['move_additional'],
        '#weight' => 4,
      );
    case 'htabs':
      break;
    case 'accordion':
      $form['instance_settings']['effect'] = array(
        '#title' => t('Effect'),
        '#type' => 'select',
        '#options' => array('none' => t('None'), 'bounceslide' => t('Bounce slide')),
        '#default_value' => isset($group->format_settings['instance_settings']['effect']) ? $group->format_settings['instance_settings']['effect'] : $formatter['instance_settings']['effect'],
        '#weight' => 2,
      );
      break;
    case 'multipage':
      break;
    case 'htab':
    case 'accordion-item':
    default:
  }
}


/**
 * Pre render alter
 *
 * @see field_group_field_group_build_pre_render_alter().
 */
function _field_group_extra_field_group_build_pre_render_alter(&$element) {

  $element['#attached']['js'][] = backdrop_get_path('module', 'field_group') . '/extra/js/field_groups.js';

  // Move additional settings to the last multipage pane if configured that way.
  // Note that multipages MUST be in the root of the form.
  foreach (element_children($element) as $name) {
    if (isset($element[$name]['#type']) && $element[$name]['#type'] == 'multipage' && isset($element['additional_settings'])) {
      $last_pane = NULL;
      foreach (element_children($element[$name], TRUE) as $pane) {
        $last_pane = $pane;
      }
      $element[$name][$last_pane]['additional_settings'] = $element['additional_settings'];
      unset($element['additional_settings']);
    }
  }

}

/**
 * Parent requirements.
 *
 * @see field_group_field_group_field_ui_parent_requirements_alter().
 */
function _field_group_extra_field_group_field_ui_parent_requirements_alter(&$parent_requirements, &$context) {
  $parent_requirements = array(
    'multipage' => array(
      'parent' => 'multipage-group',
      'message' => 'Each Multipage element needs to have a parent Multipage group element.',
    ),
    'htab' => array(
      'parent' => 'htabs',
      'message' => 'Each Horizontal tab element needs to have a parent Horizontal tabs group element.',
    ),
    'accordion-item' => array(
      'parent' => 'accordion',
      'message' => 'Each Accordion item element needs to have a parent Accordion group element.',
    ),
  );
}

/**
 * Alters classes.
 *
 * @see field_group_field_group_html_classes_alter().
 */
function _field_group_extra_field_group_html_classes_alter(&$classes, &$group) {
  // Open or closed horizontal or vertical tabs will be collapsible by default.
  if ($group->format_type == 'htab') {
    $classes->required[] = 'collapsible';
  }

  if ($group->format_type == 'multipage') {
    $class_group_type = 'field-group-multipage';
    $classes->required[] = $class_group_type;
    if ((array_search($class_group_type, $classes->optional)) !== false){
      unset($classes->optional[$class_group_type]);
    }
  }

  if (isset($group->format_settings['instance_settings'])) {
    // Extra required classes for accordions.
    if ($group->format_type == 'accordion') {
      $classes->required[] = 'field-group-' . $group->format_type . '-wrapper';
      $effect = isset($group->format_settings['instance_settings']['effect']) ? $group->format_settings['instance_settings']['effect'] : 'none';
      $classes->required[] = 'effect-' . $effect;
    }
  }
}

/**
 * Implements hook_field_group_mode_types_excluded_alter().
 *
 * @param array $excluded_types
 *   An array of excluded format types.
 * @param string $mode
 *   The mode: form or display.
 */
function field_group_field_group_mode_types_excluded_alter(&$excluded_types, $mode) {
  if ($mode != 'form') {
    $excluded_types = array('multipage', 'multipage-group');
  }
}
