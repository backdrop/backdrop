<?php

/**
 * @file
 * Content type editing user interface.
 */

/**
 * Page callback: Displays the content type admin overview page.
 *
 * @see node_menu()
 */
function node_overview_types() {
  $types = node_type_get_types();
  $names = node_type_get_names();
  $field_ui = module_exists('field_ui');
  $header = array(t('Name'), t('Operations'));
  $rows = array();

  foreach ($names as $key => $name) {
    $type = $types[$key];
    if (node_hook($type->type, 'form')) {
      $type_url_str = str_replace('_', '-', $type->type);
      $row = array(theme('node_admin_overview', array('name' => $name, 'type' => $type)));
      $links['edit'] = array(
        'title' => t('Edit'),
        'href' => 'admin/structure/types/manage/' . $type_url_str,
        'weight' => 0,
      );
      if ($field_ui) {
        $links['fields'] = array(
          'title' => t('Manage fields'),
          'href' => 'admin/structure/types/manage/' . $type_url_str . '/fields',
          'weight' => 5,
        );
        $links['display'] = array(
          'title' => t('Manage display'),
          'href' => 'admin/structure/types/manage/' . $type_url_str . '/display',
          'weight' => 10,
        );
      }
      if ($type->module === 'node') {
        $links['Delete'] = array(
          'title' => t('Delete'),
          'href' => 'admin/structure/types/manage/' . $type_url_str . '/delete',
          'weight' => 15,
        );
      }
      $row[] = array(
        'data' => array(
          '#type' => 'operations',
          '#links' => $links,
        ),
      );

      $rows[] = $row;
    }
  }

  $build['node_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No content types available. <a href="@link">Add content type</a>.', array('@link' => url('admin/structure/types/add'))),
  );

  return $build;
}

/**
 * Form constructor for the node type editing form.
 *
 * @param $type
 *   (optional) An object representing the node type, when editing an existing
 *   node type.
 *
 * @see node_type_form_validate()
 * @see node_type_form_submit()
 * @ingroup forms
 */
function node_type_form($form, &$form_state, $type = NULL) {
  if (!isset($type->type)) {
    // This is a new type. Populate with the defaults.
    $type = node_type_set_defaults();
  }

  // Make the type object available to implementations of hook_form_alter().
  $form['#node_type'] = $type;

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => $type->name,
    '#description' => t('The human-readable name of this content type. This text will be displayed as part of the list on the <em>Add new content</em> page. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => $type->type,
    '#maxlength' => 32,
    '#disabled' => $type->module !== 'node',
    '#machine_name' => array(
      'exists' => 'node_type_load',
    ),
    '#description' => t('A unique machine-readable name for this content type. It must only contain lowercase letters, numbers, and underscores. This name will be used for constructing the URL of the %node-add page, in which underscores will be converted into hyphens.', array(
      '%node-add' => t('Add new content'),
    )),
  );

  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $type->description,
    '#description' => t('Describe this content type. The text will be displayed on the <em>Add new content</em> page.'),
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'node') . '/js/node.types.js'),
    ),
  );

  $form['submission'] = array(
    '#type' => 'fieldset',
    '#title' => t('Submission form settings'),
    '#collapsible' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['submission']['title_label'] = array(
    '#title' => t('Title field label'),
    '#type' => 'textfield',
    '#default_value' => $type->title_label,
    '#required' => TRUE,
  );
  if (!$type->has_title) {
    // Avoid overwriting a content type that intentionally does not have a
    // title field.
    $form['submission']['title_label']['#attributes'] = array('disabled' => 'disabled');
    $form['submission']['title_label']['#description'] = t('This content type does not have a title field.');
    $form['submission']['title_label']['#required'] = FALSE;
  }
  $form['submission']['help']  = array(
    '#type' => 'textarea',
    '#title' => t('Explanation or submission guidelines'),
    '#default_value' => $type->help,
    '#description' => t('This text will be displayed at the top of the page when creating or editing content of this type.'),
  );
  $form['workflow'] = array(
    '#type' => 'fieldset',
    '#title' => t('Publishing settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['workflow']['status_default'] = array(
    '#type' => 'radios',
    '#title' => t('Published status'),
    '#options' => array(
      1 => t('Published'),
      0 => t('Unpublished'),
    ),
    '#default_value' => (int) $type->settings['status_default'],
    '#description' => t('Unpublished content is only accessible to the content creator or site administrators.')
  );

  if (module_exists('language')) {
    $form['workflow']['language'] = array(
      '#type' => 'checkbox',
      '#title' => t('Multilingual support'),
      '#default_value' => $type->settings['language'],
      '#description' => t('Add a language selection field to the editing form, allowing you to select from one of the <a href="!languages">enabled languages</a>. If disabled, new posts are saved with no defined language. Existing content will not be affected by changing this option.', array('!languages' => url('admin/config/regional/language'))),
    );
  }

  $form['workflow']['sticky'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Sticky'),
    '#options' => array(
      'sticky_enabled' => t('Show option to make posts sticky'),
      'sticky_default' => t('Make posts sticky by default'),
    ),
    '#description' => t('Sticky posts may be shown at the top of listings.'),
  );
  $form['workflow']['sticky']['sticky_enabled'] = array(
    '#default_value' => $type->settings['sticky_enabled'],
    '#parents' => array('sticky_enabled'),
    '#return_value' => 1,
  );
  $form['workflow']['sticky']['sticky_default'] = array(
    '#default_value' => $type->settings['sticky_default'],
    '#parents' => array('sticky_default'),
    '#return_value' => 1,
    '#states' => array(
      'invisible' => array(
        'input[name="sticky_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['workflow']['promote'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Promote'),
    '#options' => array(
      'promote_enabled' => t('Show option to promote posts'),
      'promote_default' => t('Promote posts by default'),
    ),
    '#description' => t('Promoted posts will often be shown on the main homepage or blog.'),
  );
  $form['workflow']['promote']['promote_enabled'] = array(
    '#default_value' => $type->settings['promote_enabled'],
    '#parents' => array('promote_enabled'),
    '#return_value' => 1,
  );
  $form['workflow']['promote']['promote_default'] = array(
    '#default_value' => $type->settings['promote_default'],
    '#parents' => array('promote_default'),
    '#return_value' => 1,
    '#states' => array(
      'invisible' => array(
        'input[name="promote_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['revision'] = array(
    '#type' => 'fieldset',
    '#title' => t('Revision settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['revision']['revision']['revision_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show option to create new revisions'),
    '#default_value' => $type->settings['revision_enabled'],
    '#parents' => array('revision_enabled'),
    '#description' => t('Revisions allow content editors to view changes over time and revert changes if needed.'),
  );
  $form['revision']['revision']['revision_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create new revision by default'),
    '#default_value' => $type->settings['revision_default'],
    '#parents' => array('revision_default'),
    '#states' => array(
      'invisible' => array(
        'input[name="revision_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['display']['node_submitted'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display author and date information'),
    '#default_value' => $type->settings['node_submitted'],
    '#description' => t('Author username and publish date will be displayed.'),
  );
  if (config_get('system.core', 'user_pictures')) {
    $form['display']['node_user_picture'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display the author picture'),
      '#default_value' => $type->settings['node_user_picture'],
      '#description' => t('Author picture will be included along with username and publish date, if provided.'),
      '#states' => array(
        // Hide the picture settings when submitted info is disabled.
        'visible' => array(
          'input[name="node_submitted"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $form['old_type'] = array(
    '#type' => 'value',
    '#value' => $type->type,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save content type'),
    '#weight' => 40,
  );

  if ($type->module === 'node' && !empty($type->type)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete content type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#validate' => array(),
      '#submit' => array('node_type_form_delete'),
    );
  }

  return $form;
}

/**
 * Helper function for teaser length choices.
 */
function _node_characters($length) {
  return ($length == 0) ? t('Unlimited') : format_plural($length, '1 character', '@count characters');
}

/**
 * Form validation handler for node_type_form().
 *
 * @see node_type_form_submit()
 */
function node_type_form_validate($form, &$form_state) {
  $type = new stdClass();
  $type->type = $form_state['values']['type'];
  $type->name = trim($form_state['values']['name']);

  // Work out what the type was before the user submitted this form
  $old_type = $form_state['values']['old_type'];

  $types = node_type_get_names();

  // 'theme' conflicts with theme_node_form().
  // '0' is invalid, since elsewhere we check it using empty().
  if (in_array($type->type, array('0', 'theme'))) {
    form_set_error('type', t("Invalid machine-readable name. Enter a name other than %invalid.", array('%invalid' => $type->type)));
  }

  $names = array_flip($types);

  if (isset($names[$type->name]) && $names[$type->name] != $old_type) {
    form_set_error('name', t('The human-readable name %name is already taken.', array('%name' => $type->name)));
  }
}

/**
 * Form submission handler for node_type_form().
 *
 * @see node_type_form_validate()
 */
function node_type_form_submit($form, &$form_state) {
  $type = $form['#node_type'];

  $type->type = $form_state['values']['type'];
  $type->name = trim($form_state['values']['name']);
  $type->old_type = isset($form_state['values']['old_type']) ? $form_state['values']['old_type'] : $type->type;

  $type->description = $form_state['values']['description'];
  $type->help = $form_state['values']['help'];
  $type->title_label = $form_state['values']['title_label'];
  // title_label is required in core; has_title will always be true, unless a
  // module alters the title field.
  $type->has_title = ($type->title_label != '');
  $type->modified = TRUE;

  // Check for any keys that match those provided by hook_node_type_load(). Any
  // values that match a settings key are saved automatically.
  foreach ($form_state['values'] as $key => $value) {
    if (array_key_exists($key, $type->settings)) {
      $type->settings[$key] = $value;
    }
  }

  $status = node_type_save($type);
  menu_rebuild();

  if ($status == SAVED_UPDATED) {
    backdrop_set_message(t('The content type %name has been updated.', array('%name' => $type->name)));
  }
  elseif ($status == SAVED_NEW) {
    node_add_body_field($type);
    backdrop_set_message(t('The content type %name has been added.', array('%name' => $type->name)));
    watchdog('node', 'Added content type %name.', array('%name' => $type->name), WATCHDOG_NOTICE, l(t('view'), 'admin/structure/types'));
  }

  $form_state['redirect'] = 'admin/structure/types';
  return;
}

/**
 * Form submission handler for node_type_form().
 */
function node_type_form_delete($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/types/manage/' . str_replace('_', '-', $form['#node_type']->type) . '/delete';
}

/**
 * Implements hook_node_type_load().
 */
function node_node_type_load(&$types) {
  foreach ($types as $type_name => $type) {
    $types[$type_name]->settings += array(
      // Basic workflow settings.
      'status_default' => TRUE,
      'promote_enabled' => TRUE,
      'promote_default' => FALSE,
      'sticky_enabled' => TRUE,
      'sticky_default' => FALSE,
      'revision_enabled' => TRUE,
      'revision_default' => FALSE,

      // Submitted information.
      'node_submitted' => TRUE,
      'node_user_picture' => TRUE,

      // Though Language module is required to change this setting, all node
      // types have a language setting, even if it is disabled by default.
      'language' => 0,

      // Enable permissions for this node type.
      // See node_permissions_get_configured_types().
      'node_permissions' => TRUE,
    );
  }
}

/**
 * Implements hook_node_type_update().
 */
function node_node_type_update($info) {
  if (!empty($info->old_type) && $info->old_type != $info->type) {
    $update_count = node_type_update_nodes($info->old_type, $info->type);

    if ($update_count) {
      backdrop_set_message(format_plural($update_count, 'Changed the content type of 1 post from %old-type to %type.', 'Changed the content type of @count posts from %old-type to %type.', array('%old-type' => $info->old_type, '%type' => $info->type)));
    }
  }
}

/**
 * Resets relevant fields of a module-defined node type to their default values.
 *
 * @param $type
 *   The node type to reset. The node type is passed back by reference with its
 *   resetted values. If there is no module-defined info for this node type,
 *   then nothing happens.
 */
function node_type_reset($type) {
  $info_array = module_invoke_all('node_info');
  if (isset($info_array[$type->orig_type])) {
    $info_array[$type->orig_type]['type'] = $type->orig_type;
    $info = node_type_set_defaults($info_array[$type->orig_type]);

    foreach ($info as $field => $value) {
      $type->$field = $value;
    }
  }
}

/**
 * Menu callback; delete a single content type.
 *
 * @param $type
 *   Content type object.
 *
 * @return
 *   Form array for delete confirmation form.
 *
 * @see node_type_delete_confirm_submit()
 * @ingroup forms
 */
function node_type_delete_confirm($form, &$form_state, $type) {
  $form['type'] = array('#type' => 'value', '#value' => $type->type);
  $form['name'] = array('#type' => 'value', '#value' => $type->name);

  $message = t('Are you sure you want to delete the content type %type?', array('%type' => $type->name));
  $caption = '';

  $num_nodes = db_query("SELECT COUNT(*) FROM {node} WHERE type = :type", array(':type' => $type->type))->fetchField();
  if ($num_nodes) {
    $caption .= '<p>' . format_plural($num_nodes, '%type is used by 1 piece of content on your site. If you remove this content type, you will not be able to edit the %type content and it may not display correctly.', '%type is used by @count pieces of content on your site. If you remove %type, you will not be able to edit the %type content and it may not display correctly.', array('%type' => $type->name)) . '</p>';
  }

  $caption .= '<p>' . t('This action cannot be undone.') . '</p>';

  return confirm_form($form, $message, 'admin/structure/types', $caption, t('Delete'));
}

/**
 * Process content type delete confirm submissions.
 *
 * @see node_type_delete_confirm()
 */                                                
function node_type_delete_confirm_submit($form, &$form_state) {
  $type_name = $form_state['values']['type'];
  node_type_delete($type_name);

  backdrop_set_message(t('The content type %name has been deleted.', array('%name' => $type_name)));
  watchdog('node', 'Deleted content type %name.', array('%name' => $type_name), WATCHDOG_NOTICE);

  menu_rebuild();

  $form_state['redirect'] = 'admin/structure/types';
  return;
}
