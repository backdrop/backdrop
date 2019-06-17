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

dpm($form_state['template_data']);
  foreach ($form_state['template_data']['regions'] as $position => $info) {
    $layout->positions[$position] = array();
    $template['regions'][$position] = $position;
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

  $form['content']['row_positions'] = array(
    // Use 'hidden' instead of 'value' so the JS can access it.
    '#type' => 'hidden',
    '#default_value' => implode(',', array_keys($form_state['template_data']['regions'])),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Row'),
    '#submit' => array(
      'layout_flexible_template_configure_form_submit',
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
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_flexible_template_configure_form_cancel($form, &$form_state) {
  tempstore_clear('layout.flexible', $form_state['flexible_template_id']);
  // $form_state['redirect'] = 'admin/structure/layouts/settings';
}

/**
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_flexible_template_configure_form_submit($form, &$form_state) {
    $positions = array();
  if (!empty($form_state['values']['row_positions'])) {
        $rows = array_filter(explode(',', $form_state['values']['row_positions']));
    foreach ($rows as $position) {
     $positions[$position] = $form_state['template_data']['regions'][$position];
    }
  }
  $form_state['template_data']['regions'] = $positions;

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
 *
 * @ingroup forms
 */
function layout_flexible_template_region_style_select($form, &$form_state, $flexible_template_id, $original_row) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['flexible_template_id'] = $flexible_template_id;
  $form_state['original_row'] = $original_row;

  $row_styles = layout_flexible_row_styles();
  $count_options = array();
  foreach ($row_styles as $name => $row_style) {
    $count_options[$row_style['region_count']] = $row_style['region_count'];
  }
  $form['region_count'] = array(
    '#title' => t('Number of regions'),
    '#type' => 'select',
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

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#name' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#submit' => array(
      'layout_flexible_template_edit_row_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_flexible_template_region_style_select().
 */
function layout_flexible_template_edit_row_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/row/' . $form_state['original_row'] . '/configure/' . $form_state['values']['region_style'];
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
 * @param $region_style
 *   The selected region style.
 *
 * @ingroup forms
 */
function layout_flexible_template_edit_row_form($form, &$form_state, $flexible_template_id, $original_row, $region_style = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $form_state['flexible_template_id'] = $flexible_template_id;
  $form_state['original_row'] = $original_row;
  $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);

  $row_data = $form_state['template_data']['regions'][$original_row];
dpm($row_data);
  $region_style = $region_style ? $region_style : $row_data['contains'];
  $form_state['region_style'] = $region_style;

  $row_styles = layout_flexible_row_styles();
  $selected_style = $row_styles[$region_style]['name'];
  $region_count = $row_styles[$region_style]['region_count'];

  // if ($op == 'configure') {
    // backdrop_set_title(t('Configure row !original_row', array('!original_row' => $original_row)));
  // }
  // else {
    // backdrop_set_title(t('Add row !op !original_row', array('!op' => $op, '!original_row' => $original_row)));
  // }

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

  $form['row_classes'] = array(
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

  // foreach (array('row_classes', 'element') as $form_element) {
    // if ($op != 'configure' && !$region_style) {
      // $form[$form_element]['#default_value'] = '';
    // }
  // }

  $form['region_names'] = array(
    '#type' => 'fieldset',
    '#title' => t('Region titles'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  for ($i = 0; $i < $region_count; $i++) {
    // Todo: replace uniqid() with random readable words.
    // $form_state['placeholder']['region_name_' . $i] = uniqid();
    $form['region_names']['region_name_' . $i] = array(
      '#type' => 'textfield',
      '#title' => t('Region ') . $i,
      '#default_value' => !empty($row_data['region_names']['region_name_' . $i]) ? $row_data['region_names']['region_name_' . $i] : 'change this',
    );
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
      'layout_flexible_template_edit_row_form_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_edit_row_ajax',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_flexible_template_region_style_select().
 */
function layout_flexible_template_change_region_style($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/settings/flexible-template/' . $form_state['flexible_template_id'] . '/row/' . $form_state['original_row'] . '/select-widths';
}

/**
 * Submit handler for layout_flexible_template_region_style().
 */
function layout_flexible_template_region_count_load(&$form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_flexible_template_edit_row_validate($form, &$form_state) {
  foreach ($form_state['values']['region_names'] as $key => $region_name) {
    if (empty($region_name)) {
      $form_state['values']['region_names'][$key] = 'change this';
    }
  }
  // alternative
  // array_filter($form_state['values']['region_names']);
  // array_merge($form_state['values']['region_names'], $form_state['placeholder']);
}


/**
 * Submit handler for layout_configure_region_page().
 */
function layout_flexible_template_edit_row_form_submit($form, &$form_state) {
  $template_data = $form_state['template_data'];
  $original_row = $form_state['original_row'];

  $new_row = array(
    'contains' => $form_state['region_style'],
    'element' => $form_state['values']['element'],
    'classes' => $form_state['values']['row_classes'],
    'container' => $form_state['values']['container'],
    'region_names' => $form_state['values']['region_names'],
  );

  if ($original_row == 'add-row') {
    $original_row = uniqid();
  }

  $template_data['regions'][$original_row] = $new_row;

  $form_state['template_data'] = $template_data;
dpm($form_state);

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
 * AJAX responder to add a new row, region or region to a flexible layout.
 */
function layout_flexible_template_edit_region_style_ajax($form, $form_state) {
  return $form['row_settings'];
}

/**
 * AJAX responder to add a new row, region or region to a flexible layout.
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
          'contains' => 'region_12',
          'element' => 'header',
          'classes' => '',
          'container' => 'container',
          'region_names' => array('header'),
        ),
        'content' => array(
          'contains' => 'region_12',
          'element' => '',
          'classes' => '',
          'container' => 'container',
          'region_names' => array('content'),
        ),
        'footer' => array(
          'contains' => 'region_12',
          'element' => 'footer',
          'classes' => '',
          'container' => 'container',
          'region_names' => array('footer'),
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
    'region_12' => array(
      'split' => '12:0',
      'bootstrap' => '12:0',
      'region_count' => 1,
      'name' => 'One full-width region',
    ),
    'region_10_2' => array(
      'split' => '10:2',
      'bootstrap' => '10:2',
      'region_count' => 2,
      'name' => 'Two regions 83% : 17%',
    ),
    'region_9_3' => array(
      'split' => '9:3',
      'bootstrap' => '9:3',
      'region_count' => 2,
      'name' => 'Two regions 75% : 25%',
    ),
    'region_8_4' => array(
      'split' => '8:4',
      'bootstrap' => '8:4',
      'region_count' => 2,
      'name' => 'Two regions 67% : 33%',
    ),
    'region_6_6' => array(
      'split' => '6:6',
      'bootstrap' => '6:6',
      'region_count' => 2,
      'name' => 'Two regions 50% : 50%',
    ),
    'region_4_8' => array(
      'split' => '4:8',
      'bootstrap' => '4:8',
      'region_count' => 2,
      'name' => 'Two regions 33% : 67%',
    ),
    'region_3_9' => array(
      'split' => '3:9',
      'bootstrap' => '3:9',
      'region_count' => 2,
      'name' => 'Two regions 25% : 75%',
    ),
    'region_2_10' => array(
      'split' => '2:10',
      'bootstrap' => '2:10',
      'region_count' => 2,
      'name' => 'Two regions 17% : 83%',
    ),
    'region_4_4_4' => array(
      'split' => '4:4:4',
      'bootstrap' => '4:4:4',
      'region_count' => 3,
      'name' => 'Three regions 33% : 33% : 33%',
    ),
    'region_2_8_2' => array(
      'split' => '2:8:2',
      'bootstrap' => '2:8:2',
      'region_count' => 3,
      'name' => 'Three regions 17% : 67% : 17%',
    ),
    'region_2_2_8' => array(
      'split' => '2:2:8',
      'bootstrap' => '2:2:8',
      'region_count' => 3,
      'name' => 'Three regions 17% : 17% : 67%',
    ),
    'region_8_2_2' => array(
      'split' => '8:2:2',
      'bootstrap' => '8:2:2',
      'region_count' => 3,
      'name' => 'Three regions 67% : 17% : 17%',
    ),
    'region_3_6_3' => array(
      'split' => '3:6:3',
      'bootstrap' => '3:6:3',
      'region_count' => 3,
      'name' => 'Three regions 25% : 50% : 25%',
    ),
    'region_3_3_6' => array(
      'split' => '3:3:6',
      'bootstrap' => '3:3:6',
      'region_count' => 3,
      'name' => 'Three regions 25% : 25% : 50%',
    ),
    'region_6_3_3' => array(
      'split' => '6:3:3',
      'bootstrap' => '6:3:3',
      'region_count' => 3,
      'name' => 'Three regions 50% : 25% : 25%',
    ),
    'region_4_2_6' => array(
      'split' => '4:2:6',
      'bootstrap' => '4:2:6',
      'region_count' => 3,
      'name' => 'Three regions 33% : 17% : 50%',
    ),
    'region_4_6_2' => array(
      'split' => '4:6:2',
      'bootstrap' => '4:6:2',
      'region_count' => 3,
      'name' => 'Three regions 33% : 50% : 17%',
    ),
    'region_2_6_4' => array(
      'split' => '2:6:4',
      'bootstrap' => '2:6:4',
      'region_count' => 3,
      'name' => 'Three regions 17% : 50% : 33%',
    ),
    'region_2_4_6' => array(
      'split' => '2:4:6',
      'bootstrap' => '2:4:6',
      'region_count' => 3,
      'name' => 'Three regions 17% : 33% : 50%',
    ),
    'region_6_2_4' => array(
      'split' => '6:2:4',
      'bootstrap' => '6:2:4',
      'region_count' => 3,
      'name' => 'Three regions 50% : 17% : 33%',
    ),
    'region_6_4_2' => array(
      'split' => '6:4:2',
      'bootstrap' => '6:4:2',
      'region_count' => 3,
      'name' => 'Three regions 50% : 33% : 17%',
    ),
    'region_3_3_3_3' => array(
      'split' => '3:3:3:3',
      'bootstrap' => '3:3:3:3',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 25% : 25%',
    ),
    'region_3_3_4_2' => array(
      'split' => '3:3:4:2',
      'bootstrap' => '3:3:4:2',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 33% : 17%',
    ),
    'region_3_3_2_4' => array(
      'split' => '3:3:2:4',
      'bootstrap' => '3:3:2:4',
      'region_count' => 4,
      'name' => 'Four regions 25% : 25% : 17% : 33%',
    ),
    'region_4_3_3_2' => array(
      'split' => '4:3:3:2',
      'bootstrap' => '4:3:3:2',
      'region_count' => 4,
      'name' => 'Four regions 33% : 25% : 25% : 17%',
    ),
    'region_2_3_3_4' => array(
      'split' => '2:3:3:4',
      'bootstrap' => '2:3:3:4',
      'region_count' => 4,
      'name' => 'Four regions 17% : 25% : 25% : 33%',
    ),
    'region_2_4_3_3' => array(
      'split' => '2:4:3:3',
      'bootstrap' => '2:4:3:3',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 25% : 25%',
    ),
    'region_4_2_3_3' => array(
      'split' => '4:2:3:3',
      'bootstrap' => '4:2:3:3',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 25% : 25%',
    ),
    'region_2_4_4_2' => array(
      'split' => '2:4:4:2',
      'bootstrap' => '2:4:4:2',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 33% : 17%',
    ),
    'region_2_4_2_4' => array(
      'split' => '2:4:2:4',
      'bootstrap' => '2:4:2:4',
      'region_count' => 4,
      'name' => 'Four regions 17% : 33% : 17% : 33%',
    ),
    'region_4_2_2_4' => array(
      'split' => '4:2:2:4',
      'bootstrap' => '4:2:2:4',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 17% : 33%',
    ),
    'region_4_2_4_2' => array(
      'split' => '4:2:4:2',
      'bootstrap' => '4:2:4:2',
      'region_count' => 4,
      'name' => 'Four regions 33% : 17% : 33% : 17%',
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
      'path' => 'templates',
      'preview' => 'flexible_template.png',
      'libraries' => array('bootstrap4-gs'),
    );
    // todo: default region
    foreach ($item['regions'] as $name => $region) {
      $region_style = $styles[$region['contains']];
      // todo: just add a col_count key to layout_flexible_row_styles()?
      $col_count = count(explode(':', $region_style['split']));
      for ($i = 0; $i < $col_count; $i++) {
        if (!empty($region['region_names']['region_name_' . $i])) {
          $region_name = $region['region_names']['region_name_' . $i];
        }
        else {
          $region_name = check_plain($region['name']) . ' ' . $i;
        }
        $layouts[$item['name']]['regions'][$name . '_' . $i] = $region_name;
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
