<?php
/**
 * @file
 * Provides configurable (flexible) layout templates.
 *
 * This provides the UI to list, create, edit and delete flexible layouts.
 */

/**
 * Form to add a flexible template.
 *
 * @ingroup forms
 */
function layout_flexible_template_settings_add_form() {
  $flexible_template = new LayoutFlexibleTemplate();
  $flexible_template->is_new = TRUE;
  backdrop_set_title(t('Add template'));
  return backdrop_get_form('layout_flexible_template_settings_edit_form', $flexible_template);
}

/**
 * Form to configure a flexible template.
 *
 * @param LayoutFlexibleTemplate $flexible_template
 *   The loaded flexible template object.
 *
 * @ingroup forms
 */
function layout_flexible_template_settings_edit_form($form, &$form_state, LayoutFlexibleTemplate $flexible_template) {
  $form_state['flexible_template'] = $flexible_template;
  $template_exists = layout_flexible_template_load($flexible_template->name);

  $form['name'] = array(
    '#title' => t('Template name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $flexible_template->title,
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#default_value' => $flexible_template->name,
    '#disabled' => ($template_exists && empty($flexible_template->is_new)),
    '#machine_name' => array(
      'exists' => 'layout_flexible_template_load',
    ),
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => $flexible_template->description,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#submit' => array(
      'layout_flexible_template_settings_form_submit',
    ),
    '#value' => t('Save and configure'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#limit_validation_errors' => array(array('name', 'machine_name')),
    '#submit' => array(
      'layout_flexible_template_settings_form_cancel',
    ),
  );

  return $form;
}

/**
 * Submit handler that cancels the flexible template form.
 *
 * @see layout_flexible_template_settings_edit_form()
 */
function layout_flexible_template_settings_form_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler that saves a flexible template.
 *
 * @see layout_flexible_template_settings_edit_form()
 */
function layout_flexible_template_settings_form_submit($form, &$form_state) {
  /* @var LayoutFlexibleTemplate $flexible_template */
  $flexible_template = $form_state['flexible_template'];
  $flexible_template->name = $form_state['values']['machine_name'];
  $flexible_template->title = $form_state['values']['name'];
  $flexible_template->description = $form_state['values']['description'];
  unset($flexible_template->is_new);

  $flexible_template->save();

  layout_flexible_tempstore_set($flexible_template);

  backdrop_set_message(t('Layout template "@title" saved.', array('@title' => $flexible_template->title)));
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['values']['machine_name'] . '/configure';
}

/**
 * Form to manage adding and removing rows to a flexible template.
 *
 * @param LayoutFlexibleTemplate $flexible_template
 *   The template configuration being edited.
 *
 * @ingroup forms
 */
function layout_flexible_template_configure_form($form, &$form_state, LayoutFlexibleTemplate $flexible_template) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('This page allows you to add or remove rows, and change the order of rows by dragging them up and down. You can also change the number of regions within each row, and adjust the width of each region.'),
  );

  $messages = array();
  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => $messages,
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $form_state['flexible_template'] = &$flexible_template;
  $flexible_template_name = $flexible_template->name;

  $config = array(
    'is_new' => TRUE,
    'layout_template' => $flexible_template_name,
  );
  $layout = new Layout($config);
  $template = layout_get_layout_template_info($flexible_template_name);

  backdrop_set_title($template['title']);

  $form_state['flexible_template_name'] = $flexible_template_name;

  if (empty($form_state['flexible_template'])) {
    return array();
  }

  $renderer = layout_create_renderer('flexible', $layout);

  $form['content'] = array(
    '#type' => 'item',
    '#id' => 'layout-flexible-content',
  );
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $form['content']['row_positions'] = array(
    // Use 'hidden' instead of 'value' so the JS can access it.
    '#type' => 'hidden',
    '#default_value' => implode(',', array_keys($flexible_template->rows)),
  );

  $form['content']['add_row'] = array(
    '#type' => 'submit',
    '#value' => t('Add row'),
    '#attributes' => array('class' => array('layout-flexible-add-row', 'button-secondary')),
    '#submit' => array(
      'layout_flexible_template_add_row',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save layout template'),
    '#submit' => array(
      'layout_flexible_template_configure_form_submit',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array(
      'layout_flexible_template_configure_form_cancel',
    ),
  );

  return $form;
}

/**
 * Handler for layout_flexible_template_configure_form() to add a new row.
 */
function layout_flexible_template_add_row($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/row/add/select-widths';
}

/**
 * Submit handler that cancels in-progress changes.
 *
 * @see layout_flexible_template_configure_form()
 */
function layout_flexible_template_configure_form_cancel($form, &$form_state) {
  tempstore_clear('layout.flexible', $form_state['flexible_template_name']);
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler that saves the configured flexible template.
 *
 * @see layout_flexible_template_configure_form()
 */
function layout_flexible_template_configure_form_submit($form, &$form_state) {
  // Load from tempstore rather than using $form_state, in the event a row
  // has been removed in an AJAX request.
  $flexible_template = layout_flexible_tempstore_load($form_state['flexible_template_name']);
  $positions = array();

  if (!empty($form_state['values']['row_positions'])) {
    $rows = array_filter(explode(',', $form_state['values']['row_positions']));
    foreach ($rows as $position) {
      if (isset($flexible_template->rows[$position])) {
        $positions[$position] = $flexible_template->rows[$position];
      }
    }
  }

  $flexible_template->rows = $positions;
  $flexible_template->save();

  tempstore_clear('layout.flexible', $form_state['flexible_template_name']);

  backdrop_set_message(t('Layout template "@title" saved.', array('@title' => $flexible_template->title)));
}

/**
 * Form to select the region widths for a flexible template.
 *
 * @param LayoutFlexibleTemplate $flexible_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_region_style_select($form, &$form_state, LayoutFlexibleTemplate $flexible_template, $original_row) {
  $new_row = ($original_row == 'add');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  $form_state['flexible_template_name'] = $flexible_template->name;
  $form_state['original_row'] = $original_row;

  $row_styles = layout_flexible_row_styles();
  $count_options = array();
  foreach ($row_styles as $name => $row_style) {
    $count_options[$row_style['region_count']] = $row_style['region_count'];
  }

  $row_style_in_use = 'region_12';
  $default_region_count = 1;
  if (!$new_row) {
    if (isset($form_state['values']['region_style'])) {
      $row_style_in_use = $form_state['values']['region_style'];
    }
    else {
      $row_style_in_use = $flexible_template->rows[$original_row]['contains'];
    }
    $default_region_count = $row_styles[$row_style_in_use]['region_count'];
  }
  $form['region_count'] = array(
    '#title' => t('Number of regions'),
    '#type' => 'radios',
    '#default_value' => $default_region_count,
    '#parents' => array('region_count'),
    '#options' => $count_options,
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_region_style_ajax',
      'wrapper' => 'row_settings',
      'trigger_as' => array('name' => 'region_count_update'),
    ),
  );
  $form['region_count_update'] = array(
    '#type' => 'submit',
    '#value' => t('Update regions'),
    '#attributes' => array('class' => array('js-hide')),
    '#name' => 'region_count_update',
    '#validate' => array(),
    '#submit' => array('layout_flexible_template_edit_region_style_submit'),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_region_style_ajax',
      'wrapper' => 'row_settings',
    ),
  );

  $form['row_settings'] = array(
    '#type' => 'container',
    '#id' => 'row_settings',
    '#parents' => array('row_settings'),
  );

  $region_count = isset($form_state['values']['region_count']) ? $form_state['values']['region_count'] : $default_region_count;

  $options = array();
  $row_styles = layout_flexible_row_styles();
  foreach ($row_styles as $name => $row_style) {
    if ($row_style['region_count'] == $region_count) {
      $options[$name] = theme('layout_flexible_template_style_option', array('row_style' => $row_style));
    }
  }

  if (empty($options)) {
    $options['region_12'] = '12:0';
  }

  $default_region_style = $row_style_in_use;
  if (!isset($options[$row_style_in_use])) {
    $default_region_style = key($options);
  }
  $form['row_settings']['region_style'] = array(
    '#title' => t('Region widths'),
    '#type' => 'radios',
    '#default_value' => $default_region_style,
    '#options' => $options,
  );

  $form['submit_region_style'] = array(
    '#type' => 'submit',
    '#value' => t('Choose region widths'),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
    '#submit' => array(
      'layout_flexible_template_region_count_load',
    ),
  );

  $submit_button_value = $new_row ? t('Continue') : t('Save region widths');
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#name' => 'submit',
    '#value' => $submit_button_value,
    '#attributes' => array('class' => array('layout-title-button')),
    '#submit' => array(
      'layout_flexible_template_region_style_select_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );
  if (isset($flexible_template->rows[$original_row]['in_progress'])) {
    $cancel_submit_callback = 'layout_flexible_template_region_style_select_cancel';
    $cancel_ajax_callback = 'layout_ajax_form_open_dialog';
  }
  else {
    $cancel_submit_callback = 'layout_flexible_template_cancel';
    $cancel_ajax_callback = 'layout_flexible_template_cancel_ajax';
  }
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#submit' => array(
      $cancel_submit_callback,
    ),
    '#ajax' => array(
      'callback' => $cancel_ajax_callback,
    ),
  );

  return $form;
}

/**
 * Submit handler that cancels the region style selection form.
 *
 * @see layout_flexible_template_region_style_select().
 */
function layout_flexible_template_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/configure';
}

/**
 * AJAX handler to cancels the region style selection form.
 *
 * @see layout_flexible_template_region_style_select().
 */
function layout_flexible_template_cancel_ajax($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Submit handler that cancels the region style selection form.
 *
 * @see layout_flexible_template_region_style_select().
 */
function layout_flexible_template_region_style_select_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/configure';
}

/**
 * Submit handler that saves the region style selection form.
 *
 * @see layout_flexible_template_region_style_select().
 */
function layout_flexible_template_region_style_select_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/configure/' . $form_state['values']['region_style'];
}

/**
 * Form to add or edit a row on a flexible template.
 *
 * @param LayoutFlexibleTemplate $flexible_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 * @param string $region_style
 *   The selected region style.
 *
 * @ingroup forms
 */
function layout_flexible_template_edit_row_form($form, &$form_state, LayoutFlexibleTemplate $flexible_template, $original_row, $region_style = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['flexible_template'] = &$flexible_template;
  $flexible_template_name = $flexible_template->name;

  $form_state['flexible_template_name'] = $flexible_template_name;
  $form_state['original_row'] = $original_row;

  $row_data = array();
  if ($original_row != 'add') {
    if (isset($flexible_template->rows[$original_row]['in_progress'])) {
      $row_data = $flexible_template->rows[$original_row]['in_progress'];
    }
    else {
      $row_data = $flexible_template->rows[$original_row];
    }
  }

  $region_style = $region_style ? $region_style : $row_data['contains'];
  $form_state['region_style'] = $region_style;

  $row_styles = layout_flexible_row_styles();
  $selected_style = $row_styles[$region_style]['name'];
  $region_count = $row_styles[$region_style]['region_count'];

  if ($original_row == 'add') {
    backdrop_set_title(t('Add new row'));
  }
  else {
    backdrop_set_title(t('Configure row !original_row', array('!original_row' => $original_row)));
  }

  $form['region_style'] = array(
    '#type' => 'item',
    '#title' => t('Selected region widths'),
    '#markup' => $selected_style,
  );

  if ($original_row != 'add') {
    $form['change_region_style'] = array(
      '#type' => 'submit',
      '#value' => t('Change region widths'),
      '#attributes' => array('class' => array('layout-link-button')),
      '#submit' => array(
        'layout_flexible_template_change_region_style',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
    );
  }

  $form['region_names'] = array(
    '#type' => 'fieldset',
    '#title' => t('Regions'),
    '#collapsed' => $original_row != 'add',
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  $form_state['last_region_number'] = layout_flexible_template_get_last_region_number($flexible_template);
  for ($i = 0; $i < $region_count; $i++) {
    $form['region_names']['region_' . $i] = array(
      '#type' => 'container',
    );
    if (isset($row_data['region_names']['region_' . $i]['label'])) {
      $region_name = $row_data['region_names']['region_' . $i]['label'];
    }
    $form['region_names']['region_' . $i]['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Region name') . ' ' . ($i + 1),
      '#default_value' => !empty($region_name) ? $region_name : t('Region') . ' ' . ($form_state['last_region_number'] + $i + 1),
    );
    $form['region_names']['region_' . $i]['name'] = array(
      '#type' => 'machine_name',
      '#description' => t('A machine name to be used as the default CSS class. It must only contain uppercase letters, lowercase letters, numbers, hyphens and underscores.'),
      '#default_value' => !empty($row_data['region_names']['region_' . $i]['name']) ? $row_data['region_names']['region_' . $i]['name'] : '',
      '#maxlength' => 255,
      '#field_prefix' => '<span dir="ltr">l-region--',
      '#field_suffix' => '</span>&lrm;',
      '#size' => 15,
      '#disabled' => FALSE,
      '#machine_name' => array(
        'exists' => '_layout_flexible_template_region_name_exists',
        'source' => array('region_names', 'region_' . $i, 'label'),
        'standalone' => TRUE,
        'label' => t('Name'),
        'replace_pattern' => '[^a-z0-9-]+',
        'replace' => '-',
      ),
    );
    if (isset($row_data['region_names']['region_' . $i]['region_class_enable'])){
      $region_class_enable = $row_data['region_names']['region_' . $i]['region_class_enable'];
    }
    $form['region_names']['region_' . $i]['region_class_enable'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add CSS classes'),
      '#default_value' => !empty($region_class_enable) ? $region_class_enable : 0,
    );
    if (isset($row_data['region_names']['region_' . $i]['classes'])) {
      $classes = $row_data['region_names']['region_' . $i]['classes'];
    }
    $form['region_names']['region_' . $i]['classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Additional region CSS classes'),
      '#description' => t('Separate multiple classes with spaces.'),
      '#default_value' => !empty($classes) ? $classes : '',
      '#states' => array(
        'visible' => array(
          ':input[name="region_names[region_' . $i . '][region_class_enable]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $form['region_styles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Row Style'),
    '#collapsed' => $original_row != 'add',
    '#collapsible' => TRUE,
  );

  $options = array(
    'div' => 'DIV',
    'nav' => 'NAV',
    'aside' => 'ASIDE',
    'section' => 'SECTION',
    'header' => 'HEADER',
    'footer' => 'FOOTER',
    'main' => 'MAIN',
  );
  $form['region_styles']['element'] = array(
    '#title' => t('Row wrapper tag'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($row_data['element']) ? $row_data['element'] : '',
  );

  $form['region_styles']['container'] = array(
    '#title' => t('Row width behavior'),
    '#type' => 'radios',
    '#options' => array(
      'container' => t('Fixed maximum width'),
      'container_fluid' => t('Fluid width'),
      'no_container' => t('Full width'),
    ),
    '#default_value' => isset($row_data['container']) ? $row_data['container'] : 'container_fluid',
  );
  $form['region_styles']['container']['container']['#description'] = t('Adds the <code>container</code> class to the row.');
  $form['region_styles']['container']['container_fluid']['#description'] = t('Adds the <code>container-fluid</code> class to the row (no <code>max-width</code>, but <code>padding</code>).');
  $form['region_styles']['container']['no_container']['#description'] = t('No container-related classes added to the row (e.g. for Hero blocks).');

  $form['region_styles']['row_classes'] = array(
    '#title' => t('Additional row CSS classes'),
    '#type' => 'textfield',
    '#default_value' => isset($row_data['classes']) ? $row_data['classes'] : '',
    '#description' => t('Separate multiple classes with spaces.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#name' => 'submit',
    '#value' => t('Save configuration'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#validate' => array(
      'layout_flexible_template_edit_row_validate',
    ),
    '#submit' => array(
      'layout_flexible_template_edit_row_form_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_row_ajax',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#submit' => array(
      'layout_flexible_template_cancel',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_cancel_ajax',
    ),
  );

  return $form;
}

/**
 * Callback: Check if machine_name is unique.
 *
 * Even though callback is required for the Form API machine_name field type,
 * the region classes are not required to be unique. It will accept any
 * machine_name.
 *
 * @param $machine_name
 *   The machine name for a region.
 *
 * @return
 *   Whether or not the region name is taken.
 *
 * @see layout_flexible_template_edit_row_form()
 * @see form_validate_machine_name()
 */
function _layout_flexible_template_region_name_exists($machine_name, $element, $form_state) {
  $flexible_template = layout_flexible_tempstore_load($form_state['flexible_template_name']);
  foreach ($flexible_template->rows as $row) {
    foreach ($row['region_names'] as $region_name) {
      if ($region_name['name'] == $machine_name) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
 * Returns the last number of all regions named "Region X" where X is a number.
 */
function layout_flexible_template_get_last_region_number(LayoutFlexibleTemplate $flexible_template, $form_state = NULL) {
  $last = 0;
  $region_names = array();
  if ($form_state) {
    $regions = array_filter($form_state['values']['region_names']);
    $last = $form_state['last_region_number'];
    foreach ($regions as $key => $region) {
      $region_names[] = $region['label'];
    }
  }
  else {
    foreach ($flexible_template->rows as $row) {
      foreach ($row['region_names'] as $key => $region) {
        $region_names[] = $region['label'];
      }
    }
  }

  foreach ($region_names as $region) {
    $str = explode('Region ', $region);
    if (!empty($str[1]) && is_numeric($str[1]) && $str[1] > $last) {
      $last = $str[1];
    }
  }

  return $last;
}

/**
 * Submit handler for the change region style button on the
 * layout_flexible_template_edit_row_form().
 */
function layout_flexible_template_change_region_style($form, &$form_state) {
  $old_row = array(
    'contains' => $form_state['region_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'region_names' => $form_state['values']['region_names'],
  );
  $form_state['flexible_template']->rows[$form_state['original_row']]['in_progress'] = $old_row;

  layout_flexible_tempstore_set($form_state['flexible_template']);

  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/row/' . $form_state['original_row'] . '/select-widths';
}

/**
 * Submit handler on the hidden button to change region widths on
 * layout_flexible_template_edit_row_form() if AJAX is disabled.
 */
function layout_flexible_template_region_count_load(&$form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Validation handler for layout_flexible_template_edit_row_form().
 */
function layout_flexible_template_edit_row_validate($form, &$form_state) {
  $last_region_number = layout_flexible_template_get_last_region_number($form_state['flexible_template'], $form_state);

  foreach ($form_state['values']['region_names'] as $key => $region) {
    if (empty($region)) {
      $number = str_replace('region_', '', $key);
      $form_state['values']['region_names'][$key]['label'] = t('Region') . ' ' . ($last_region_number + $number + 1);
      $last_region_number++;
    }
  }
}

/**
 * Submit handler for layout_flexible_template_edit_row_form().
 */
function layout_flexible_template_edit_row_form_submit($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  /* @var LayoutFlexibleTemplate $flexible_template */
  $flexible_template = $form_state['flexible_template'];
  $original_row = $form_state['original_row'];

  $new_row = array(
    'contains' => $form_state['region_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'region_names' => $form_state['values']['region_names'],
  );

  if ($original_row == 'add') {
    $uuid = new Uuid();
    $original_row = $uuid->generate();
  }

  $flexible_template->rows[$original_row] = $new_row;

  $form_state['flexible_template'] = $flexible_template;

  $form_state['ajax_rebuild_form'] = 'layout_flexible_template_configure_form';
  $form_state['ajax_rebuild_args'] = array($flexible_template);
  $form_state['ajax_update'] = array('content');
  layout_flexible_tempstore_set($flexible_template);
  layout_flexible_reset_caches();
  if (!backdrop_is_ajax()) {
    $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_name'] . '/configure';
  }
}

/**
 * AJAX callback to change the region count.
 *
 * @see layout_flexible_template_region_style_select()
 */
function layout_flexible_template_edit_region_style_ajax($form, &$form_state) {
  return $form['row_settings'];
}

/**
 * Submit callback to change the region count.
 *
 * @see layout_flexible_template_region_style_select()
 */
function layout_flexible_template_edit_region_style_submit($form, &$form_state) {
  // Remove the current region style when switching between region counts.
  $form_state['input']['region_style'] = NULL;
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX callback on the submit button for editing a layout row.
 *
 * @see layout_flexible_template_edit_row_form().
 */
function layout_flexible_template_edit_row_ajax($form, $form_state) {
  $commands = array();
  // Display error messages in the form if any.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
  }
  else {
    $commands[] = ajax_command_close_modal_dialog();

    $update_ajax = layout_ajax_form_update($form, $form_state);
    $commands = array_merge($commands, $update_ajax['#commands']);
  }

  // Update the messages area.
  $commands[] = ajax_command_remove('#messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Menu callback to delete a row from a flexible template.
 *
 * @param LayoutFlexibleTemplate $flexible_template
 *   The loaded flexible template object.
 * @param string $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_row(LayoutFlexibleTemplate $flexible_template, $original_row) {
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'layout-region-' . $original_row)) {
    return MENU_ACCESS_DENIED;
  }

  $commands = array();
  $flexible_template_name = $flexible_template->name;

  unset($flexible_template->rows[$original_row]);
  layout_flexible_tempstore_set($flexible_template);

  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#flexible-row--' . $original_row);

    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Row "@title" removed.', array('@title' => $flexible_template->title)));
    backdrop_goto('admin/structure/layouts/settings/flexible-template/' . $flexible_template_name . '/configure');
    return array();
  }
}

/**
 * Menu callback; Delete a flexible template.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_form($form, &$form_state, LayoutFlexibleTemplate $flexible_template) {
  $form_state['flexible_template'] = &$flexible_template;
  $question = t('Delete flexible template @title?', array('@title' => $flexible_template->title));
  $description = t('This action cannot be undone.');
  backdrop_set_title($question, PASS_THROUGH);

  // Check if this template is in use.
  $errors = array();
  foreach (layout_load_all() as $layout) {
    if ($layout->layout_template == $flexible_template->name) {
      $errors[] = check_plain($layout->title);
    }
  }

  if (!empty($errors)) {
    backdrop_set_message(t('The "@title" layout template is currently in use in the following layouts and may not be deleted: !list', array('@title' => $flexible_template->title, '!list' => theme('item_list', array('items' => $errors)))), 'error');
  }
  else {
    $form['description'] = array(
      '#markup' => $description
    );
  }

  $form['actions'] = array(
    '#type' => 'actions'
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#disabled' => !empty($errors),
    '#value' => t('Delete template'),
    '#attributes' => array('class' => array('button-danger')),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/layouts/settings',
  );

  $form['#attributes']['class'][] = 'confirmation';
  $form['#theme'] = 'confirm_form';

  return $form;
}

/**
 * Submit handler that deletes a flexible template.
 *
 * @see layout_flexible_template_delete_form()
 */
function layout_flexible_template_delete_form_submit(&$form, &$form_state) {
  /* @var LayoutFlexibleTemplate $flexible_template */
  $flexible_template = &$form_state['flexible_template'];
  $flexible_template->delete();

  backdrop_set_message(t('The layout template "@title" has been deleted.', array('@title' => $flexible_template->title)));
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}
