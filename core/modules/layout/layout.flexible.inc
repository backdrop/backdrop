<?php
/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
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
  $new_row = $original_row == 'add';
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['flexible_template_id'] = $flexible_template['name'];
  $form_state['original_row'] = $original_row;

  $row_styles = layout_flexible_row_styles();
  $count_options = array();
  foreach ($row_styles as $name => $row_style) {
    $count_options[$row_style['region_count']] = $row_style['region_count'];
  }
  $form['region_count'] = array(
    '#title' => t('Number of regions'),
    '#type' => 'radios',
    '#default_value' => '1',
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

  $region_count = isset($form_state['values']['region_count']) ? $form_state['values']['region_count'] : 1;

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
    '#default_value' => key($options),
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
 * Provides the default row styles.
 */
function layout_flexible_row_styles() {
  $styles = array(
    'region_12' => array(
      'split' => '100:0',
      'bootstrap' => '12:0',
      'region_count' => 1,
      'name' => 'One full-width region',
    ),
    'region_10_2' => array(
      'split' => '83:17',
      'bootstrap' => '10:2',
      'region_count' => 2,
      'name' => 'Two regions 83% : 17%',
    ),
    'region_9_3' => array(
      'split' => '75:25',
      'bootstrap' => '9:3',
      'region_count' => 2,
      'name' => 'Two regions 75% : 25%',
    ),
    'region_8_4' => array(
      'split' => '67:33',
      'bootstrap' => '8:4',
      'region_count' => 2,
      'name' => 'Two regions 67% : 33%',
    ),
    'region_6_6' => array(
      'split' => '50:50',
      'bootstrap' => '6:6',
      'region_count' => 2,
      'name' => 'Two regions 50% : 50%',
    ),
    'region_4_8' => array(
      'split' => '33:67',
      'bootstrap' => '4:8',
      'region_count' => 2,
      'name' => 'Two regions 33% : 67%',
    ),
    'region_3_9' => array(
      'split' => '25:75',
      'bootstrap' => '3:9',
      'region_count' => 2,
      'name' => 'Two regions 25% : 75%',
    ),
    'region_2_10' => array(
      'split' => '17:83',
      'bootstrap' => '2:10',
      'region_count' => 2,
      'name' => 'Two regions 17% : 83%',
    ),
    'region_4_4_4' => array(
      'split' => '33:33:33',
      'bootstrap' => '4:4:4',
      'region_count' => 3,
      'name' => 'Three regions 33% : 33% : 33%',
    ),
    'region_2_8_2' => array(
      'split' => '17:67:17',
      'bootstrap' => '2:8:2',
      'region_count' => 3,
      'name' => 'Three regions 17% : 67% : 17%',
    ),
    'region_2_2_8' => array(
      'split' => '17:17:67',
      'bootstrap' => '2:2:8',
      'region_count' => 3,
      'name' => 'Three regions 17% : 17% : 67%',
    ),
    'region_8_2_2' => array(
      'split' => '67:17:17',
      'bootstrap' => '8:2:2',
      'region_count' => 3,
      'name' => 'Three regions 67% : 17% : 17%',
    ),
    'region_3_6_3' => array(
      'split' => '25:50:25',
      'bootstrap' => '3:6:3',
      'region_count' => 3,
      'name' => 'Three regions 25% : 50% : 25%',
    ),
    'region_3_3_6' => array(
      'split' => '25:25:50',
      'bootstrap' => '3:3:6',
      'region_count' => 3,
      'name' => 'Three regions 25% : 25% : 50%',
    ),
    'region_6_3_3' => array(
      'split' => '50:25:25',
      'bootstrap' => '6:3:3',
      'region_count' => 3,
      'name' => 'Three regions 50% : 25% : 25%',
    ),
    'region_4_2_6' => array(
      'split' => '33:17:50',
      'bootstrap' => '4:2:6',
      'region_count' => 3,
      'name' => 'Three regions 33% : 17% : 50%',
    ),
    'region_4_6_2' => array(
      'split' => '33:50:17',
      'bootstrap' => '4:6:2',
      'region_count' => 3,
      'name' => 'Three regions 33% : 50% : 17%',
    ),
    'region_2_6_4' => array(
      'split' => '17:50:33',
      'bootstrap' => '2:6:4',
      'region_count' => 3,
      'name' => 'Three regions 17% : 50% : 33%',
    ),
    'region_2_4_6' => array(
      'split' => '17:33:50',
      'bootstrap' => '2:4:6',
      'region_count' => 3,
      'name' => 'Three regions 17% : 33% : 50%',
    ),
    'region_6_2_4' => array(
      'split' => '50:17:33',
      'bootstrap' => '6:2:4',
      'region_count' => 3,
      'name' => 'Three regions 50% : 17% : 33%',
    ),
    'region_6_4_2' => array(
      'split' => '50:33:17',
      'bootstrap' => '6:4:2',
      'region_count' => 3,
      'name' => 'Three regions 50% : 33% : 17%',
    ),
    'region_3_3_3_3' => array(
      'split' => '25:25:25:25',
      'bootstrap' => '3:3:3:3',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 25% : 25%',
    ),
    'region_3_3_4_2' => array(
      'split' => '25:25:33:17',
      'bootstrap' => '3:3:4:2',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 33% : 17%',
    ),
    'region_3_3_2_4' => array(
      'split' => '25:25:17:33',
      'bootstrap' => '3:3:2:4',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 17% : 33%',
    ),
    'region_4_3_3_2' => array(
      'split' => '33:25:25:17',
      'bootstrap' => '4:3:3:2',
      'region_count' => 4,
      'name' => 'Four regions 33% : 25% : 25% : 17%',
    ),
    'region_2_3_3_4' => array(
      'split' => '17:25:25:33',
      'bootstrap' => '2:3:3:4',
      'region_count' => 4,
      'name' => 'Four regions 17% : 25% : 25% : 33%',
    ),
    'region_2_4_3_3' => array(
      'split' => '17:33:25:25',
      'bootstrap' => '2:4:3:3',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 25% : 25%',
    ),
    'region_4_2_3_3' => array(
      'split' => '33:17:25:25',
      'bootstrap' => '4:2:3:3',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 25% : 25%',
    ),
    'region_2_4_4_2' => array(
      'split' => '17:33:33:17',
      'bootstrap' => '2:4:4:2',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 33% : 17%',
    ),
    'region_2_4_2_4' => array(
      'split' => '17:33:17:33',
      'bootstrap' => '2:4:2:4',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 17% : 33%',
    ),
    'region_4_2_2_4' => array(
      'split' => '33:17:17:33',
      'bootstrap' => '4:2:2:4',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 17% : 33%',
    ),
    'region_4_2_4_2' => array(
      'split' => '33:17:33:17',
      'bootstrap' => '4:2:4:2',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 33% : 17%',
    ),
  );

  return $styles;
}

/**
 * Store changes to a flexible layout template in the temporary store.
 *
 * @param $flexible_template
 *   The loaded flexible template object.
 */
function layout_set_flexible_tempstore($flexible_template) {
  tempstore_set('layout.flexible', $flexible_template['name'], $flexible_template, 604800);
}

/**
 * Get a flexible template currently being edited from the tempstore.
 *
 * If a template is not yet being edited, the template will be loaded from
 * configuration.
 *
 * @param string $flexible_template_id
 *   The machine name of the flexible template to load.
 */
function layout_flexible_tempstore_load($flexible_template_id = NULL) {
  if (!$flexible_template_id) {
    return layout_flexible_template_template_default();
  }

  $caches = &backdrop_static(__FUNCTION__, array());
  if (!isset($caches[$flexible_template_id])) {
    if (!$item = tempstore_get('layout.flexible', $flexible_template_id)) {
      $item = layout_flexible_template_load($flexible_template_id);
    }
    $caches[$flexible_template_id] = $item;
  }

  return $caches[$flexible_template_id];
}

/**
 * Load an individual flexible template.
 *
 * @param string $flexible_template_id
 *   The machine name of the template to load.
 *
 * @return Array
 */
function layout_flexible_template_load($flexible_template_id = NULL) {
  $configs = &backdrop_static(__FUNCTION__, array());

  if (empty($configs)) {
    $cache = cache()->get('layout:flexible:config');
    if ($cache && $cache->data) {
      $configs = $cache->data;
    }
  }

  if (empty($configs)) {
    $config_names = config_get_names_with_prefix('layout.flexible.');
    foreach ($config_names as $config_file) {
      $config = config($config_file);
      $data = $config->get();
      $configs[$data['name']] = $data;
    }

    cache()->set('layout:flexible:config', $configs);
  }
  if ($flexible_template_id) {
    if (isset($configs[$flexible_template_id])) {
      return $configs[$flexible_template_id];
    }
    else {
      return FALSE;
    }
  }
  else {
    return $configs;
  }
}

/**
 * Provides the default template for new flexible templates.
 */
function layout_flexible_template_template_default() {
  $settings = array(
    'name' => '',
    'title' => '',
    'description' => '',
    'rows' => array(
      'header' => array(
        'contains' => 'region_12',
        'element' => 'header',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Header')),
      ),
      'content' => array(
        'contains' => 'region_12',
        'element' => '',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Content')),
      ),
      'footer' => array(
        'contains' => 'region_12',
        'element' => 'footer',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Footer')),
      ),
    ),
  );

  return $settings;
}

/**
 * Implements hook_layout_info().
 */
function layout_layout_info() {
  $layouts = array();
  $flexible_templates = layout_flexible_template_load();
  $styles = layout_flexible_row_styles();
  foreach ($flexible_templates as $flexible_template) {
    $layouts[$flexible_template['name']] = array(
      'title' => $flexible_template['title'],
      'regions' => array(),
      'default region' => '',
      'template' => 'layout--flexible',
      'flexible' => TRUE,
      'type' => 'layout',
      'path' => 'templates',
      'preview' => 'flexible_template.png',
      'libraries' => array('bootstrap4-gs'),
    );
    foreach ($flexible_template['rows'] as $name => $region) {
      $region_style = $styles[$region['contains']];
      for ($i = 0; $i < $region_style['region_count']; $i++) {
        if (!empty($region['region_names']['region_name_' . $i])) {
          $region_name = $region['region_names']['region_name_' . $i];
        }
        else {
          $region_name = $name . ' ' . $i;
        }
        $layouts[$flexible_template['name']]['regions'][$name . '_' . $i] = $region_name;
      }
    }
  }

  return $layouts;
}

/**
 * Reset all caches provided by Layout module.
 */
// todo: merger into layout_flexible_reset_caches().
function layout_flexible_reset_caches() {
  cache()->delete('layout:flexible:config');

  backdrop_static_reset('layout_flexible_template_load');
  backdrop_static_reset('layout_flexible_tempstore_load');
}
