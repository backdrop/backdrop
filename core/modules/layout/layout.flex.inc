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
function layout_flexible_template_add_template($form, &$form_state, $flexible_template_id = NULL) {
  $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);
  $template_data = $form_state['template_data'];
  dpm($form_state['template_data']);

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
      'exists' => 'layout_flexible_row_name_exists',
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

  return $form;
}

function layout_flexible_template_add_template_submit($form, &$form_state) {
  $template_data = $form_state['template_data'];
  $template_data['name'] = $form_state['values']['machine_name'];
  $template_data['title'] = $form_state['values']['name'];
  $template_data['description'] = $form_state['values']['description'];
  dpm($template_data);

  $config = config('layout.flexible.' . $form_state['values']['machine_name']);
  $config->setData($template_data);
  $config->save();

  layout_set_flex_tempstore($template_data, $form_state['values']['machine_name']);
  backdrop_set_message(t('Template saved.'));
  $form_state['redirect'] = 'admin/structure/layouts/settings/' . $form_state['values']['machine_name'] . '/configure';
}

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_add_flexible_template($form, &$form_state, $flexible_template_id) {
  // $layout = layout_load('default');
  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);
  $template = layout_get_layout_template_info($flexible_template_id);
  $template['template'] = 'layout-flexible';
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
    '#id' => 'flex-content',
  );
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save layout'),
    '#submit' => array(
      'layout_add_flexible_template_submit',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
  );

  return $form;
}

/**
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_add_flexible_template_submit($form, &$form_state) {
  $config = config('layout.flexible.' . $form_state['flexible_template_id']);
  $config->setData($form_state['template_data']);
  $config->save();
  // tempstore_clear('layout.flex', $form_state['flexible_template_id']);
  // $form_state['redirect'] = 'admin/structure/layouts/settings';
}

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
function layout_flexible_template_add_row($form, &$form_state, $flexible_template_id, $original_row, $position) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  // Todo this must become stored and pulled from tempstore.
  if (!isset($form_state['template_data'])) {
    $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);
  }
  $form_state['flexible_template_id'] = $flexible_template_id;
  $form_state['original_row'] = $original_row;
  $form_state['position'] = $position;

  $options = array();
  $row_styles = layout_flexible_row_styles();
  foreach ($row_styles as $name => $row_style) {
    $options[$name] = $row_style['name'];
  }

  $form['row_style'] = array(
    '#title' => t('Select row style'),
    '#type' => 'select',
    '#default_value' => 'column_50_25_25',
    '#options' => $options,
  );
  $form['name'] = array(
    '#title' => t('Row name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => '',
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 21,
    '#machine_name' => array(
      'exists' => 'layout_flexible_row_name_exists',
    ),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#submit' => array(
      'layout_flexible_template_add_row_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_flexible_template_add_row_ajax',
    ),
  );

  return $form;
}

function layout_flexible_row_name_exists($machine_name) {
  return FALSE;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_flexible_template_add_row_submit($form, &$form_state) {
  $template_data = $form_state['template_data'];

  $flexible_template_id = $form_state['flexible_template_id'];
  $original_row = $form_state['original_row'];
  $position = $form_state['position'];

  $new_row = array(
    'name' => $form_state['values']['name'],
    'contains' => $form_state['values']['row_style'],
  );

  $newdata = layout_flexible_array_insert_key_value($original_row, $template_data['regions'], $form_state['values']['machine_name'], $new_row, $position);

  $template_data['regions'] = $newdata;
  $form_state['template_data'] = $template_data;


  $form_state['ajax_rebuild_form'] = 'layout_add_flexible_template';
  $form_state['ajax_rebuild_args'] = array($form_state['flexible_template_id']);
  $form_state['ajax_update'] = array('content');
  layout_set_flex_tempstore($template_data, $form_state['flexible_template_id']);
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
 * AJAX responder to add a new row, column or region to a flexible layout.
 */
function layout_flexible_template_add_row_ajax($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  $update_ajax = layout_ajax_form_update($form, $form_state);
  $commands = array_merge($commands, $update_ajax['#commands']);

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
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
          'contains' => 'column_50_50',
        ),
        'top' => array(
          'name' => t('Top'),
          'contains' => 'column_50_25_25',
        ),
      ),
    );
  }
  else {
    $settings = layout_get_flex_tempstore($flexible_template_id);
  }
  return $settings;
}

/**
 * Provides the default row styles.
 */
function layout_flexible_row_styles() {
  $styles = array(
    'column_50_50' => array(
      'split' => '50:50',
      'bootstrap' => '6:6',
      'name' => 'Two column 50 50',
    ),
    'column_50_25_25' => array(
      'split' => '50:25:25',
      'bootstrap' => '6:3:3',
      'name' => 'Three column 50 25 25',
    ),
  );

  return $styles;
}

/**
 * Provides the default template for new layouts.
 */
function theme_layout_flexible($variables) {
  $region_buttons = $variables['region_buttons'];
  $content = $variables['content'];
  $template_id = $variables['layout_info']['name'];
  $data = layout_flexible_template_template($template_id);
  $column_data = layout_flexible_row_styles();

  $output = '<div class="layout--boxtonian">';
  foreach ($data['regions'] as $key => $region) {
    $row_class = 'flex-row-' . $key;
    $output .= '<div class="l-header-inner container container-fluid ' . $row_class . '">';
    $output .= '<div class="layout-editor-region-title clearfix">';
    $output .= $region_buttons[$key];
    $output .= '</div>';
    $output .= '<div class="l-flex-row row">';


    // $output .= '<div class="l-flex-row">' . $region_buttons . '</div>';
    $col_info = $column_data[$region['contains']];
    $split = explode(':', $col_info['bootstrap']);
    foreach ($split as $col) {
      $output .= '<div class="l-split col-md-' . $col . '">' . $content[$key];
      $output .= '</div>';
    }
    $output .= '</div>';
    $output .= '</div>';
  }
  $output .= '</div>';
  return $output;
}

/**
 * Provides the default template for new layouts.
 */
function theme_layout_layout_flexible($variables) {
  $content = $variables['content'];
  $template_id = $variables['layout_info']['name'];
  $data = layout_flexible_template_template($template_id);
  $column_data = layout_flexible_row_styles();

  $output = '<div class="layout--boxtonian">';
  foreach ($data['regions'] as $key => $region) {
    $row_class = 'flex-row-' . $key;
    $output .= '<div class="container container-fluid ' . $row_class . '">';
    $output .= '<div class="l-flex-row row">';

    $col_info = $column_data[$region['contains']];
    $split = explode(':', $col_info['bootstrap']);
    $i = 0;
    foreach ($split as $col) {
      $output .= '<div class="l-split col-md-' . $col . '">' . $content[$key . '_' . $i];
      $output .= '</div>';
      $i++;
    }
    $output .= '</div>';
    $output .= '</div>';
  }
  $output .= '</div>';
  return $output;
}

/**
 * Store changes to a layout or menu item in the temporary store.
 *
 * @param Layout|LayoutMenuItem $item
 *   The Layout item to save into tempstore.
 * @param string $type
 *   The type of item to save. Must be either "layout" or "menu_item".
 */
function layout_set_flex_tempstore($item, $name) {
  tempstore_set('layout.flex', $name, $item, 604800);
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
function layout_get_flex_tempstore($name) {
  $caches = &backdrop_static(__FUNCTION__, array());
  if (!isset($caches[$name])) {
    if (!$item = tempstore_get('layout.flex', $name)) {
      dpm($name);
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
  dpm($cache);
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
      'template' => 'layout_layout_flexible',
      'flexible' => TRUE,
      'type' => 'layout',
      'path' => '',
      'libraries' => array('bootstrap4-gs'),
    );
    // todo: default region
    foreach ($item['regions'] as $name => $region) {
      $region_style = $styles[$region['contains']];
      // todo: just add a col_count key to layout_flexible_row_styles()?
      $col_count = count(explode(':', $region_style['split']));
      for ($i = 0; $i < $col_count; $i++) {
        $layouts[$item['name']]['regions'][$name . '_' . $i] = check_plain($region['name']) . ' ' . $i;
      }
    }
  }

  return $layouts;
}

/**
 * Reset all caches provided by Layout module.
 */
function layout_flex_reset_caches() {
  dpm(cache()->get('layout:flexible:config'));
  cache()->delete('layout:flexible:config');

  backdrop_static_reset('layout_flexible_template_load');
  backdrop_static_reset('layout_get_flex_tempstore');
}
