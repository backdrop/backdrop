<?php
/**
 * @file
 * Administrative functions for custom layout contexts.
 */

/**
 * Submit handler for layout_settings_form() that lets the user add a context
 * or relationship.
 */
function layout_settings_form_context_add($form, &$form_state) {
  // Remove destination flags when adding contexts, otherwise the user will
  // be taken to the destination instead of adding a context. This only
  // affects non-JS behavior.
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  if ($form_state['clicked_button']['#name'] === 'relationship_add_button') {
    $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/relationship/add';
  }
  else {
    $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/context/add';
  }
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
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $is_new_context = TRUE;
  if (isset($context_id)) {
    $is_new_context = FALSE;
  }

  // Ensure AJAX callbacks display the correct title by setting it here.
  $form['#title'] = $is_new_context ? t('Add context') : t('Configure context');

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
    if (!in_array($key, array('string', 'overrides_path'))) {
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
    '#name' => 'context_add_button',
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
 * Form callback; Displays form for adding new relationships to a layout.
 *
 * @param Layout $layout
 *   The layout to which a context is being added.
 * @param int|null $relationship_id
 *   The ID of the relationship being configured. If adding a new
 *   relationship, no value will be passed in.
 *
 * @ingroup forms
 */
function layout_context_relationship_add_form($form, &$form_state, Layout $layout = NULL, $relationship_id = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $contexts = $layout->getContexts();

  // Get contexts from relationships
  $form_state['relevant_info'] = layout_relationships_get_relevant_info($contexts);

  $relevant_relationships = array();
  $parent_relationship = array();

  foreach ($form_state['relevant_info'] as $rid => $info) {
    $relevant_relationships[$rid] = $info['title'];
  }

  $is_new_context = TRUE;
  if (isset($relationship_id)) {
    $is_new_context = FALSE;
  }

  // Ensure AJAX callbacks display the correct title by setting it here.
  $form['#title'] = $is_new_context ? t('Add relationship') : t('Configure relationship');

  $form_state['layout'] = $layout;
  $form_state['contexts'] = $contexts;
  if (!empty($relationship_id)) {
    $form_state['relationship_id'] = $relationship_id;
  }
  elseif (isset($form_state['values']['context'])) {
    $form_state['relationship_id'] = $form_state['values']['context'];
  }

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => 'Relationships create contexts from data provided by existing contexts, like the author from node context.',
  );

  $current_relationships = $layout->relationships;
  $form_state['relationships'] = $current_relationships;
  if (isset($current_relationships[$relationship_id])) {
    $handler = $current_relationships[$relationship_id];
    $form_state['handler'] = $handler;
  }
  else {
    $handler = NULL;
  }
  if (!empty($relevant_relationships)) {
    backdrop_sort($relevant_relationships, array('title'));
    $form['relationship'] = array(
      '#type' => 'select',
      '#title' => t('Available context relationships'),
      '#options' => $relevant_relationships,
      '#required' => TRUE,
      '#default_value' => !empty($handler) ? $handler->plugin : NULL,
      '#parents' => array('relationship'),
      '#ajax' => array(
        'wrapper' => 'context_settings',
        'callback' => 'layout_context_ajax_style',
      ),
    );
  }
  else {
    $form['no_relationship'] = array(
      '#markup' => t('No relevant relationships available'),
    );
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
  $form['load_relationship_nojs'] = array(
    '#type' => 'submit',
    '#value' => t('Load relationship'),
    '#submit' => array(
      'layout_context_add_load_context_nojs',
    ),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
  );
  $form['actions']['add'] = array(
    '#type' => 'submit',
    '#name' => 'relationship_add_button',
    '#access' => !empty($relevant_relationships),
    '#value' => $is_new_context ? t('Add relationship') : t('Save relationship'),
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
 * AJAX callback to update the context or relationship settings.
 */
function layout_context_ajax_style($form, $form_state) {
  return $form['context_settings'];
}


/**
 * Submit handler to select a context or relationship. Hidden when JavaScript
 * is enabled.
 */
function layout_context_add_load_context_nojs($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Helper function to return a partial context or relationship settings form.
 */
function layout_context_return_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  $layout = $form_state['layout'];

  // If no context or relationship has yet been selected, there is no sub-form to display.
  if (!isset($form_state['values']['context']) && !isset($form_state['values']['relationship']) && !isset($form_state['handler'])) {
    return array();
  }

  if (isset($form_state['values']['context'])) {
    $handler = layout_create_context($form_state['values']['context']);
    $handler->is_new = TRUE;
  }
  elseif (isset($form_state['values']['relationship'])) {
    list($plugin_name, $child_delta) = explode(':', $form_state['values']['relationship']);
    $handler = layout_create_handler('layout_relationship', $plugin_name);
    $handler->childDelta = $child_delta;
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
 * Saves a context or relationship into the layout.
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
    if ($form_state['clicked_button']['#name'] === 'context_add_button') {
      $layout->setContexts($handler->name, $handler);
    }
    else {
      $uuid = new Uuid();
      $key = $uuid->generate();
      $layout->relationships[$key] = $handler;
    }
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
 * Submit handler for layout_settings_form() that edits a relationship.
 */
function layout_settings_form_context_relationship_edit($form, &$form_state) {
  $edit_relationship = $form_state['clicked_button']['#array_parents'][3];
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/relationship/edit/layout/' . $edit_relationship;
}

/**
 * Submit handler for layout_settings_form() that removes a context.
 */
function layout_settings_form_context_remove($form, &$form_state) {
  $removed_context = $form_state['clicked_button']['#array_parents'][3];

  // Find all relationships to this context and remove them too.
  $contexts = $form_state['layout']->getContexts();
  $id = $contexts[$removed_context]->settings['id'];
  $relationships = $form_state['layout']->relationships;
  foreach ($relationships as $key => $relationship) {
    if ($relationship->settings['context_parent'] == 'id:' . $id) {
      unset($form_state['layout']->relationships[$key]);
    }
  }
  // Then finally remove the context.
  $form_state['layout']->clearContexts($removed_context);

  layout_set_layout_tempstore($form_state['layout']);

  $form_state['ajax_update'] = array('context_wrapper');
}

/**
 * Submit handler for layout_settings_form() that removes a relationship.
 */
function layout_settings_form_context_relationship_remove($form, &$form_state) {
  $removed_relationship = $form_state['clicked_button']['#array_parents'][3];
  unset($form_state['layout']->relationships[$removed_relationship]);
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

