<?php
/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */

/**
 * Form to add a row to a flexible template.
 *
 * @param $flexible_template_id
 *   The template to which a row is being added.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 * @param $position
 *   Where the new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_settings_form($form, &$form_state, $flexible_template_id = NULL) {
  $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);
  $template_data = $form_state['template_data'];

  $form['name'] = array(
    '#title' => t('Template name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $template_data['title'],
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#default_value' => $template_data['name'],
    '#machine_name' => array(
      'exists' => 'layout_flexible_template_name_exists',
    ),
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => $template_data['description'],
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
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

function layout_flexible_template_name_exists($machine_name) {
  return layout_flexible_template_load($machine_name);
}

function layout_flexible_template_settings_form_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}


function layout_flexible_template_settings_form_submit($form, &$form_state) {
  $template_data = $form_state['template_data'];
  $template_data['name'] = $form_state['values']['machine_name'];
  $template_data['title'] = $form_state['values']['name'];
  $template_data['description'] = $form_state['values']['description'];

  $config = config('layout.flexible.' . $form_state['values']['machine_name']);
  $config->setData($template_data);
  $config->save();

  layout_set_flexible_tempstore($template_data, $form_state['values']['machine_name']);
  backdrop_set_message(t('Layout template saved.'));
  layout_flexible_reset_caches();
  layout_reset_caches();
  $form_state['redirect'] = 'admin/structure/layouts/settings/' . $form_state['values']['machine_name'] . '/configure';
}

/**
 * Render the form for layout.
 *
 * @ingroup forms
 */

/**
 * Form to manage adding and removing rows to a flexible template.
 *
 * @param $flexible_template_id
 *   The template being edited.
 *
 * @ingroup forms
 */
function layout_flexible_template_configure_form($form, &$form_state, $flexible_template_id) {
  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);
  $template = layout_get_layout_template_info($flexible_template_id);
  $template['plugin'] = array('class' => 'LayoutRendererFlexible');
  $template['regions'] = array();
  $form_state['flexible_template_id'] = $flexible_template_id;

  $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);

  if (empty($form_state['template_data'])) {
    return;
  }

  foreach ($form_state['template_data']['regions'] as $position => $info) {
    $layout->positions[$position] = array();
    $template['regions'][$position] = $info['name'];
  }

  // Don't use layout_create_renderer() because we need to pass in template
  // info.
  $renderer = new LayoutRendererFlexible($layout, $template);
  $form['#attached'] = array(
    'library' => array(
      array('system', 'backdrop.ajax'),
    ),
  );
  $form['content'] = array(
    '#type' => 'item',
    '#id' => 'flexible-content',
  );
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
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
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_flexible_template_configure_form_cancel($form, &$form_state) {
  tempstore_clear('layout.flexible', $form_state['flexible_template_id']);
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_flexible_template_configure_form_submit($form, &$form_state) {
  $config = config('layout.flexible.' . $form_state['flexible_template_id']);
  $config->setData($form_state['template_data']);
  $config->save();
  tempstore_clear('layout.flexible', $form_state['flexible_template_id']);
  layout_flexible_reset_caches();
  layout_reset_caches();
}

/**
 * Form to add a row to a flexible template.
 *
 * @param $flexible_template_id
 *   The template to which a row is being added.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 * @param $op
 *   The operation being performed on this row.
 *
 * @ingroup forms
 */
function layout_flexible_template_edit_row($form, &$form_state, $flexible_template_id, $original_row, $op) {
  $form_state['flexible_template_id'] = $flexible_template_id;
  $form_state['original_row'] = $original_row;
  $form_state['op'] = $op;

  $form['row_settings'] = array(
    '#type' => 'container',
    '#id' => 'row_settings',
    '#parents' => array('row_settings'),
  );

  if (!isset($form_state['values']['column_count']) || $form_state['values']['column_count'] == 'none' || !empty($form_state['column_style_selected'])) {
    $form['row_settings']['content'] = layout_flexible_template_edit_row_form($form['row_settings'], $form_state);
  }
  else {
    $form['row_settings']['content'] = layout_flexible_template_column_style($form['row_settings'], $form_state);
  }

  return $form;
}

/**
 * Form to add a row to a flexible template.
 *
 * @param $flexible_template_id
 *   The template to which a row is being added.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 * @param $op
 *   The operation being performed on this row.
 *
 * @ingroup forms
 */
function layout_flexible_template_edit_row_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $flexible_template_id = $form_state['flexible_template_id'];
  $original_row = $form_state['original_row'];
  $op = $form_state['op'];

  // If we're returning from the column style selection form, re-load the stored
  // form state data.
  if (isset($form_state['storage']['row_data'])) {
    $row_data = $form_state['storage']['row_data'];
  }
  // If the form is loading for the first time, get fresh data.
  elseif (!isset($form_state['template_data'])) {
    $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);
    $row_data = $form_state['template_data']['regions'][$original_row];
  }
  else {
    $row_data = $form_state['template_data']['regions'][$original_row];
  }

  // If we haven't visited the column style form yet, add defaults.
  if (!empty($form_state['values']['column_style'])) {
    $form_state['column_data']['column_style'] = $form_state['values']['column_style'];
    $form_state['column_data']['column_names'] = $form_state['values']['column_names'];
  }
  else {
    $form_state['column_data']['column_style'] = 'column_12';
    $form_state['column_data']['column_names'] = array();
  }

  $row_styles = layout_flexible_row_styles();
  $options = array('none' => t('select number of columns'));
  foreach ($row_styles as $name => $row_style) {
    $options[$row_style['column_count']] = $row_style['column_count'];
  }

  $selected_style = $row_styles[$form_state['column_data']['column_style']]['name'];

  if ($op == 'configure') {
    backdrop_set_title(t('Configure row !original_row', array('!original_row' => $row_data['name'])));
  }
  else {
    backdrop_set_title(t('Add row !op !original_row', array('!op' => $op, '!original_row' => $row_data['name'])));
  }

  $form['name'] = array(
    '#title' => t('Row name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => isset($row_data['name']) ? $row_data['name'] : '',
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#default_value' => isset($row_data['machine_name']) ? $row_data['machine_name'] : '',
    '#machine_name' => array(
      'exists' => 'layout_flexible_row_name_exists',
      'source' => array('row_settings', 'content', 'name'),
    ),
  );
  $form['column_style'] = array(
    '#type' => 'item',
    '#title' => t('Selected column style'),
    '#markup' => $selected_style,
  );
  $form['column_count'] = array(
    '#title' => t('Change column style'),
    '#type' => 'select',
    '#default_value' => 'none',
    '#options' => $options,
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_column_style_ajax',
      'wrapper' => 'row_settings',
    ),
  );
  // todo
  $form['submit_column_style'] = array(
    '#type' => 'submit',
    '#value' => t('Choose column style'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
    '#submit' => array(
      'layout_flexible_template_column_style_select_submit',
    ),
  );
  $form['row_classes'] = array(
    '#title' => t('Additional row classes'),
    '#type' => 'textfield',
    '#default_value' => isset($row_data['row_classes']) ? $row_data['row_classes'] : '',
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
  $form['element'] = array(
    '#title' => t('Row wrapper tag'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($row_data['element']) ? $row_data['element'] : '',
  );

  $form['container'] = array(
    '#title' => t('Row width behavior'),
    '#type' => 'select',
    '#options' => array(
      'container' => t('Container'),
      'container_fluid' => t('Container fluid'),
      'no_container' => t('No container'),
    ),
    '#default_value' => isset($row_data['container']) ? $row_data['container'] : 'container',
  );

  foreach (array('name', 'machine_name', 'row_classes', 'element') as $form_element) {
    if ($op != 'configure' && !isset($form_state['storage']['row_data'])) {
      $form[$form_element]['#default_value'] = '';
    }
  }

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
      'layout_flexible_template_edit_row_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_row_ajax',
    ),
  );

  return $form;
}

function layout_flexible_row_name_exists($machine_name) {
  // Use validate function;
  return FALSE;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_flexible_template_edit_row_validate($form, &$form_state) {
  $machine_name = $form_state['values']['machine_name'];
  if ($machine_name != $form_state['original_row'] && isset($form_state['template_data']['regions'][$machine_name])) {
    form_error($form['machine_name'], t('This row name already exists.'));
  }
}

/**
 * Submit handler for layout_flexible_template_column_style().
 */
function layout_flexible_template_column_style_select_submit(&$form, &$form_state) {
  $form_state['column_style_selected'] = FALSE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_flexible_template_edit_row_submit($form, &$form_state) {
  $template_data = $form_state['template_data'];

  $flexible_template_id = $form_state['flexible_template_id'];
  $original_row = $form_state['original_row'];
  $op = $form_state['op'];

  $new_row = array(
    'name' => $form_state['values']['name'],
    'contains' => $form_state['column_data']['column_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'column_names' => $form_state['column_data']['column_names'],
  );

  if ($op != 'configure') {
    $template_data['regions'] = layout_flexible_array_insert_key_value($original_row, $template_data['regions'], $form_state['values']['machine_name'], $new_row, $op);
  }
  else {
    $template_data['regions'][$original_row] = $new_row;
  }

  $form_state['template_data'] = $template_data;


  $form_state['ajax_rebuild_form'] = 'layout_flexible_template_configure_form';
  $form_state['ajax_rebuild_args'] = array($form_state['flexible_template_id']);
  $form_state['ajax_update'] = array('content');
  layout_set_flexible_tempstore($template_data, $form_state['flexible_template_id']);
  layout_flexible_reset_caches();
}

/*
 * Inserts a new key/value before/after a key in the array.
 * Code modified from http://eosrei.net/comment/287
 *
 * @param $key
 *   The key to insert.
 * @param $array
 *   An array to insert in to.
 * @param $new_key
 *   The key to insert.
 * @param $new_value
 *   An value to insert.
 *
 * @return
 *   The new array if the key exists, FALSE otherwise.
 */
function layout_flexible_array_insert_key_value($key, array &$array, $new_key, $new_value, $position) {
  if (array_key_exists($key, $array)) {
    $new = array();
    if ($position == 'before') {
      foreach ($array as $k => $value) {
        if ($k === $key) {
          $new[$new_key] = $new_value;
        }
        $new[$k] = $value;
      }
    }
    else {
      foreach ($array as $k => $value) {
        $new[$k] = $value;
        if ($k === $key) {
          $new[$new_key] = $new_value;
        }
      }
    }
    return $new;
  }
  return FALSE;
}

/**
 * Form to select column style for a flexible template.
 *
 * @param $flexible_template_id
 *   The template to which a row is being added.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 * @param $op
 *   The operation being performed on this row.
 * @param $column_count
 *   The selected column count for this row.
 *
 * @ingroup forms
 */
function layout_flexible_template_column_style($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  // Store form state values from layout_flexible_template_edit_row_form().
  $form_state['storage']['row_data'] = $form_state['values'];

  $column_count = $form_state['values']['column_count'];

  $options = array();
  $row_styles = layout_flexible_row_styles();
  foreach ($row_styles as $name => $row_style) {
    if ($row_style['column_count'] == $column_count) {
      $options[$name] = $row_style['bootstrap'];
    }
  }

  $form['column_style'] = array(
    '#title' => t('Column style'),
    '#type' => 'select',
    //'#default_value' => isset($row_data['contains']) ? $row_data['contains'] : 'column_12',
    '#options' => $options,
  );
  $form['column_names'] = array(
    '#type' => 'fieldset',
    '#title' => t('Column titles'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );
  for ($i = 0; $i < $column_count; $i++) {
    $form['column_names']['column_name_' . $i] = array(
      '#type' => 'textfield',
      '#title' => t('Column ') . $i,
      '#default_value' => !empty($row_data['column_names']['column_name_' . $i]) ? $row_data['column_names']['column_name_' . $i] : '',
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit_style'] = array(
    '#type' => 'button',
    '#name' => 'submit_style',
    '#value' => t('Save style'),
    '#attributes' => array('class' => array('layout-title-button')),
    // '#executes_submit_callback' => FALSE,
    '#limit_validation_errors' => FALSE,
    '#submit' => array(
      'layout_flexible_template_column_style_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_column_style_ajax',
      'wrapper' => 'row_settings',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_flexible_template_column_style().
 */
function layout_flexible_template_column_style_submit(&$form, &$form_state) {
  $form_state['column_style_selected'] = TRUE;
}

/**
 * AJAX responder to add a new row, column or region to a flexible layout.
 */
function layout_flexible_template_edit_column_style_ajax($form, &$form_state) {
  return $form['row_settings'];
}

/**
 * AJAX responder to add a new row, column or region to a flexible layout.
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
 * Form to delete a row from a flexible template.
 *
 * @param $flexible_template_id
 *   The template to which a row is being added.
 * @param $original_row
 *   The row above or below which a new row is being inserted.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_row($flexible_template_id, $original_row) {
  // Todo use token, like in block add page.

  $commands = array();
  $template = layout_flexible_template_template($flexible_template_id);
  unset($template['regions'][$original_row]);
  layout_set_flexible_tempstore($template, $flexible_template_id);

  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#flexible-row-id-' . $original_row);

    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Row "@title" removed.', array('@title' => $template['title'])));
    backdrop_goto('admin/structure/layouts/settings/' . $flexible_template_id . '/configure');
  }
}

/**
 * Form callback; Delete a flexible template.
 *
 * @ingroup forms
 */
function layout_flexible_template_delete_form($form, &$form_state, $flexible_template_id) {
  $template = layout_flexible_template_load($flexible_template_id);
  $form_state['template'] = $template;

  $question = t('Delete flexible template @title?', array('@title' => $template['title']));
  $text = t('Deleting the template will affect any layouts using this template.');
  $button_text = t('Delete template');

  return confirm_form($form, $question, 'admin/structure/layouts/settings', $text, $button_text);
}

/**
 * Submit handler for layout_flexible_template_delete_form(). Deletes a flexible template.
 */
function layout_flexible_template_delete_form_submit(&$form, &$form_state) {
  $template = $form_state['template'];

  $config = config('layout.flexible.' . $template['name']);
  $config->delete();

  layout_flexible_reset_caches();
  layout_reset_caches();

  backdrop_set_message(t('The layout template has been deleted.'));
  $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Provides the default template for new layouts.
 */
function layout_flexible_template_template($flexible_template_id = NULL) {
  if (!$flexible_template_id) {
    // set up a default
    $settings = array(
      'name' => '',
      'title' => '',
      'description' => '',
      'regions' => array(
        'header' => array(
          'name' => t('Header'),
          'contains' => 'column_12',
          'element' => 'header',
          'classes' => '',
          'container' => 'container',
          'column_names' => array(),
        ),
        'content' => array(
          'name' => t('Content'),
          'contains' => 'column_12',
          'element' => '',
          'classes' => '',
          'container' => 'container',
          'column_names' => array(),
        ),
        'footer' => array(
          'name' => t('Footer'),
          'contains' => 'column_12',
          'element' => 'footer',
          'classes' => '',
          'container' => 'container',
          'column_names' => array(),
        ),
      ),
    );
  }
  else {
    $settings = layout_get_flexible_tempstore($flexible_template_id);
  }
  return $settings;
}

/**
 * Provides the default row styles.
 */
function layout_flexible_row_styles() {
  $styles = array(
    'column_12' => array(
      'split' => '12:0',
      'bootstrap' => '12:0',
      'column_count' => 1,
      'name' => 'One column',
    ),
    'column_10_2' => array(
      'split' => '10:2',
      'bootstrap' => '10:2',
      'column_count' => 2,
      'name' => 'Two columns 10:2',
    ),
    'column_9_3' => array(
      'split' => '9:3',
      'bootstrap' => '9:3',
      'column_count' => 2,
      'name' => 'Two columns 9:3',
    ),
    'column_8_4' => array(
      'split' => '8:4',
      'bootstrap' => '8:4',
      'column_count' => 2,
      'name' => 'Two columns 8:4',
    ),
    'column_6_6' => array(
      'split' => '6:6',
      'bootstrap' => '6:6',
      'column_count' => 2,
      'name' => 'Two columns 6:6',
    ),
    'column_4_8' => array(
      'split' => '4:8',
      'bootstrap' => '4:8',
      'column_count' => 2,
      'name' => 'Two columns 4:8',
    ),
    'column_3_9' => array(
      'split' => '3:9',
      'bootstrap' => '3:9',
      'column_count' => 2,
      'name' => 'Two columns 3:9',
    ),
    'column_2_10' => array(
      'split' => '2:10',
      'bootstrap' => '2:10',
      'column_count' => 2,
      'name' => 'Two columns 2:10',
    ),
    'column_4_4_4' => array(
      'split' => '4:4:4',
      'bootstrap' => '4:4:4',
      'column_count' => 3,
      'name' => 'Three columns 4:4:4',
    ),
    'column_2_8_2' => array(
      'split' => '2:8:2',
      'bootstrap' => '2:8:2',
      'column_count' => 3,
      'name' => 'Three columns 2:8:2',
    ),
    'column_2_2_8' => array(
      'split' => '2:2:8',
      'bootstrap' => '2:2:8',
      'column_count' => 3,
      'name' => 'Three columns 2:2:8',
    ),
    'column_8_2_2' => array(
      'split' => '8:2:2',
      'bootstrap' => '8:2:2',
      'column_count' => 3,
      'name' => 'Three columns 8:2:2',
    ),
    'column_3_6_3' => array(
      'split' => '3:6:3',
      'bootstrap' => '3:6:3',
      'column_count' => 3,
      'name' => 'Three columns 3:6:3',
    ),
    'column_3_3_6' => array(
      'split' => '3:3:6',
      'bootstrap' => '3:3:6',
      'column_count' => 3,
      'name' => 'Three columns 3:3:6',
    ),
    'column_6_3_3' => array(
      'split' => '6:3:3',
      'bootstrap' => '6:3:3',
      'column_count' => 3,
      'name' => 'Three columns 6:3:3',
    ),
    'column_4_2_6' => array(
      'split' => '4:2:6',
      'bootstrap' => '4:2:6',
      'column_count' => 3,
      'name' => 'Three columns 4:2:6',
    ),
    'column_4_6_2' => array(
      'split' => '4:6:2',
      'bootstrap' => '4:6:2',
      'column_count' => 3,
      'name' => 'Three columns 4:6:2',
    ),
    'column_2_6_4' => array(
      'split' => '2:6:4',
      'bootstrap' => '2:6:4',
      'column_count' => 3,
      'name' => 'Three columns 2:6:4',
    ),
    'column_2_4_6' => array(
      'split' => '2:4:6',
      'bootstrap' => '2:4:6',
      'column_count' => 3,
      'name' => 'Three columns 2:4:6',
    ),
    'column_6_2_4' => array(
      'split' => '6:2:4',
      'bootstrap' => '6:2:4',
      'column_count' => 3,
      'name' => 'Three columns 6:2:4',
    ),
    'column_6_4_2' => array(
      'split' => '6:4:2',
      'bootstrap' => '6:4:2',
      'column_count' => 3,
      'name' => 'Three columns 6:4:2',
    ),
    'column_3_3_3_3' => array(
      'split' => '3:3:3:3',
      'bootstrap' => '3:3:3:3',
      'column_count' => 4,
      'name' => 'Four columns 3:3:3:3',
    ),
    'column_3_3_4_2' => array(
      'split' => '3:3:4:2',
      'bootstrap' => '3:3:4:2',
      'column_count' => 4,
      'name' => 'Four columns 3:3:4:2',
    ),
    'column_3_3_2_4' => array(
      'split' => '3:3:2:4',
      'bootstrap' => '3:3:2:4',
      'column_count' => 4,
      'name' => 'Four columns 3:3:2:4',
    ),
    'column_4_3_3_2' => array(
      'split' => '4:3:3:2',
      'bootstrap' => '4:3:3:2',
      'column_count' => 4,
      'name' => 'Four columns 4:3:3:2',
    ),
    'column_2_3_3_4' => array(
      'split' => '2:3:3:4',
      'bootstrap' => '2:3:3:4',
      'column_count' => 4,
      'name' => 'Four columns 2:3:3:4',
    ),
    'column_2_4_3_3' => array(
      'split' => '2:4:3:3',
      'bootstrap' => '2:4:3:3',
      'column_count' => 4,
      'name' => 'Four columns 2:4:3:3',
    ),
    'column_4_2_3_3' => array(
      'split' => '4:2:3:3',
      'bootstrap' => '4:2:3:3',
      'column_count' => 4,
      'name' => 'Four columns 4:2:3:3',
    ),
    'column_2_4_4_2' => array(
      'split' => '2:4:4:2',
      'bootstrap' => '2:4:4:2',
      'column_count' => 4,
      'name' => 'Four columns 2:4:4:2',
    ),
    'column_2_4_2_4' => array(
      'split' => '2:4:2:4',
      'bootstrap' => '2:4:2:4',
      'column_count' => 4,
      'name' => 'Four columns 2:4:2:4',
    ),
    'column_4_2_2_4' => array(
      'split' => '4:2:2:4',
      'bootstrap' => '4:2:2:4',
      'column_count' => 4,
      'name' => 'Four columns 4:2:2:4',
    ),
    'column_4_2_4_2' => array(
      'split' => '4:2:4:2',
      'bootstrap' => '4:2:4:2',
      'column_count' => 4,
      'name' => 'Four columns 4:2:4:2',
    ),
  );

  return $styles;
}

/**
 * Store changes to a layout or menu item in the temporary store.
 *
 * @param Layout|LayoutMenuItem $item
 *   The Layout item to save into tempstore.
 * @param string $type
 *   The type of item to save. Must be either "layout" or "menu_item".
 */
function layout_set_flexible_tempstore($item, $name) {
  tempstore_set('layout.flexible', $name, $item, 604800);
}

/**
 * Get a layout or menu item currently being edited from the tempstore.
 *
 * If a layout is not yet being edited, the layout will be loaded from
 * configuration.
 *
 * @param string $name
 *   The machine name of the Layout item to load.
 * @param string $type
 *   The type of item to load. Must be either "layout" or "menu_item".
 */
function layout_get_flexible_tempstore($name) {
  $caches = &backdrop_static(__FUNCTION__, array());
  if (!isset($caches[$name])) {
    if (!$item = tempstore_get('layout.flexible', $name)) {
      $item = layout_flexible_template_load($name);
    }
    $caches[$name] = $item;
  }

  return $caches[$name];
}

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
 * Implements layout_info().
 */
function layout_layout_info() {
  $layouts = array();
  $items = layout_flexible_template_load();
  $styles = layout_flexible_row_styles();
  foreach ($items as $item) {
    $layouts[$item['name']] = array(
      'title' => $item['title'],
      'regions' => array(),
      'default region' => '',
      'template' => 'layout--flexible',
      'flexible' => TRUE,
      'type' => 'layout',
      'path' => 'images',
      'preview' => 'flexible_template.png',
      'libraries' => array('bootstrap4-gs'),
    );
    // todo: default region
    foreach ($item['regions'] as $name => $region) {
      $region_style = $styles[$region['contains']];
      // todo: just add a col_count key to layout_flexible_row_styles()?
      $col_count = count(explode(':', $region_style['split']));
      for ($i = 0; $i < $col_count; $i++) {
        if (!empty($region['column_names']['column_name_' . $i])) {
          $column_name = $region['column_names']['column_name_' . $i];
        }
        else {
          $column_name = check_plain($region['name']) . ' ' . $i;
        }
        $layouts[$item['name']]['regions'][$name . '_' . $i] = $column_name;
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
  backdrop_static_reset('layout_get_flexible_tempstore');
}
