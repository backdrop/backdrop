<?php
module_load_include('inc', 'layout', 'layout.admin');


/**
 * Submit handler for layout_settings_form() that lets the user add a context.
 */
function layout_settings_form_context_add($form, &$form_state) {
  // Remove destination flags when adding contexts, otherwise the user will
  // be taken to the destination instead of adding a context. This only
  // affects non-JS behavior.
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/context/add';
}

/**
 * Form callback; Displays form for adding new contexts to a layout.
 *
 * @param Layout $layout
 *   The layout to which a context is being added.
 * @param int|null $context_id
 *   The ID of the context being configured. If adding a new
 *   context, no value will be passed in.
 *
 * @ingroup forms
 */
function layout_context_add_form($form, &$form_state, Layout $layout = NULL, $context_id = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $is_new_context = TRUE;
  if (isset($context_id)) {
    $is_new_context = FALSE;
  }

  $form_state['layout'] = $layout;
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
      'layout_context_add_load_context_nojs',
    ),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
  );
  $form['actions']['add'] = array(
    '#type' => 'submit',
    '#value' => $is_new_context ? t('Add context') : t('Save context'),
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
    '#submit' => array(),
    '#ajax' => array(
      'callback' => 'layout_ajax_context_cancel_dialog',
    ),
  );
  return $form;
}

/**
 * AJAX callback to update the context settings.
 */
function layout_context_ajax_style($form, $form_state) {
  return $form['context_settings'];
}


/**
 * Submit handler to select a context. Hidden when JavaScript is enabled.
 */
function layout_context_add_load_context_nojs($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Helper function to return a partial context settings form.
 *
 * This can be used to add a context to either a layout, block, or menu item,
 * based on the values in the $form_state parameter.
 */
function layout_context_return_form($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  $context_id = $form_state['context_id'];

  // If no context has yet been selected, there is no sub-form to display.
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
  $form_state['handler'] = $handler;
  $handler->form($form, $form_state);

  return $form;
}


/**
 * Validation handler for layout_context_add_form().
 */
function layout_context_add_form_validate($form, &$form_state) {
  if (isset($form_state['handler'])) {
    /* @var LayoutAccess $handler */
    $handler = $form_state['handler'];
    $handler->formValidate($form, $form_state);
  }
}

/**
 * Submit handler for layout_context_add_form().
 *
 * Saves a context into the layout, block, or menu item.
 */
function layout_context_add_form_submit($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  /* @var Layout $layout */
  $layout = $form_state['layout'];

  if (!isset($form_state['handler'])) {
    return;
  }
  /* @var LayoutAccess $handler */
  $handler = $form_state['handler'];
  $handler->formSubmit($form, $form_state);
  if (!empty($handler->is_new)) {
    $handler->is_new = FALSE;
    $layout->contexts[$handler->name] = $handler;
  }
  layout_set_layout_tempstore($layout);

  $form_state['ajax_rebuild_form'] = 'layout_settings_form';
  $form_state['ajax_rebuild_args'] = array($layout);
  $form_state['ajax_update'] = array('context_wrapper');
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/configure';
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

/**
 * AJAX handler that cancels the context edit dialog.
 */
function layout_ajax_context_cancel_dialog($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

