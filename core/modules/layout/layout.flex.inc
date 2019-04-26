<?php
/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */


/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_add_flexible_template($form, &$form_state, $flexible_template_id = 'flextemp') {
  // $layout = layout_load('default');
  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);
  $flexible_template_defaults = layout_flexible_template_defaults();
  $flexible_template_defaults['name'] = $flexible_template_id;
  $form_state['flexible_template_id'] = $flexible_template_id;

  if (!isset($form_state['template_data'])) {
    $form_state['template_data'] = layout_flexible_template_template($flexible_template_id);
  }

  foreach ($form_state['template_data']['regions'] as $position => $info) {
    $layout->positions[$position] = array();
    $flexible_template_defaults['regions'][$position] = $info['name'];
  }

  // Don't use layout_create_renderer() because we need to pass in template
  // info.
  $renderer = new LayoutRendererFlexible($layout, $flexible_template_defaults);
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
  tempstore_clear('layout.flex', $form_state['flexible_template_id']);
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
  $form['hide_empty'] = array(
    '#title' => t('Hide element if empty'),
    '#type' => 'checkbox',
    '#default_value' => 0,
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
      'exists' => 'flexible_layout_row_name_exists',
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

function flexible_layout_row_name_exists($machine_name) {
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
    'contains' => 'three_column',
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
 * Code copied from http://eosrei.net/comment/287
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

  // Rerender our parent item links:
  // $commands[] = ajax_command_replace('.flexible-links-' . $id, flexible_layout_render_item_links($renderer, $id, $parent));

  // $commands[] = array(
    // 'command' => 'flexible_fix_firstlast',
    // 'selector' => '.' . $parent_class . '-inside',
    // 'base' => 'flexible-layout-' . $item['type'],
  // );

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_flexible_template_defaults() {
  $defaults = array(
    'type' => 'layout',
    'name' => 'flex_default',
    'version' => '1.13.x-dev',
    'backdrop' => '1.x',
    'regions' => array(
      'header' => 'Header',
      'top' => 'Top',
      'content' => 'Content',
      'bottom' => 'Bottom',
      'footer' => 'Footer',
    ),
    'default region' => 'content',
    'preview' => 'flex_default.png',
    'libraries' => array(
      'bootstrap4-gs',
    ),
    'path' => 'core/layouts/flex_default',
    'title' => 'Flex default',
    'hidden' => FALSE,
    'stylesheets' => array(
      'all' => array(
        'flex_default.css',
      ),
    ),
    'template' => 'layout-flexible',
  );
  return $defaults;
}
  


/**
 * Provides the default template for new layouts.
 */
function layout_flexible_template_template($flexible_template_id = NULL) {
  if (!$settings = layout_get_flex_tempstore($flexible_template_id)) {
    // set up a default
    $settings = array(
      'regions' => array(
        'header' => array(
          'name' => t('Header'),
          'contains' => 'two_column',
        ),
        'top' => array(
          'name' => t('Top'),
          'contains' => 'three_column',
        ),
        'content' => array(
          'name' => t('Content'),
          'contains' => 'column',
        ),
        'bottom' => array(
          'name' => t('Bottom'),
          'contains' => 'column',
        ),
        'footer' => array(
          'name' => t('Footer'),
          'contains' => 'column',
        ),
      ),
    );
  }

  return $settings;
}

/**
 * Provides the default template for new layouts.
 */
function theme_layout_flexible($variables) {
  $content = $variables['content'];
  $template_id = $variables['layout_info']['name'];
  $data = layout_flexible_template_template($template_id);

  $output = '<div class="layout--boxtonian">';
  foreach ($data['regions'] as $key => $region) {
    $row_class = 'flex-row-' . $key;
    $output .= '<div class="l-header-inner container container-fluid ' . $row_class . '">' . $content[$key] . '</div>';
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
    $item = tempstore_get('layout.flex', $name);
    $caches[$name] = $item;
  }

  return $caches[$name];
}
