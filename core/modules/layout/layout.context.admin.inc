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
    '#markup' => 'Contexts provide additional information to blocks in the layout to display. For example, information from node 5, or the user "administrator" could be made available.',
  );

  $contexts = $layout->getContexts();
  if (isset($contexts[$context_id])) {
    $handler = $contexts[$context_id];
    $form_state['handler'] = $handler;
  }
  else {
    $handler = NULL;
  }

  $all_context_info = _layout_get_all_info('layout_context');
  backdrop_sort($all_context_info, array('title'));
  $form['context'] = array(
    '#type' => 'select',
    '#title' => t('Available contexts'),
    '#options' => array(),
    '#required' => TRUE,
    '#parents' => array('context'),
    '#ajax' => array(
      'wrapper' => 'context_settings',
      'callback' => 'layout_context_ajax_style',
    ),
  );
  foreach ($all_context_info as $key => $context_info) {
    // @todo: add a "configurable" key to hook_context_info () instead?
    if (!in_array($key, array('string', 'overrides_path', 'dashboard'))) {
      $form['context']['#options'][$context_info['name']] = $context_info['title'];
      $form['context']['#default_value'] = isset($handler->plugin) ? $handler->plugin : NULL;
    }
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
    '#submit' => array('layout_context_add_form_cancel'),
    '#ajax' => array(
      'callback' => 'layout_ajax_context_cancel_dialog',
    ),
  );
  return $form;
}

/**
 * Submit handler for the cancel button on layout_context_add_form().
 */
function layout_context_add_form_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/configure';
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
 */
function layout_context_return_form($form, &$form_state) {
  $layout = $form_state['layout'];
  
  // If no context has yet been selected, there is no sub-form to display.
  if (!isset($form_state['values']['context']) && !isset($form_state['handler'])) {
    return array();
  }

  if (isset($form_state['values']['context'])) {
    $handler = layout_create_context($form_state['values']['context']);
    $handler->is_new = TRUE;
  }
  else {
    $handler = $form_state['handler'];
  }

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
    $handler = $form_state['handler'];
    $handler->formValidate($form, $form_state);
  }
}

/**
 * Submit handler for layout_context_add_form().
 *
 * Saves a context into the layout.
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
    $handler->storage = TRUE;
    $layout->setContexts($handler->name, $handler);
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
  $edit_context = $form_state['clicked_button']['#array_parents'][3];
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/context/edit/layout/' . $edit_context;
}

/**
 * Submit handler for layout_settings_form() that removes a context.
 */
function layout_settings_form_context_remove($form, &$form_state) {
  $removed_context = $form_state['clicked_button']['#array_parents'][3];
  $form_state['layout']->clearContexts($removed_context);
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

