<?php
/**
 * @file
 * Admin page callbacks for the Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_entity_admin_form($form, &$form_state, $entity_type, $bundle_arg) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');

  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => '',
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );
  $bundle = field_extract_bundle($entity_type, $bundle_arg);
  $layouts = layout_get_entity_layouts($entity_type, $bundle);
  $entity_info = entity_get_info($entity_type);

  $header = array(
    array('data' => t('Layout'), 'class' => array('layout-title')),
    array('data' => t('Template'), 'class' => array('layout-template', 'priority-low')),
    array('data' => t('Visibility conditions'), 'class' => array('layout-conditions', 'priority-low')),
    array('data' => t('Storage state'), 'class' => array('layout-status', 'priority-low')),
    array('data' => t('Operations'), 'class' => array('layout-operations')),
  );

  $rows = array();
  foreach ($layouts['all'] as $layout) {
    $operations = array(
      '#type' => 'dropbutton',
      '#links' => _layout_get_operations($layout),
    );
    foreach ($operations['#links'] as $link_key => $op) {
      $operations['#links'][$link_key]['query'] = backdrop_get_destination();
    }

    if ($layout->storage === LAYOUT_STORAGE_OVERRIDE) {
      $storage = t('Overridden');
    }
    elseif ($layout->storage === LAYOUT_STORAGE_DEFAULT) {
      $storage = t('Default (module-provided)');
    }
    else {
      $storage = t('Custom');
    }

    // Create a link to the settings page.
    $info = layout_get_layout_template_info($layout->layout_template);
    $template = l($info['title'], 'admin/structure/layouts/manage/' . $layout->name . '/configure');

    $row = array();
    $row[] = theme('layout_info', array('layout' => $layout));
    $row[] = $template;
    $row[] = theme('layout_condition_info', array('layout' => $layout));
    $row[] = $storage;
    $row[] = backdrop_render($operations);
    $class = array('layout-row');
    if ($layout->disabled) {
      $class[] = 'disabled';
    }

    $rows[] = array('data' => $row, 'class' => $class);
  }

  $form['stand_alone_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('class' => array('layout-list')),
    '#empty' => t('No layout overrides have been created for !entity_types yet.', array('!entity_types' => $entity_info['plural label'])),
    '#weight' => 1,
  );

  return $form;
}


/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_get_entity_layouts($entity_type, $bundle_name = '') {
  $all_layouts = layout_load_all();
  $matched_layouts = array();
  foreach ($all_layouts as $layout) {
    if ($path = $layout->getPath()) {
      $path_contexts = layout_context_required_by_path($path);
      foreach ($path_contexts as $path_context) {
        if (is_a($path_context, 'EntityLayoutContext') && $path_context->isA($entity_type)) {
          if (empty($layout->conditions)) {
            $matched_layouts['all'][] = $layout;
          }
          else {
            foreach ($layout->conditions as $condition) {
              if (is_a($condition, 'EntityBundleLayoutAccess')) {
                if (empty($bundle_name)) {
                  $matched_layouts['all'][] = $layout;
                  foreach ($condition->settings['bundles'] as $bundle) {
                    $matched_layouts['bundles'][$bundle][] = $layout;
                  }
                }
                elseif (in_array($bundle_name, $condition->settings['bundles'])) {
                  $matched_layouts['all'][] = $layout;
                  $matched_layouts['bundles'][$bundle_name][] = $layout;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  return $matched_layouts;

}

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_entity_admin_add_form($form, &$form_state, $entity_type, $bundle_arg, $all_types = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  backdrop_set_title(layout_entity_menu_title($entity_type, $bundle_arg, $all_types));

  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);

  $form_state['layout'] = &$layout;
  $form_state['all_types'] = $all_types;
  $form_state['entity_type'] = $entity_type;
  $form_state['bundle_type'] = field_extract_bundle($entity_type, $bundle_arg);

  $form['#tree'] = TRUE;

  $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

  $messages = array();
  if ($layout->locked) {
    $messages['warning'] = array(
      layout_locked_message($layout, 'layout'),
    );
  }

  $entity_layouts = layout_get_entity_layouts($entity_type);
  $bundle = field_extract_bundle($entity_type, $bundle_arg);
  $entity_info = entity_get_info($entity_type);
  if (!$all_types) {
    $bundle_name = '';
    if (is_object($bundle_arg) && property_exists($bundle_arg, 'name')) {
      $bundle_name = $bundle_arg->name;
    }
    elseif (is_object($bundle_arg) && property_exists($bundle_arg, 'label')) {
      $bundle_name = $bundle_arg->label;
    }
    elseif (is_string($bundle_arg)) {
      if (isset($entity_info['label'])) {
        $bundle_name = $entity_info['label'];
      }
    }
    if (isset($entity_layouts['bundles'][$bundle]) && $bundle_count = count($entity_layouts['bundles'][$bundle])) {
      $default_name = t('!bundle_name layout !count', array('!bundle_name' => $bundle_name, '!count' => $bundle_count + 1));
    }
    else {
      $default_name = t('!bundle_name layout', array('!bundle_name' => $bundle_name));
    }
  }
  else {
    if (isset($entity_layouts['all']) && $bundle_count = count($entity_layouts['all'])) {
      $default_name = t('!entity_label layout !count', array('!entity_label' => $entity_info['label'], '!count' => $bundle_count + 1));
    }
    else {
      $default_name = t('!entity_label layout', array('!entity_label' => $entity_info['label']));
    }
  }

  $form['title'] = array(
    '#title' => t('Layout name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $default_name,
    '#required' => TRUE,
    '#access' => !$layout->isDefault(),
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('title'),
      'exists' => 'layout_load',
    ),
    '#maxlength' => 128,
    '#default_value' => $layout->name,
    '#required' => TRUE,
    '#access' => !$layout->isDefault(),
  );
  $form['layout_template'] = array(
    '#title' => t('Layout template'),
    '#type' => 'radios',
    '#default_value' => $layout->layout_template,
    '#options' => array(),
    '#wrapper_attributes' => array('class' => array('clearfix', 'layout-options')),
    '#required' => TRUE,
  );
  $form['content_area'] = array(
    '#title' => t('"Main content" block'),
    '#type' => 'checkboxes',
    '#default_value' => 1,
    '#options' => array(
      'field_blocks' => t('Replace the "Main content" block with individual field blocks.'),
    ),
  );

  // Get the list of layout template options. The list needs to be rebuilt (see
  // https://github.com/backdrop/backdrop-issues/issues/984)
  $all_template_info = layout_get_layout_template_info(NULL, TRUE);

  $excluded = config_get('layout.settings', 'excluded_templates');
  foreach ($all_template_info as $template_name => $template_info) {
    if (!in_array($template_name, $excluded)) {
      $form['layout_template']['#options'][$template_name] = theme('layout_template_option', array('template_info' => $template_info));
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($layout->is_new) ? t('Save layout') : t('Create layout'),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_entity_admin_add_form_submit',
    ),
  );
  if (isset($_SESSION['layout_new_name']) || isset($layout->locked)) {
    $form['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#limit_validation_errors' => array(array('actions', 'reset')),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_reset',
      ),
    );
  }

  return $form;
}
/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_entity_admin_add_form_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  if (isset($_SESSION['layout_new_name'])) {
    unset($_SESSION['layout_new_name']);
  }

  $entity_type = $form_state['entity_type'];
  $bundle_type = $form_state['bundle_type'];

  // Build the expected path string and set the path.
  $all_info = _layout_get_all_info('layout_context');
  $entity_path_type = $all_info[$entity_type]['menu paths'][0];
  $entity_path = preg_replace('/%[a-z0-9_]+/', '%', $entity_path_type);
  $layout->setPath($entity_path);

  // Set the bundle type visibility condition.
  if (!$form_state['all_types']) {
    $handler = layout_create_access('node_type');
    $handler->settings['bundles'][] = $bundle_type;
    $layout->conditions[] = $handler;
  }

  // If this is a new layout, populate blocks from default layout.
  if (!empty($layout->is_new)) {
    $default_layout_name = path_is_admin($layout->getPath()) ? 'admin_default' : 'default';
    $default_layout = layout_load($default_layout_name);
    // Get the template being used by this layout.
    $template_info = layout_get_layout_template_info($layout->layout_template);
    foreach ($default_layout->positions as $region => $region_block_list) {
      if (array_key_exists($region, $template_info['regions'])) {
        foreach ($region_block_list as $uuid) {
          if (isset($default_layout->content[$uuid])) {
            $block = $default_layout->content[$uuid];
            $new_block = $block->getClone();
            // Only add the block from the default if required contexts are
            // present for each block.
            if ($layout->hasContexts($new_block->getRequiredContexts())) {
              $layout->positions[$region][] = $new_block->uuid;
              $layout->content[$new_block->uuid] = $new_block;
            }
          }
        }
      }
    }

    if ($form_state['values']['content_area']['field_blocks']) {
      $content_region = '';
      if (isset($template_info['regions']['content'])) {
        $content_region = 'content';
      }
      else {
        $template_regions = array_keys($template_info['regions']);
        $content_region = end($template_regions);
      }
      $fields = field_info_instances($entity_type, $bundle_type);

      foreach ($fields as $field) {
        $key = 'field:field_block:' . $field['entity_type'] . '-' . $field['field_name'];
        $field_block = layout_create_handler('block', $key);
        $build_state = array(
          'build_info' => array('args' => array($layout, $field_block, 'standard', $content_region)),
        );
        $build = backdrop_build_form('layout_block_configure_form', $build_state);
        form_execute_handlers('validate', $build, $build_state);
        form_execute_handlers('submit', $build, $build_state);
      }
      $main_content_uuid = '';
      foreach ($layout->content as $uuid => $block) {
        if ($block->module === 'system' && $block->delta === 'main') {
          $main_content_uuid = $uuid;
          break;
        }
      }
      if ($main_content_uuid) {
        $layout->removeBlock($main_content_uuid);
      }
    }

  }

  $layout->save();

  layout_clear_layout_tempstore($layout->name);
  if (!empty($layout->is_new)) {
    backdrop_set_message(t('Layout created. Blocks may now be added to this layout.'));
  }
  else {
    backdrop_set_message(t('Layout saved.'));
  }
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name;
}
