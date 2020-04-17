<?php
module_load_include('inc', 'layout', 'layout.admin');


/**
 * Submit handler for layout_settings_form() that lets the user add a condition.
 */
function layout_settings_form_context_add($form, &$form_state) {
  // Remove destination flags when adding conditions, otherwise the user will
  // be taken to the destination instead of adding a condition. This only
  // affects non-JS behavior.
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/context/add';
}

/**
 * Form callback; Displays form for adding new conditions to a layout or block.
 *
 * @param Layout $layout
 *   The layout to which a condition is being added.
 * @param Block|null $block
 *   If applying a condition to a particular block, the block instance.
 * @param LayoutMenuItem|null $menu_item
 *   If applying a condition to an menu item, the menu item object.
 * @param int|null $condition_id
 *   The integer ID of the condition being configured. If adding a new
 *   condition, no value will be passed in. Note that this value may be a
 *   FALSE-like value, such as 0, indicating the first condition is being
 *   configured, which is different than a NULL value indicating a new
 *   condition is being added.
 *
 * @ingroup forms
 */
function layout_context_add_form($form, &$form_state, Layout $layout = NULL, Block $block = NULL, LayoutMenuItem $menu_item = NULL, $context_id = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  $form_state['menu_item'] = $menu_item;
  $form_state['context_id'] = $context_id;

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => 'help',
  );

  $handler = isset($layout->contexts[$context_id]) ? $layout->contexts[$context_id] : NULL;

  $all_context_info = _layout_get_all_info('layout_context');
  backdrop_sort($all_context_info, array('title'));
  $form['context'] = array(
    '#type' => 'select',
    '#title' => t('Contexts'),
    '#options' => array(),
    '#required' => TRUE,
    '#default_value' => isset($handler->plugin) ? $handler->plugin : NULL,
    '#parents' => array('context'),
    '#ajax' => array(
      'wrapper' => 'context_settings',
      'callback' => 'layout_context_ajax_style',
    ),
  );
  foreach ($all_context_info as $context_info) {
    $form['context']['#options'][$context_info['name']] = $context_info['title'];
  }

  $form['context_settings'] = array(
    '#type' => 'container',
    '#id' => 'context_settings',
    '#parents' => array('context_settings'),
  );
  $form['context_settings']['content'] = layout_context_return_form($form['context_settings'], $form_state);
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['load_context_nojs'] = array(
    '#type' => 'submit',
    '#value' => t('Load context'),
    '#submit' => array(
      'layout_context_add_load_condition_nojs',
    ),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
  );
  $form['actions']['add'] = array(
    '#type' => 'submit',
    '#value' => 'kk',//$is_new_condition ? t('Add visibility condition') : t('Save visibility condition'),
    '#submit' => array(
      'layout_context_add_form_submit',
    ),
    '#validate' => array(
      'layout_context_add_form_validate',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_save_dialog',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#limit_validation_errors' => array(),
    '#submit' => array(
      'layout_context_add_form_cancel',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_context_cancel_dialog',
    ),
  );
  return $form;
}


/**
 * AJAX callback to update the condition settings.
 */
function layout_context_ajax_style($form, $form_state) {
  return $form['context_settings'];
}


/**
 * Submit handler to select a condition. Hidden when JavaScript is enabled.
 */
function layout_context_add_load_condition_nojs($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Helper function to return a partial condition settings form.
 *
 * This can be used to add a condition to either a layout, block, or menu item,
 * based on the values in the $form_state parameter.
 */
function layout_context_return_form($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutMenuItem $menu_item */
  $menu_item = $form_state['menu_item'];
  $context_id = $form_state['context_id'];

  // If no condition has yet been selected, there is no sub-form to display.
  if (!isset($form_state['values']['context'])) {
    return array();
  }
  else {
    $handler_id = $form_state['values']['context'];
  }

  if (isset($context_id)) {
      $handler = layout_create_context($context_id);
  }
  else {
    $handler = layout_create_context($handler_id);
    $handler->is_new = TRUE;
  }

  $form['#title'] = layout_condition_edit_title($layout, $block, $menu_item, $handler_id);
  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  $form_state['menu_item'] = $menu_item;
  $form_state['handler'] = $handler;
  $handler->form($form, $form_state);

  return $form;
}


/**
 * Validation handler for layout_condition_add_form().
 */
function layout_context_add_form_validate($form, &$form_state) {
  if (isset($form_state['handler'])) {
    /* @var LayoutAccess $handler */
    $handler = $form_state['handler'];
    $handler->formValidate($form, $form_state);
  }
}

/**
 * Submit handler for layout_condition_add_form().
 *
 * Saves a condition into the layout, block, or menu item.
 */
function layout_context_add_form_submit($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutMenuItem $menu_item */
  $menu_item = $form_state['menu_item'];

  if (!isset($form_state['handler'])) {
    return;
  }
  /* @var LayoutAccess $handler */
  $handler = $form_state['handler'];
  $handler->formSubmit($form, $form_state);
  if (!empty($handler->is_new)) {
    $handler->is_new = FALSE;
    if ($menu_item) {
      $menu_item->conditions[] = $handler;
    }
    elseif ($block) {
      $block->conditions[] = $handler;
    }
    else {
      $layout->contexts[] = $handler;
    }
  }
  if ($menu_item) {
    layout_set_layout_tempstore($menu_item, 'menu_item');
  }
  else {
    layout_set_layout_tempstore($layout);
  }

  // Update the portion of the page for menu item access.
  if ($menu_item) {
    $form_state['ajax_rebuild_form'] = 'layout_menu_item_form';
    $form_state['ajax_rebuild_args'] = array($menu_item);
    $form_state['ajax_update'] = array('conditions');
    $form_state['redirect'] = 'admin/structure/layouts/menu/' . $menu_item->name;
  }
  // Blocks return to the dialog/page for editing that block.
  elseif ($block) {
    $renderer_name = isset($layout->in_progress['renderer_name']) ? $layout->in_progress['renderer_name'] : 'editor';
    $renderer = layout_create_renderer($renderer_name, $layout);
    $form_state['redirect'] = $renderer->getUrl('configure-block', $block->uuid);
  }
  // Set the portion of the page to be updated for layouts.
  else {
    $form_state['ajax_rebuild_form'] = 'layout_settings_form';
    $form_state['ajax_rebuild_args'] = array($layout);
    $form_state['ajax_update'] = array('context_wrapper');
    $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/configure';
  }
}

/**
 * Submit handler for layout_settings_form() that edits a context.
 */
function layout_settings_form_context_edit($form, &$form_state) {
  $edit_context = $form_state['clicked_button']['#array_parents'][2];
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/context/edit/layout/' . $edit_context;
}

/**
 * Submit handler for layout_settings_form() that removes a context.
 */
function layout_settings_form_context_remove($form, &$form_state) {
  $removed_context = $form_state['clicked_button']['#array_parents'][3];
  unset($form_state['layout']->contexts[$removed_context]);
  layout_set_layout_tempstore($form_state['layout']);

  $form_state['ajax_update'] = array('context_wrapper');
}

