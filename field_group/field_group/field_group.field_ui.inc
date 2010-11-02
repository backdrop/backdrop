<?php
// $Id$


/**
 * Menu loader; Load a group instance based on group and bundle name.
 *
 * @param $group_name
 *   The name of the group, as contained in the path.
 * @param $entity_type
 *   The name of the entity.
 * @param $bundle_name
 *   The name of the bundle, as contained in the path.
 * @param $bundle_pos
 *   The position of $bundle_name in $map.
 * @param $map
 *   The translated menu router path argument map.
 */
function field_group_menu_load($group_name, $entity_type, $bundle_name, $bundle_pos, $map) {
  // A unique group is identified by a [entity_type, bundle, group_name, view_mode] tuple.
  $query = db_select('field_group', 'fg');
  $query->fields('fg');
  $query->condition('group_name', $group_name);
  $query->condition('entity_type', $entity_type);
  $query->condition('bundle', $bundle_name);
  $group = $query->execute()->fetchObject();
  if ($group->id) {
    // @todo should happen in a proper CRUD API function
    $data = unserialize($group->data);
    unset($group->data);
    $group->label = $data['label'];
    $group->weight = $data['weight'];
    $group->children = $data['children'];
    $group->visible = $data['visible'];

    return $group;
  }
  return FALSE;
}

/**
 * Menu callback; present a form for removing a group.
 */
function field_group_delete_form($form, &$form_state, $group, $view_mode = 'form') {

  $form['#group'] = $group;
  $admin_path = _field_ui_bundle_admin_path($group->entity_type, $group->bundle);
  if ($view_mode == 'form') {
    $admin_path .= '/fields';
  }
  else {
    $admin_path .= '/display/'. $view_mode;
  }
  $form['#redirect'] = array($admin_path);
  $output = confirm_form($form,
    t('Are you sure you want to delete the group %group?', array('%group' => $group->label)),
    $admin_path,
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel'),
    'confirm'
  );
  return $output;
}

/**
 * Remove group from bundle.
 *
 * @todo we'll have to reset all view mode settings - that will be fun :)
 */
function field_group_delete_form_submit($form, &$form_state) {
  $group = $form['#group'];
  $bundle = $group->bundle;
  $entity_type = $group->entity_type;
  $mode = $form_state['build_info']['args'][1];

  $bundles = field_info_bundles();
  $bundle_label = $bundles[$entity_type][$bundle]['label'];

  db_delete('field_group')->condition('id', $group->id)->condition('mode', $mode)->execute();
  drupal_set_message(t('The group %group has been deleted from the %type content type.', array('%group' => $group->label, '%type' => $bundle_label)));
  cache_clear_all('field_groups', 'cache_field');

  // Redirect.
  $form_state['redirect'] = $form['#redirect'];
}

/**
 * Helper function to get the form parameters to use while
 * building the fields and display overview form.
 */
function field_group_field_ui_form_params($form, $display_overview) {

  $params = new stdClass();
  $params->entity_type = $form['#entity_type'];
  $params->bundle = $form['#bundle'];
  $params->admin_path = _field_ui_bundle_admin_path($params->entity_type, $params->bundle);
  $params->display_overview = $display_overview;

  $formatters = field_group_formats();

  if ($display_overview) {
    $params->region_callback = 'field_group_display_overview_row_region';
    $params->mode = $form['#view_mode'];
    $params->format_options = $formatters['display'];
  }
  else {
    $params->region_callback = 'field_group_field_overview_row_region';
    $params->mode = 'form';
    $params->format_options = $formatters['form'];
  }

  $params->groups = field_group_info_groups($params->entity_type, $params->bundle, $params->mode, TRUE);

  // Gather parenting data.
  $params->parents = array();
  foreach ($params->groups as $name => $group) {
    foreach ($group->children as $child) {
      $params->parents[$child] = $name;
    }
  }

  return $params;
}

/**
 * Function to alter the fields overview and display overview screen.
 */
function field_group_field_ui_overview_form_alter(&$form, &$form_state, $display_overview = FALSE) {

  $params = field_group_field_ui_form_params($form, $display_overview);

  if (!$display_overview) {
    // Overview form does not have this key by default.
    $form_state += array(
      'formatter_settings_edit' => NULL,
    );
    // Add AJAX wrapper.
    $form['fields']['#prefix'] = '<div id="field-display-overview-wrapper">';
    $form['fields']['#suffix'] = '</div>';
  }
  $form['#groups'] = array_keys($params->groups);

  $table = &$form['fields'];

  // Add a region for 'add_new' rows, but only when fields are
  // available and thus regions.
  if (isset($table['#regions'])) {
    $table['#regions'] += array(
      'add_new' => array('title' => '&nbsp;'),
    );
  }

  // Extend available parenting options.
  foreach ($params->groups as $name => $group) {
    $table['#parent_options'][$name] = $group->label;
  }
  $table['#parent_options']['_add_new_group'] = t('Add new group');

  // Update existing rows accordingly to the parents.
  foreach (element_children($table) as $name) {
    $table[$name]['parent_wrapper']['parent']['#options'] = $table['#parent_options'];
    $table[$name]['parent_wrapper']['parent']['#default_value'] = isset($params->parents[$name]) ? $params->parents[$name] : '';
  }
  // Update rows that are made visible (dragged/selected out of hidden).
  //if (isset($_POST['refresh_rows']) && $_POST['refresh_rows'] == $name && isset($_POST['fields'][$_POST['refresh_rows']]['type'])) {
  //  $params->parents[$name] = '';
  //  // override the #value because at this point in this context, #default_value is too late.
  //  $table[$name]['parent_wrapper']['parent']['#value'] = $params->parents[$name];
  //}

  // Create the group rows and check actions.
  foreach (array_keys($params->groups) as $name) {

    $group = & $params->groups[$name];

    // Formatter settings.

    // Check the currently selected formatter, and merge persisted values for
    // formatter settings for the group.
    // This needs to be done first, so all fields are updated before creating elements.
    if (isset($form_state['values']['fields'][$name])) {
      if (!empty($form_state['values']['fields'][$name]['format']['type'])) {
        $formatter_type = $form_state['values']['fields'][$name]['format']['type'];
      }
      elseif (!empty($form_state['input']['fields'][$name]['format']['type'])) {
        $formatter_type = $form_state['input']['fields'][$name]['format']['type'];
      }
      else {
        $formatter_type = $group->visible;
      }
      // If the region has been updated because of a format selection,
      // load the format defaults.
      if ($formatter_type != $group->visible) {
        $group->visible = $formatter_type;
        field_group_format_settings_submit($group, $form_state['values']['fields'][$name], TRUE);
      }
      // Save the group when the configuration is submitted.
      if (!empty($form_state['values'][$name .'_formatter_settings_update'])) {
        field_group_format_settings_submit($group, $form_state['values']['fields'][$name]);
      }
    }

    $settings = field_group_format_settings_form($group);

    $id = strtr($name, '_', '-');
    $js_rows_data[$id] = array('type' => 'group', 'name' => $name);
    // A group cannot be selected as its own parent.
    $parent_options = $table['#parent_options'];
    unset($parent_options[$name]);
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'field-group'), 'id' => $id),
      '#row_type' => 'group',
      '#region_callback' => $params->region_callback,
      '#js_settings' => array('rowHandler' => 'group'),
      'human_name' => array(
        '#markup' => check_plain($group->label),
        '#prefix' => '<span class="group-label">',
        '#suffix' => '</span>',
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $group->weight,
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#options' =>  $parent_options,
          '#empty_value' => '',
          '#default_value' => isset($params->parents[$name]) ? $params->parents[$name] : '',
          '#attributes' => array('class' => array('field-parent')),
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
    );

    $table[$name] += array(
      'label' => array(),
      'format' => array(
        'type' => array(
          '#type' => 'select',
          '#options' => $params->format_options,
          '#default_value' => $group->visible,
          '#attributes' => array('class' => array('field-group-type')),
        ),
      ),
    );

    $base_button = array(
      '#submit' => array('field_ui_display_overview_multistep_submit'),
      '#ajax' => array(
        'callback' => 'field_ui_display_overview_multistep_js',
        'wrapper' => 'field-display-overview-wrapper',
        'effect' => 'fade',
      ),
      '#field_name' => $name,
    );

    if ($form_state['formatter_settings_edit'] == $name) {
      $table[$name]['format']['#cell_attributes'] = array('colspan' => $display_overview ? 3 : 3);
      $table[$name]['format']['format_settings'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('field-formatter-settings-edit-form')),
        '#parents' => array('fields', $name, 'format_settings'),
        '#weight' => -5,
        'label' => array(
          '#markup' => t('Field group format:') .' <span class="formatter-name">'. $group->visible . '</span>',
        ),
        // Create a settings form where hooks can pick in.
        'settings' => $settings,
        'actions' => array(
          '#type' => 'actions',
          'save_settings' => $base_button + array(
            '#type' => 'submit',
            '#name' => $name . '_formatter_settings_update',
            '#value' => t('Update'),
            '#op' => 'update',
          ),
          'cancel_settings' => $base_button + array(
            '#type' => 'submit',
            '#name' => $name . '_formatter_settings_cancel',
            '#value' => t('Cancel'),
            '#op' => 'cancel',
            // Do not check errors for the 'Cancel' button.
            '#limit_validation_errors' => array(),
          ),
        ),
      );
      $table[$name]['#attributes']['class'][] = 'field-formatter-settings-editing';
      $table[$name]['format']['type']['#attributes']['class'] = array('element-invisible');
    }
    else {
      // After saving, the settings are updated here aswell. First we create
      // the element for the table cell.
      $table[$name]['settings_summary'] = array('#markup' => '');
      if (!empty($group->format_settings)) {
        $table[$name]['settings_summary'] = field_group_format_settings_summary($name, $group);
      }
      // Add the configure button.
      $table[$name]['settings_edit'] = $base_button + array(
        '#type' => 'image_button',
        '#name' => $name . '_group_settings_edit',
        '#src' => 'misc/configure.png',
        '#attributes' => array('class' => array('field-formatter-settings-edit'), 'alt' => t('Edit')),
        '#op' => 'edit',
        // Do not check errors for the 'Edit' button.
        '#limit_validation_errors' => array(),
        '#prefix' => '<div class="field-formatter-settings-edit-wrapper">',
        '#suffix' => '</div>',
      );
      if ($display_overview) {
        $table[$name]['settings_edit']['#suffix'] .= l(t('delete'), $params->admin_path . '/groups/' . $name . '/delete/' . $params->mode);
      }
    }

    if (!$display_overview) {
      $table[$name] += array(
        'delete' => array(
          '#markup' => l(t('delete'), $params->admin_path . '/groups/' . $name . '/delete/form'),
        ),
      );
    }

    /**
     * Special case for region.
     */
    if ($group->visible == 'region') {
      // @todo clean this up - and maybe hide the tabledrag handlers ?
      $table[$name]['format']['type']['#access'] = FALSE;
      $table[$name]['settings_summary']['#access'] = FALSE;
      $table[$name]['format']['#access'] = FALSE;
      $table[$name]['#attributes']['class'] = array('draggable', 'tabledrag-root', 'static-region', 'field-group');
      $table[$name]['#attributes']['no_striping'] = TRUE;
    }
  }

  // Additional row: add new group.
  $parent_options = $table['#parent_options'];
  unset($parent_options['_add_new_group']);
  $table['_add_new_group'] = field_group_add_row('_add_new_group', $parent_options, $params);

  $table['_add_new_group'] += array(
    'format' => array(
      'type' => array(
        '#type' => 'select',
        '#options' => $params->format_options,
        '#default_value' => 'fieldset',
      ),
    ),
    'settings_summary' => array(),
    'settings_edit' => array(),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'field_group') . '/field_group.field_ui.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'field_group') . '/field_group.field_ui.js';

  $form['#validate'][] = 'field_group_field_overview_validate';
  $form['#submit'][] = 'field_group_field_overview_submit';
}

/**
 * Returns the region to which a row in the 'Manage fields' screen belongs.
 * @param Array $row A field or field_group row
 * @return String the current region.
 */
function field_group_field_overview_row_region($row) {
  switch ($row['#row_type']) {
    case 'group':
      return 'main';
    case 'add_new_group':
      // If no input in 'label', assume the row has not been dragged out of the
      // 'add new' section.
      if (empty($row['label']['#value'])) {
        return 'add_new';
      }
      return 'main';
  }
}

/**
 * Returns the region to which a row in the 'Manage display' screen belongs.
 * @param Array $row A field or field_group row
 * @return String the current region.
 */
function field_group_display_overview_row_region($row) {
  switch ($row['#row_type']) {
    case 'group':
      return ($row['format']['type']['#value'] == 'hidden' ? 'hidden' : 'visible');
    case 'add_new_group':
      // If no input in 'label', assume the row has not been dragged out of the
      // 'add new' section.
      if (empty($row['label']['#value'])) {
        return 'add_new';
      }
      return ($row['format']['type']['#value'] == 'hidden' ? 'hidden' : 'visible');
  }
}

/**
 * Helper function to add a row in the overview forms.
 */
function field_group_add_row($name, $parent_options, $params) {
  return array(
    '#attributes' => array('class' => array('draggable', 'field-group', 'add-new')),
    '#row_type' => 'add_new_group',
    '#js_settings' => array('rowHandler' => 'group'),
    '#region_callback' => $params->region_callback,
    'label' => array(
      '#type' => 'textfield',
      '#size' => 15,
      '#description' => t('Label'),
      '#prefix' => '<div class="label-input"><div class="add-new-placeholder">' . t('Add new group') .'</div>',
      '#suffix' => '</div>',
    ),
    'weight' => array(
      '#type' => 'textfield',
      '#default_value' => field_info_max_weight($params->entity_type, $params->bundle, $params->mode) + 3,
      '#size' => 3,
      '#title_display' => 'invisible',
      '#title' => t('Weight for new group'),
      '#attributes' => array('class' => array('field-weight')),
      '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
    ),
    'parent_wrapper' => array(
      'parent' => array(
        '#type' => 'select',
        '#options' => $parent_options,
        '#empty_value' => '',
        '#attributes' => array('class' => array('field-parent')),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
        '#parents' => array('fields', $name, 'parent'),
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $name,
        '#attributes' => array('class' => array('field-name')),
      ),
    ),
    'group_name' => array(
      '#type' => 'textfield',
      // This field should stay LTR even for RTL languages.
      '#field_prefix' => '<span dir="ltr">group_',
      '#field_suffix' => '</span>&lrm;',
      '#attributes' => array('dir'=>'ltr'),
      '#size' => 15,
      '#description' => t('Group name (a-z, 0-9, _)'),
      '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
      '#cell_attributes' => array('colspan' => $params->display_overview ? 1 : 2),
    ),
  );
}

/**
 * Creates a form for field_group formatters.
 * @param Object $group The FieldGroup object.
 */
function field_group_format_settings_form(&$group) {
  $form = array();
  $form['label'] = array('#type' => 'textfield', '#title' => t('Field group label'), '#default_value' => $group->label, '#weight' => -5);
  $form += module_invoke_all('field_group_format_settings', $group);
  return $form;
}

/**
 * Submit handler to save the configuration of a fieldgroup.
 * @param Ojbect $group The group object
 * @param Array $settins Configuration settings
 */
function field_group_format_settings_submit(& $group, $settings, $format_change = FALSE) {
  // for format changes we load the defaults
  if ($format_change) {
    $group->format_settings = NULL;
    // The group will be saved by reference.
    field_group_format_settings_form($group);
  }
  else {
    $group->visible = $settings['format']['type'];
    $group->label = $settings['format_settings']['settings']['label'];
    $group->format_settings = $settings['format_settings']['settings'];
  }
  // Save the group settings.
  field_group_save($group);
  cache_clear_all('field_groups', 'cache_field');
}

/**
 * Creates a summary for the field format configuration summary.
 * @param String $group_name The name of the group
 * @param Object $group The group object
 * @return Array ready to be rendered.
 */
function field_group_format_settings_summary($group_name, $group) {
  $summary = implode('', module_invoke_all('field_group_format_summary', $group));
  return array(
    '#markup' => '<div class="field-formatter-summary">'. $summary .'</div>',
    '#cell_attributes' => array('class' => array('field-formatter-summary-cell')),
  );
}

/**
 * eof().
 */