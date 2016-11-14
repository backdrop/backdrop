<?php

/**
 * List all custom view modes.
 */
function entity_view_mode_list() {
  $custom_view_modes = config_get('entity.view_modes', 'view_modes');
  $entity_info = entity_get_info();

  $build = array(
    '#prefix' => '<div id="view-modes-accordion">',
    '#suffix' => '</div>',
    '#attached' => array(
      'library' => array(
        //array('system', 'ui.accordion'),
      ),
      'js' => array(
        //backdrop_get_path('module', 'entity_view_mode') . '/entity_view_mode.admin.js',
      ),
    ),
  );

  foreach (array_keys($entity_info) as $entity_type) {
    if (empty($entity_info[$entity_type]['view modes'])) {
      continue;
    }

    $build[$entity_type] = array(
      '#type' => 'fieldset',
      '#title' => $entity_info[$entity_type]['label'],
      '#collapsible' => TRUE,
    );

    // Sort view modes by machine name.
    ksort($entity_info[$entity_type]['view modes']);

    $rows = array();
    foreach ($entity_info[$entity_type]['view modes'] as $view_mode => $view_mode_info) {
      $rows[$view_mode]['label'] = check_plain($view_mode_info['label']);
      $rows[$view_mode]['type'] = !empty($custom_view_modes[$entity_type][$view_mode]) ? t('In configuration') : t('In code');
      $rows[$view_mode]['custom_settings'] = !empty($view_mode_info['custom settings']) ? t('Yes') : t('No');

      $operations = array();
      if (isset($custom_view_modes[$entity_type][$view_mode])) {
        $operations['edit'] = array(
          'title' => t('Edit'),
          'href' => "admin/config/system/view-modes/edit/{$entity_type}/{$view_mode}",
        );
        $operations['delete'] = array(
          'title' => t('Delete'),
          'href' => "admin/config/system/view-modes/delete/{$entity_type}/{$view_mode}",
        );
      }
      if (!empty($operations)) {
        $rows[$view_mode]['operations'] = array(
          'data' => array(
            '#theme' => 'links',
            '#links' => $operations,
            '#attributes' => array('class' => array('links', 'inline')),
          ),
        );
      }
      else {
        $rows[$view_mode]['operations'] = t('None (view mode locked)');
      }
    }

    $rows['_add_new'][] = array(
      'data' => l(t('Add new view mode'), "admin/config/system/view-modes/add/{$entity_type}"),
      'colspan' => 4,
    );

    $build[$entity_type]['view_modes'] = array(
      '#theme' => 'table',
      '#header' => array(
        t('View mode'),
        t('Type'),
        t('Custom settings'),
        t('Operations'),
      ),
      '#rows' => $rows,
    );
  }

  return $build;
}

/**
 * Form builder; edit a custom view mode.
 */
function entity_view_mode_edit_form($form, &$form_state, $entity_type, $machine_name = NULL) {
  $form['#entity_type'] = $entity_type;
  $form['#entity_info'] = $entity_info = entity_get_info($entity_type);
  $view_mode = entity_view_mode_load($entity_type, $machine_name);

  if (empty($entity_info['view modes']) || (isset($machine_name) && empty($view_mode))) {
    backdrop_not_found();
    backdrop_exit();
  }

  $form['entity_type'] = array(
    '#type' => 'item',
    '#title' => t('Entity type'),
    '#markup' => $entity_info['label'],
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($view_mode['label']) ? $view_mode['label'] : '',
    '#required' => TRUE,
  );

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

  $form['custom_settings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use custom display settings'),
    '#description' => t('Unchecking this will delete all the display settings for this view mode.'),
    '#default_value' => isset($entity_info['view modes'][$machine_name]['custom settings']) ? $entity_info['view modes'][$machine_name]['custom settings'] : TRUE,
  );

  $bundle_options = entity_view_mode_get_possible_bundles($entity_type);
  $bundle_values = !empty($machine_name) ? array_keys(entity_view_mode_get_enabled_bundles($entity_type, $machine_name)) : array_keys($bundle_options);

  $form['enabled_bundles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable this view mode for the following types'),
    '#description' => t('Unchecking a type that has already been configured will delete the display settings for this view mode.'),
    '#options' => $bundle_options,
    '#default_value' => $bundle_values,
    '#states' => array(
      'visible' => array(
        ':input[name="custom_settings"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function entity_view_mode_exists($machine_name, $element, $form_state) {
  $entity_info = entity_get_info($element['#entity_type']);
  return !empty($entity_info['view modes'][$machine_name]);
}

function entity_view_mode_edit_form_submit($form, &$form_state) {
  // Save the view mode.
  form_state_values_clean($form_state);
  $view_mode = $form_state['values'];
  entity_view_mode_save($form['#entity_type'], $view_mode);

  backdrop_set_message(t('Saved the %view-mode @entity-type view mode.', array(
    '@entity-type' => backdrop_strtolower($form['#entity_info']['label']),
    '%view-mode' => $view_mode['label'],
  )));
  $form_state['redirect'] = 'admin/config/system/view-modes';
}

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
    'admin/config/system/view-modes',
    t('Deleting a view mode will cause any output still requesting to use that view mode to use the default display settings.'),
    t('Delete'),
    t('Cancel')
  );
}

function entity_view_mode_delete_form_submit($form, &$form_state) {
  entity_view_mode_delete($form['#entity_type'], $form_state['values']['machine_name']);

  backdrop_set_message(t('Deleted the %view-mode @entity-type view mode.', array(
    '@entity-type' => backdrop_strtolower($form['#entity_info']['label']),
    '%view-mode' => $form['#view_mode']['label'],
  )));
  $form_state['redirect'] = 'admin/config/system/view-modes';
}
