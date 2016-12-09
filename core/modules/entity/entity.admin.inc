<?php
/**
 * @file
 * Administration pages and forms for entity module.
 */

/**
 * Admin listing page for entity view modes.
 */
function entity_view_mode_list() {
  $custom_view_modes = config_get('entity.view_modes', 'view_modes');
  $entity_info = entity_get_info();

  $rows = array();
  foreach (array_keys($entity_info) as $entity_type) {
    if (empty($entity_info[$entity_type]['view modes'])) {
      continue;
    }

    // Sort view modes by machine name.
    ksort($entity_info[$entity_type]['view modes']);

    foreach ($entity_info[$entity_type]['view modes'] as $view_mode => $view_mode_info) {
      $rows[$entity_type . '-' . $view_mode]['label'] = check_plain($view_mode_info['label']);
      $rows[$entity_type . '-' . $view_mode]['entity'] = $entity_info[$entity_type]['label'];
      $rows[$entity_type . '-' . $view_mode]['type'] = !empty($custom_view_modes[$entity_type][$view_mode]) ? t('In configuration') : t('In code');

      $operations = array();
      if (isset($custom_view_modes[$entity_type][$view_mode])) {
        $operations['edit'] = array(
          'title' => t('Edit'),
          'href' => "admin/structure/view-modes/edit/{$entity_type}/{$view_mode}",
          'query' => array('destination' => current_path()),
        );
        $operations['delete'] = array(
          'title' => t('Delete'),
          'href' => "admin/structure/view-modes/delete/{$entity_type}/{$view_mode}",
          'query' => array('destination' => current_path()),
        );
      }
      if (!empty($operations)) {
        $rows[$entity_type . '-' . $view_mode]['operations'] = array(
          'data' => array(
            '#type' => 'dropbutton',
            '#links' => $operations,
          ),
        );
      }
    }
  }

  $build['view_modes'] = array(
    '#theme' => 'table',
    '#header' => array(
      array('data' => t('View mode')),
      array('data' => t('Entity')),
      array('data' => t('Storage state')),
      array(
        'data' => t('Operations'),
        'class' => array('class' => 'operations'),
      ),
    ),
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Form builder: edit a custom view mode.
 */
function entity_view_mode_edit_form($form, &$form_state, $entity_type = NULL, $machine_name = NULL) {
  $view_mode = entity_view_mode_load($entity_type, $machine_name);
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($view_mode['label']) ? $view_mode['label'] : '',
    '#required' => TRUE,
  );

  $entity_info = entity_get_info($entity_type);
  if ($entity_type === NULL) {

    // Get the type from form state (for ajax).
    $entity_type = !empty($form_state['values']['entity_type']) ? $form_state['values']['entity_type'] : 'node';

    // Only offer entity options with view modes.
    $options = array();
    foreach ($entity_info as $machine => $info) {
      if (!empty($info['view modes'])) {
        $options[$machine] = t($info['label']);
      }
    }
    $form['entity_type'] = array(
      '#type' => 'select',
      '#title' => t('Entity type'),
      '#options' => $options,
      '#default_value' => array_key_exists($entity_type, $options)? $entity_type : '',
      '#ajax' => array(
        'wrapper' => 'ajax-bundles',
        'callback' => 'entity_view_mode_ajax_update',
        'method' => 'replace',
        'effect' => 'fade',
       ),
    );
  }
  else {
    $form['entity_type'] = array(
      '#type' => 'item',
      '#title' => t('Entity type'),
      '#markup' => $entity_info['label'],
      '#access' => FALSE,
    );
  }

  $form['#entity_type'] = $entity_type;
  $form['#entity_info'] = $entity_info[$entity_type];

  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('label'),
      'exists' => 'entity_view_mode_exists',
    ),
    '#default_value' => $machine_name,
    '#entity_type' => $entity_type,
  );

  if (isset($machine_name)) {
    $form['old_machine_name'] = array(
      '#type' => 'value',
      '#value' => $machine_name,
    );
  }

  $bundle_options = entity_view_mode_get_possible_bundles($entity_type);
  $bundle_values = !empty($machine_name) ? array_keys(entity_view_mode_get_enabled_bundles($entity_type, $machine_name)) : array_keys($bundle_options);

  $form['enabled_bundles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable this view mode for the following types'),
    '#options' => $bundle_options,
    '#default_value' => $bundle_values,
    '#prefix' => '<div id="ajax-bundles">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ':input[name="custom_settings"]' => array('checked' => TRUE),
      ),
    ),
  );

  if (isset($machine_name)) {
    // Add some extra help text for existing view modes.
    $form['enabled_bundles']['#description'] = t('Unchecking a type that has already been configured will disable the display settings for this view mode.');
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Ajax callback to update bundle checkbox options.
 */
function entity_view_mode_ajax_update($form, $form_state) {
  return $form['enabled_bundles'];
}

/**
 * Helper function: checks if the view mode exists.
 */
function entity_view_mode_exists($machine_name, $element, $form_state) {
  $entity_info = entity_get_info($element['#entity_type']);
  return !empty($entity_info['view modes'][$machine_name]);
}

/**
 * Submit hanlder for entity_view_mode_edit_form().
 */
function entity_view_mode_edit_form_submit($form, &$form_state) {
  // Save the view mode.
  form_state_values_clean($form_state);
  $view_mode = $form_state['values'];
  entity_view_mode_save($form['#entity_type'], $view_mode);

  backdrop_set_message(t('Saved the %view-mode @entity-type view mode.', array(
    '@entity-type' => backdrop_strtolower($form['#entity_info']['label']),
    '%view-mode' => $view_mode['label'],
  )));
}

/**
 * Form builder: delete a view mode.
 */
function entity_view_mode_delete_form(array $form, $form_state, $entity_type, $machine_name) {
  $form['#entity_type'] = $entity_type;
  $form['#entity_info'] = $entity_info = entity_get_info($entity_type);
  $form['#view_mode'] = $view_mode = entity_view_mode_load($entity_type, $machine_name);

  if (empty($form['#view_mode'])) {
    backdrop_not_found();
    backdrop_exit();
  }

  $form['machine_name'] = array(
    '#type' => 'value',
    '#value' => $machine_name,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the %view-mode @entity-type view mode?', array(
      '@entity-type' => backdrop_strtolower($entity_info['label']),
      '%view-mode' => $view_mode['label'],
    )),
    'admin/structure/view-modes',
    t('Deleting a view mode will cause any content still using it to revert to the default display settings.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for entity_view_mode_delete_form().
 */
function entity_view_mode_delete_form_submit($form, &$form_state) {
  entity_view_mode_delete($form['#entity_type'], $form_state['values']['machine_name']);

  backdrop_set_message(t('Deleted the %view-mode @entity-type view mode.', array(
    '@entity-type' => backdrop_strtolower($form['#entity_info']['label']),
    '%view-mode' => $form['#view_mode']['label'],
  )));
}
