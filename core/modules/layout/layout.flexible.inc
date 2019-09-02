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
  $flexible_template = layout_flexible_template_template_default();
  $flexible_template['is_new'] = TRUE;
  backdrop_set_title(t('Add template'));
  return backdrop_get_form('layout_flexible_template_settings_edit_form', $flexible_template);
}

/**
 * Form to configure a flexible template.
 *
 * @param $flexible_template
 *   The loaded flexible template object.
 *
 * @ingroup forms
 */
function layout_flexible_template_settings_edit_form($form, &$form_state, $flexible_template) {
  $form_state['flexible_template'] = $flexible_template;
  $template_exists = layout_flexible_template_load($flexible_template['name']);

  $form['name'] = array(
    '#title' => t('Template name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $flexible_template['title'],
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#default_value' => $flexible_template['name'],
    '#disabled' => ($template_exists && empty($flexible_template['is_new'])),
    '#machine_name' => array(
      'exists' => 'layout_flexible_template_load',
    ),
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => $flexible_template['description'],
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
 * Submit handler for layout_flexible_template_settings_edit_form() that
 * cancels the form.
 */
function layout_flexible_template_settings_form_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler for layout_flexible_template_settings_edit_form() that
 * submits the form.
 */
function layout_flexible_template_settings_form_submit($form, &$form_state) {
  $flexible_template = $form_state['flexible_template'];
  $flexible_template['name'] = $form_state['values']['machine_name'];
  $flexible_template['title'] = $form_state['values']['name'];
  $flexible_template['description'] = $form_state['values']['description'];
  unset($flexible_template['is_new']);

  $config = config('layout.flexible.' . $form_state['values']['machine_name']);
  $config->setData($flexible_template);
  $config->save();

  layout_set_flexible_tempstore($flexible_template);
  backdrop_set_message(t('Layout template saved.'));
  backdrop_theme_rebuild();
  layout_flexible_reset_caches();
  layout_reset_caches();
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['values']['machine_name'] . '/configure';
}

/**
 * Menu callback; Enable or disable a layout template.
 */
function layout_flexible_template_change_enabled_status($template, $status) {
  if (backdrop_get_token('layout-template-' . $template) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  $config = config('layout.settings');
  $data = $config->get();
  $errors = FALSE;
  
  if ($status == 'disable') {
    if (!in_array($template, $data['excluded_templates'])) {
      $data['excluded_templates'][] = $template;
    }
    // Check which layout templates are in use.
    foreach (layout_load_all() as $layout) {
      $info = layout_get_layout_template_info($layout->layout_template);
      if (in_array($layout->layout_template, $data['excluded_templates'])) {
        $errors = TRUE;
        break;
      }
    }
    if ($errors) {
      backdrop_set_message(t('The "@layout" layout template is currently in use and may not be excluded.', array('@layout' => $info['title'])), 'error');
    }
    else {
      backdrop_set_message(t('Layout template "@title" disabled.', array('@title' => $template)));
    }
  }
  else {
    $data['excluded_templates'] = array_diff($data['excluded_templates'], array($template));
    backdrop_set_message(t('Layout "@title" enabled.', array('@title' => $template)));
  }

  if (!$errors) {
    $config->setData($data);
    $config->save();
  }

  backdrop_goto('admin/structure/layouts/settings');
}

/**
 * Form to manage adding and removing rows to a flexible template.
 *
 * @param $flexible_template_id
 *   The template being edited.
 *
 * @ingroup forms
 */
function layout_flexible_template_configure_form($form, &$form_state, $flexible_template) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');

  $messages = array();
  if (!empty($flexible_template['locked'])) {
    $messages['warning'] = array(
      layout_flexible_locked_message($flexible_template),
    );
  }
  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => $messages,
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $form_state['flexible_template'] = $flexible_template;
  $flexible_template_id = $flexible_template['name'];

  $config = array(
    'is_new' => TRUE,
    'layout_template' => $flexible_template_id,
  );
  $layout = new Layout($config);
  $template = layout_get_layout_template_info($flexible_template_id);

  backdrop_set_title($template['title']);

  $form_state['flexible_template_id'] = $flexible_template_id;


  if (empty($form_state['flexible_template'])) {
    return;
  }

  $renderer = layout_create_renderer('flexible', $layout);

  $form['content'] = array(
    '#type' => 'item',
    '#id' => 'flexible-content',
  );
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $form['content']['row_positions'] = array(
    // Use 'hidden' instead of 'value' so the JS can access it.
    '#type' => 'hidden',
    '#default_value' => implode(',', array_keys($form_state['flexible_template']['rows'])),
  );

  $form['add_row'] = array(
    '#type' => 'submit',
    '#value' => t('Add row'),
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
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/row/add/select-widths';
}

/**
 * Submit handler for layout_flexible_template_configure_form() that cancels
 * in-progress changes.
 */
function layout_flexible_template_configure_form_cancel($form, &$form_state) {
  tempstore_clear('layout.flexible', $form_state['flexible_template_id']);
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler for layout_flexible_template_configure_form().
 */
function layout_flexible_template_configure_form_submit($form, &$form_state) {
    $positions = array();
  if (!empty($form_state['values']['row_positions'])) {
        $rows = array_filter(explode(',', $form_state['values']['row_positions']));
    foreach ($rows as $position) {
     $positions[$position] = $form_state['flexible_template']['rows'][$position];
    }
  }
  $form_state['flexible_template']['rows'] = $positions;
  unset($form_state['flexible_template']['locked']);

  $config = config('layout.flexible.' . $form_state['flexible_template_id']);
  $config->setData($form_state['flexible_template']);
  $config->save();

  tempstore_clear('layout.flexible', $form_state['flexible_template_id']);
  layout_flexible_reset_caches();
  layout_reset_caches();
}

/**
 * Form to select the region widths for a flexible template.
 *
 * @param $flexible_template
 *   The loaded flexible template object.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_region_style_select($form, &$form_state, $flexible_template, $original_row) {
  $new_row = ($original_row == 'add');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  $form_state['flexible_template_id'] = $flexible_template['name'];
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
      $row_style_in_use = $flexible_template['rows'][$original_row]['contains'];
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

  // We want to set the default as the first option (so at least one option
  // is always selected) but seems we cannot set #default_value for a field if
  // its in $form_state['input']. https://drupal.stackexchange.com/questions/251056/set-default-form-values-after-ajax-callback
  $default_value = key($options);
  unset($form_state['input']['region_style']);

  $form['row_settings']['region_style'] = array(
    '#title' => t('Region widths'),
    '#type' => 'radios',
    '#default_value' => $row_style_in_use,
    '#options' => $options,
  );

  $form['submit_region_style'] = array(
    '#type' => 'submit',
    '#value' => t('Choose region widths'),
    '#attributes' => array('class' => array('layout-title-button')),
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
  if (isset($flexible_template['rows'][$original_row]['in_progress'])) {
    $cancel_submit_callback = 'layout_flexible_template_region_style_select_submit';
    $cancel_ajax_callback = 'layout_ajax_form_open_dialog';
  }
  else {
    $cancel_submit_callback = 'layout_flexible_template_cancel_dialog';
    $cancel_ajax_callback = 'layout_ajax_flexible_template_cancel_dialog';
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
 * Submit handler for the cancel button on the
 * layout_flexible_template_region_style_select() form.
 */
function layout_flexible_template_cancel_dialog($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/configure';
}

/**
 * AJAX handler that cancels the layout_flexible_template_region_style_select()
 * dialog.
 */
function layout_ajax_flexible_template_cancel_dialog($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Submit handler for layout_flexible_template_region_style_select().
 */
function layout_flexible_template_region_style_select_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/row/' . $form_state['original_row'] . '/configure/' . $form_state['values']['region_style'];
}

/**
 * Form to add or edit a row on a flexible template.
 *
 * @param $flexible_template
 *   The loaded flexible template object.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 * @param $region_style
 *   The selected region style.
 *
 * @ingroup forms
 */
function layout_flexible_template_edit_row_form($form, &$form_state, $flexible_template, $original_row, $region_style = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.flexible');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['flexible_template'] = $flexible_template;
  $flexible_template_id = $flexible_template['name'];

  $form_state['flexible_template_id'] = $flexible_template_id;
  $form_state['original_row'] = $original_row;

  $row_data = array();
  if ($original_row != 'add') {
    if(isset($form_state['flexible_template']['rows'][$original_row]['in_progress'])) {
      $row_data = $form_state['flexible_template']['rows'][$original_row]['in_progress'];
    }
    else {
      $row_data = $form_state['flexible_template']['rows'][$original_row];
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

  $form_state['last_region_number'] = layout_flexible_template_get_last_region_number($form_state['flexible_template']);
  for ($i = 0; $i < $region_count; $i++) {
    $form['region_names']['region_name_' . $i] = array(
      '#type' => 'textfield',
      '#title' => t('Region') . ' ' . ($i + 1) . ' ' . t('name'),
      '#default_value' => !empty($row_data['region_names']['region_name_' . $i]) ? $row_data['region_names']['region_name_' . $i] : t('Region') . ' ' . ($form_state['last_region_number'] + $i + 1),
    );
  }

  $form['region_styles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style'),
    '#collapsed' => $original_row != 'add',
    '#collapsible' => TRUE,
  );

  $form['region_styles']['row_classes'] = array(
    '#title' => t('Additional row classes'),
    '#type' => 'textfield',
    '#default_value' => isset($row_data['classes']) ? $row_data['classes'] : '',
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
    '#type' => 'select',
    '#options' => array(
      'container_fluid' => t("Maximum width (adds the 'container-fluid' row class)"),
      'container' => t("Fixed width (adds the 'container' row class)"),
      'no_container' => t('Always full width of page (no row classes added)'),
    ),
    '#default_value' => isset($row_data['container']) ? $row_data['container'] : 'container_fluid',
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
      'layout_flexible_template_cancel_dialog',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_flexible_template_cancel_dialog',
    ),
  );

  return $form;
}

/**
 * Returns the last number of all regions named "Region X" where X is a number.
 */
function layout_flexible_template_get_last_region_number($flexible_template, $form_state = NULL) {
  $last = 0;
  if ($form_state) {
    $region_names = array_filter($form_state['values']['region_names']);
    $last = $form_state['last_region_number'];
  }
  else {
    foreach ($flexible_template['rows'] as $row) {
      foreach ($row['region_names'] as $name) {
        $region_names[] = $name;
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
  $form_state['flexible_template']['rows'][$form_state['original_row']]['in_progress'] = $old_row;

  layout_set_flexible_tempstore($form_state['flexible_template']);

  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/row/' . $form_state['original_row'] . '/select-widths';
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

  foreach ($form_state['values']['region_names'] as $key => $region_name) {
    if (empty($region_name)) {
      $form_state['values']['region_names'][$key] = t('Region') . ' ' . ($last_region_number + $key + 1);
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

  $flexible_template['rows'][$original_row] = $new_row;

  $form_state['flexible_template'] = $flexible_template;

  $form_state['ajax_rebuild_form'] = 'layout_flexible_template_configure_form';
  $form_state['ajax_rebuild_args'] = array($flexible_template);
  $form_state['ajax_update'] = array('content');
  layout_set_flexible_tempstore($flexible_template);
  layout_flexible_reset_caches();
  if (!backdrop_is_ajax()) {
    $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/configure';
  }
}

/**
 * AJAX responder to change the region count on the
 * layout_flexible_template_region_style_select() form.
 */
function layout_flexible_template_edit_region_style_ajax($form, $form_state) {
  return $form['row_settings'];
}

/**
 * AJAX responder on the submit button for
 * layout_flexible_template_edit_row_form().
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
  if ($message = layout_flexible_locked_message($form_state['flexible_template'])) {
    backdrop_set_message($message, 'warning');
  }
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
 * @param $flexible_template
 *   The loaded flexible template object.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_row($flexible_template, $original_row) {
  // Todo use token, like in block add page.

  $commands = array();
  $flexible_template_id = $flexible_template['name'];

  unset($flexible_template['rows'][$original_row]);
  layout_set_flexible_tempstore($flexible_template);

  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#flexible-row-id-' . $original_row);

    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Row "@title" removed.', array('@title' => $flexible_template['title'])));
    backdrop_goto('admin/structure/layouts/settings/flexible-template/' . $flexible_template_id . '/configure');
  }
}

/**
 * Menu callback; Delete a flexible template.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_form($form, &$form_state, $flexible_template) {
  $form_state['template'] = $flexible_template;
  $question = t('Delete flexible template @title?', array('@title' => $flexible_template['title']));
  $description = t('This action cannot be undone.');
  backdrop_set_title($question, PASS_THROUGH);

  // Check if this template is in use.
  $errors = array();
  foreach (layout_load_all() as $layout) {
    if ($layout->layout_template == $flexible_template['name']) {
      $errors[] = check_plain($layout->title);
    }
  }

  if (!empty($errors)) {
    backdrop_set_message(t('The %layout layout template is currently in use in the following layouts and may not be deleted: !list', array('%layout' => $flexible_template['title'], '!list' => theme('item_list', array('items' => $errors)))), 'error');
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
 * Submit handler for layout_flexible_template_delete_form(). Deletes a 
 * flexible template.
 */
function layout_flexible_template_delete_form_submit(&$form, &$form_state) {
  $template = $form_state['template'];

  $config = config('layout.flexible.' . $template['name']);
  $config->delete();

  backdrop_theme_rebuild();
  layout_flexible_reset_caches();
  layout_reset_caches();

  backdrop_set_message(t('The layout template has been deleted.'));
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Prepare a message and link based on if a template is updated or locked.
 *
 * @see layout_flexible_break_lock_page()
 */
function layout_flexible_locked_message($flexible_template) {
  global $user;

  $button_title = 'Save flexible template';

  $message = '';
  if (!empty($flexible_template['locked']) && $user->uid !== $flexible_template['locked']['uid']) {
    $account = user_load($flexible_template['locked']['uid']);
    $lock_user = theme('username', array('account' => $account));
    $lock_age = format_interval(REQUEST_TIME - $item->locked['updated']);
    $lock_break_url = url('admin/structure/layouts/flexible-template/' . $flexible_template['name'] . '/break-lock', array('query' => array('token' => backdrop_get_token($type . '-' . $item->name)) + backdrop_get_destination()));
    $message = t('Settings on this page are being edited by user !user, and is therefore locked from editing by others. This lock is !age old. Click here to <a href="!break">break this lock</a>.',  array('!user' => $lock_user, '!age' => $lock_age, '!break' => $lock_break_url));
  }
  elseif (!empty($flexible_template['locked']['updated'])) {
    $message = t('This form has unsaved changes. Click "@button" to make changes permanent or "Cancel" to discard changes.', array('@button' => $button_title));
  }

  return $message;
}

