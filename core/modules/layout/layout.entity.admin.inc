<?php
/**
 * @file
 * Provides a user interface for managing layouts for entity bundles.
 */

/**
 * Render the settings form for listing entity layouts.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 * @param string $entity_type
 *   The entity type such as "node", "user", "taxonomy_term", etc.
 * @param string|stdClass $bundle_arg
 *   The bundle as a string or object. The object may be a TaxonomyVocabulary
 *   or stdClass (in the case of a node type).
 *
 * @return array
 *   The built form array.
 *
 * @ingroup forms
 */
function layout_entity_admin_form(array $form, array &$form_state, $entity_type, $bundle_arg) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

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
  if ($entity_info['label'] == 'Node') {
    $entity_info['label'] = 'Content';
  }

  $header = array(
    array('data' => t('Layout'), 'class' => array(
      'layout-title',
    )),
    array('data' => t('Template'), 'class' => array(
      'layout-template',
      'priority-low',
    )),
    array('data' => t('Conditions'), 'class' => array(
      'layout-conditions',
      'priority-low',
    )),
    array('data' => t('Operations'), 'class' => array(
      'layout-operations',
    )),
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

    // Create a link to the settings page.
    $info = layout_get_layout_template_info($layout->layout_template);
    $template = l($info['title'], 'admin/structure/layouts/manage/' . $layout->name . '/configure');

    $row = array();
    $row[] = theme('layout_info', array('layout' => $layout));
    $row[] = $template;
    $row[] = theme('layout_condition_info', array('layout' => $layout));
    $row[] = backdrop_render($operations);
    $class = array('layout-row');
    if ($layout->disabled) {
      $class[] = 'disabled';
    }

    $rows[] = array('data' => $row, 'class' => $class);
  }

  $bundle_label = _layout_entity_bundle_label($entity_type, $bundle_arg);
  $form['stand_alone_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('class' => array('layout-list')),
    '#empty' => t('No layout overrides have been created for @bundle pages yet. All pages for @bundle will display using the <a href="@default_layout_href">Default layout</a>.', array(
      '@bundle' => $bundle_label,
      '@default_layout_href' => '/admin/structure/layouts/manage/default',
    )),
    '#weight' => 1,
  );

  return $form;
}


/**
 * Returns layouts with visibility conditions matching an entity type or bundle.
 *
 * @param string $entity_type
 *   The entity type.
 * @param string $bundle_name
 *   The entity bundle name.
 *
 * @return array
 *   Associative array of matched layouts.
 *   Array keys:
 *   - all: An array of all layouts that apply to this entity type, as well as
*       layouts with visibility conditions matching $bundle_name if provided.
 *       layouts without bundle visibility conditions.
 *   - bundles: An array of layouts with visibility conditions for this bundle
 *       type if the $bundle_name parameter is provided.
 */
function layout_get_entity_layouts($entity_type, $bundle_name = '') {
  $all_layouts = layout_load_all();
  $matched_layouts = array('all' => array(), 'bundles' => array());
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
 * Render the form for adding a layout for an entity.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 * @param string $entity_type
 *   The entity type such as "node", "user", "taxonomy_term", etc.
 * @param string|stdClass $bundle_arg
 *   The bundle as a string or object. The object may be a TaxonomyVocabulary
 *   or stdClass (in the case of a node type).
 *
 * @return array
 *   The built form array.
 *
 * @ingroup forms
 */
function layout_entity_admin_add_form(array $form, array &$form_state, $entity_type, $bundle_arg) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');
  backdrop_set_title(layout_entity_menu_title($entity_type, $bundle_arg));

  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);

  $form_state['layout'] = &$layout;
  $form_state['entity_type'] = $entity_type;
  $form_state['bundle_type'] = field_extract_bundle($entity_type, $bundle_arg);

  $form['#tree'] = TRUE;

  $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

  $entity_layouts = layout_get_entity_layouts($entity_type);
  $bundle = field_extract_bundle($entity_type, $bundle_arg);
  $bundle_label = _layout_entity_bundle_label($entity_type, $bundle_arg);
  if (isset($entity_layouts['bundles'][$bundle]) && $bundle_count = count($entity_layouts['bundles'][$bundle])) {
    $default_name = t('@bundle_label layout @count', array(
      '@bundle_label' => $bundle_label,
      '@count' => $bundle_count + 1,
    ));
  }
  else {
    $default_name = t('@bundle_label layout', array(
      '@bundle_label' => $bundle_label,
    ));
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
    '#default_value' => layout_load('default')->layout_template,
    '#options' => array(),
    '#wrapper_attributes' => array(
      'class' => array('clearfix', 'layout-options'),
    ),
    '#required' => TRUE,
  );

  $fields = field_info_instances($entity_type, $form_state['bundle_type']);
  $field_blocks = array();
  foreach ($fields as $key => $field) {
    $field_blocks[$key] = $field['label'];
  }
  $field_blocks += array('main' => t('Main content block'));

  $first_field = reset($fields);
  $form['content_area'] = array(
    '#title' => t('Field blocks'),
    '#type' => 'radios',
    '#default_value' => 'main_content',
    '#options' => array(
      'main_content' => t('Use the "Main content" block to render fields'),
      'field_blocks' => t('Select individual field blocks'),
    ),
    '#description' => empty($first_field) ? t('There are no fields attached to this entity.') : '',
    '#disabled' => empty($first_field),
  );
  $form['field_blocks'] = array(
    '#title' => t('Available field blocks'),
    '#type' => 'checkboxes',
    '#default_value' => array('main'),
    '#options' => $field_blocks,
    '#states' => array(
      'visible' => array(
        ':input[name="content_area"]' => array('value' => 'field_blocks'),
      ),
    ),
    '#indentation' => 1,
  );

  // Set the new layout weight to be one less than the lowest weight for this
  // path. This ensures that the new layout takes effect, regardless of other
  // conditions that might be in place at the same path.
  $lowest_weight = count($entity_layouts['all']);
  foreach ($entity_layouts['all'] as $entity_layout) {
    /* @var Layout $entity_layout */
    if ($entity_layout->weight < $lowest_weight) {
      $lowest_weight = $entity_layout->weight;
    }
  }
  $form['weight'] = array(
    '#type' => 'hidden',
    '#value' => $lowest_weight - 1,
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
 * Validate handler for the layout_entity_admin_add_form().
 */
function layout_entity_admin_add_form_validate($form, &$form_state) {
  if ($form_state['values']['content_area'] == 'field_blocks' && empty(array_filter($form_state['values']['field_blocks']))) {
    form_set_error('field_blocks', t('At least one block should be placed.'));
  }
}

/**
 * Submit handler for layout_entity_admin_add_form().
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
  $layout->weight = $form_state['values']['weight'];

  // Set the bundle type visibility condition if the entity has bundles.
  $entity_info = entity_get_info($entity_type);
  $single_bundle = (count($entity_info['bundles']) == 1) && isset($entity_info['bundles'][$entity_type]);
  if (!$single_bundle) {
    $key = $entity_type . '_' . $entity_info['entity keys']['bundle'];
    $handler = layout_create_access($key);
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

    if ($form_state['values']['content_area'] == 'field_blocks') {
      $fields = field_info_instances($entity_type, $bundle_type);
      $first_field = reset($fields);
      if (!empty($first_field)) {
        $content_region = '';
        if (isset($template_info['regions']['content'])) {
          $content_region = 'content';
        }
        else {
          $template_regions = array_keys($template_info['regions']);
          $content_region = end($template_regions);
        }

        foreach ($fields as $field) {
          if ($form_state['values']['field_blocks'][$field['field_name']]) {
            $key = 'field:field_block:' . $field['entity_type'] . '-' . $field['field_name'];
            $field_block = layout_create_handler('block', $key);
            $build_state = array(
              'build_info' => array('args' => array(
                $layout,
                $field_block,
                'standard',
                $content_region,
              )),
            );
            $build = backdrop_build_form('layout_block_configure_form', $build_state);
            form_execute_handlers('validate', $build, $build_state);
            form_execute_handlers('submit', $build, $build_state);
          }
        }
        if (!$form_state['values']['field_blocks']['main']) {
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
    }

  }

  layout_set_layout_tempstore($layout);

  layout_locked_message($layout, 'layout');

  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name;
}

/**
 * Get a bundle's label.
 *
 * This function is a temporary measure until there is a central way to get the
 * label of an entity type's bundles.
 *
 * @param string $entity_type
 *   The entity type such as "node", "user", "taxonomy_term", etc.
 * @param string|stdClass $bundle_arg
 *   The bundle as a string or object. The object may be a TaxonomyVocabulary
 *   or stdClass (in the case of a node type).
 *
 * @return string
 *   The label of the given entity bundle.
 *
 * @private
 */
function _layout_entity_bundle_label($entity_type, $bundle_arg) {
  $entity_info = entity_get_info($entity_type);
  $bundle_label = t('Unknown');
  if (is_object($bundle_arg) && property_exists($bundle_arg, 'label')) {
    $bundle_label = $bundle_arg->label;
  }
  elseif (is_object($bundle_arg) && property_exists($bundle_arg, 'name')) {
    $bundle_label = $bundle_arg->name;
  }
  // If unable to find a bundle label, fallback to the entity label.
  elseif (is_string($bundle_arg)) {
    if (isset($entity_info['label'])) {
      $bundle_label = $entity_info['label'];
    }
  }
  return $bundle_label;
}
