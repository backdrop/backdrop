<?php
/**
 * @file
 * File type editing user interface.
 */

/**
 * Displays the file type admin overview page.
 */
function file_list_types_page() {
  $file_entity_info = entity_get_info('file');
  $types = file_type_get_types();
  $names = file_type_get_names();
  $field_ui = module_exists('field_ui') && user_access('administer fields');
  $header = array(t('Name'), t('Status'), t('Operations'));
  $rows = array();
  $weight = 0;

  // Sort the list of content types by name instead of machine name.
  natcasesort($names);

  foreach ($names as $key => $type) {
    $type = $types[$key];
    $weight++;
    $row = array(
      array(
        'data' => theme('file_type_overview',
          array(
            'name' => $type->name,
            'description' => $type->description,
          )
        ),
      ),
    );

    if ($type->disabled) {
      $storage = t('Disabled');
    }
    elseif ($type->storage === FILE_TYPE_STORAGE_OVERRIDE) {
      $storage = t('Overridden');
    }
    elseif ($type->storage === FILE_TYPE_STORAGE_DEFAULT) {
      $storage = t('Default (module-provided)');
    }
    else {
      $storage = t('Custom');
    }
    $row[] = $storage;

    $path = isset($file_entity_info['bundles'][$type->type]['admin']['real path']) ? $file_entity_info['bundles'][$type->type]['admin']['real path'] : NULL;

    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => $path,
      'weight' => 0,
    );
    if ($field_ui) {
      $links['fields'] = array(
        'title' => t('Manage fields'),
        'href' => $path . '/fields',
        'weight' => 5,
      );
      $links['display'] = array(
        'title' => t('Manage display'),
        'href' => $path . '/display',
        'weight' => 10,
      );
    }
    $links['file-display'] = array(
      'title' => t('Manage file display'),
      'href' => $path . '/file-display',
      'weight' => 10,
    );
    if ($type->disabled) {
      $links['enable'] = array(
        'title' => t('Enable'),
        'href' => $path . '/enable',
        'weight' => 10,
      );
    }
    else {
      $links['disable'] = array(
        'title' => t('Disable'),
        'href' => $path . '/disable',
        'weight' => 10,
      );
    }
    if ($type->storage == FILE_TYPE_STORAGE_OVERRIDE) {
      $links['revert'] = array(
        'title' => t('Revert'),
        'href' => $path . '/revert',
        'weight' => 20,
      );
    }
    if ($type->storage == FILE_TYPE_STORAGE_NORMAL) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => $path . '/delete',
        'weight' => 25,
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

  $build['file_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No file types available. <a href="@link">Add file type</a>.', array('@link' => url('admin/structure/types/add'))),
  );

  return $build;
}

/**
 * Form constructor for the file type settings form.
 *
 * @param object $type
 *   The file type.
 *
 * @see file_file_type_form_validate()
 * @see file_file_type_form_submit()
 */
function file_type_form($form, &$form_state, $type = NULL) {
  if (!isset($type->type)) {
    // This is a new type.
    $type = array(
      'type' => '',
      'name' => '',
      'description' => '',
      'module' => 'file',
      'mimetypes' => array(),
    );
    $type = (object) $type;
  }
  $form['#file_type'] = $type;

  $form['module'] = array(
    '#type' => 'hidden',
    '#value' => $type->module,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('This is the human readable name of the file type.'),
    '#required' => TRUE,
    '#default_value' => $type->name,
  );

  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => $type->type,
    '#maxlength' => 255,
    '#disabled' => (bool) $type->type,
    '#machine_name' => array(
      'exists' => 'file_type_load',
      'source' => array('name'),
    ),
    '#description' => t('A unique machine-readable name for this file type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('This is the description of the file type.'),
    '#default_value' => $type->description,
  );

  $form['mimetypes'] = array(
    '#type' => 'textarea',
    '#title' => t('Media types'),
    '#description' => t("This is the list of media types (MIME types) which will be aassociated with this file type. When files with these media types are uploaded, Backdrop will identify the file type based on its media type. You should not use the same media types in more than one file type. Enter one media type per line. <br/><a href='@media_types_info'>More information about media types</a>.", array('@media_types_info' => 'https://en.wikipedia.org/wiki/Media_type')),
    '#default_value' => implode("\n", $type->mimetypes),
  );

  include_once BACKDROP_ROOT . '/core/includes/file.mimetypes.inc';
  $mimetypes = file_mimetype_mapping();

  $form['mimetype_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available media types'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['mimetype_mapping']['mapping'] = array(
    '#theme' => 'item_list',
    '#items' => $mimetypes['mimetypes'],
  );

  $form['actions'] = array('#type' => 'actions');

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if (!empty($type->type)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }

  return $form;
}

/**
 * Form validation handler for file_file_type_form().
 *
 * @see file_file_type_form_submit()
 */
function file_type_form_validate($form, &$form_state) {
  include_once BACKDROP_ROOT . '/core/includes/file.mimetypes.inc';
  $mimetype_mapping = file_mimetype_mapping();

  $valid_mimetypes = $mimetype_mapping['mimetypes'];
  $submitted_mimetypes = array_filter(array_map('trim', explode("\n", $form_state['values']['mimetypes'])));

  $invalid_mimetypes = array();
  foreach ($submitted_mimetypes as $mimetype) {
    if (!file_match_mimetypes($mimetype, $valid_mimetypes)) {
      $invalid_mimetypes[] = $mimetype;
    }
  }

  foreach ($invalid_mimetypes as $mimetype) {
    form_set_error('mimetypes', t('The mimetype %mimetype is not a valid mimetype.', array('%mimetype' => $mimetype)));
  }
}

/**
 * Form submission handler for file_type_form().
 *
 * @see file_type_form_validate()
 */
function file_type_form_submit($form, &$form_state) {
  if (!empty($form['#file_type']->type)) {
    $type = file_type_load($form['#file_type']->type);
    if ($type->storage = FILE_TYPE_STORAGE_DEFAULT) {
      $type->storage = FILE_TYPE_STORAGE_OVERRIDE;
    }
  }
  else {
    $type = (object) array(
      'type' => $form_state['values']['type'],
    );
    $type->is_new = TRUE;
    $type->storage = FILE_TYPE_STORAGE_NORMAL;
  }
  if ($form_state['values']['op'] == t('Delete')) {
    $form_state['redirect'] = 'admin/structure/file-types/manage/' . $type->type . '/delete';
    return;
  }
  $type->name = $form_state['values']['name'];
  $type->module = $form_state['values']['module'];
  $type->description = $form_state['values']['description'];
  $type->mimetypes = array_filter(array_map('trim', explode("\n", $form_state['values']['mimetypes'])));

  file_type_save($type);

  backdrop_set_message(t('The file type %type has been updated.', array('%type' => $type->name)));
  $form_state['redirect'] = 'admin/structure/file-types';
}

/**
 * Form callback; presents file display settings for a given view mode.
 */
function file_display_form($form, &$form_state, $file_type, $view_mode) {
  $form['#file_type'] = $file_type->type;
  $form['#view_mode'] = $view_mode;
  $form['#tree'] = TRUE;
  $form['#attached']['js'][] = backdrop_get_path('module', 'file') . '/js/file.admin.js';

  // Retrieve available formatters for this file type.
  $formatters = file_info_formatter_types();

  foreach ($formatters as $name => $formatter) {
    if (!empty($formatter['hidden'])) {
      unset($formatters[$name]);
    }
    if (isset($formatter['mime types'])) {
      if (file_match_mimetypes($formatter['mime types'], $file_type->mimetypes)) {
        continue;
      }
      unset($formatters[$name]);
    }
  }
  $current_displays = file_displays_load($file_type->type, $view_mode);
  foreach ($current_displays as $name => $display) {
    $current_displays[$name] = (array) $display;
  }

  // Formatter status.
  $form['displays']['status'] = array(
    '#type' => 'item',
    '#title' => t('Enabled displays'),
    '#prefix' => '<div id="file-displays-status-wrapper">',
    '#suffix' => '</div>',
  );
  $i = 0;
  foreach ($formatters as $name => $formatter) {
    $form['displays']['status'][$name] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($formatter['label']),
      '#default_value' => !empty($current_displays[$name]['status']),
      '#description' => isset($formatter['description']) ? filter_xss($formatter['description']) : NULL,
      '#parents' => array('displays', $name, 'status'),
      '#weight' => (isset($formatter['weight']) ? $formatter['weight'] : 0) + ($i / 1000),
    );
    $i++;
  }

  // Formatter order (tabledrag).
  $form['displays']['order'] = array(
    '#type' => 'item',
    '#title' => t('Display precedence order'),
    '#theme' => 'file_display_order',
  );
  foreach ($formatters as $name => $formatter) {
    $form['displays']['order'][$name]['label'] = array(
      '#markup' => check_plain($formatter['label']),
    );
    $form['displays']['order'][$name]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $formatter['label'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => isset($current_displays[$name]['weight']) ? $current_displays[$name]['weight'] : 0,
      '#parents' => array('displays', $name, 'weight'),
    );
    $form['displays']['order'][$name]['#weight'] = $form['displays']['order'][$name]['weight']['#default_value'];
  }

  // Formatter settings.
  $form['display_settings_title'] = array(
    '#type' => 'item',
    '#title' => t('Display settings'),
  );
  $form['display_settings'] = array(
    '#type' => 'vertical_tabs',
  );
  $i = 0;
  foreach ($formatters as $name => $formatter) {
    if (isset($formatter['settings callback']) && ($function = $formatter['settings callback']) && function_exists($function)) {
      $defaults = !empty($formatter['default settings']) ? $formatter['default settings'] : array();
      $settings = !empty($current_displays[$name]['settings']) ? $current_displays[$name]['settings'] : array();
      $settings += $defaults;
      $settings_form = $function($form, $form_state, $settings, $name, $file_type->type, $view_mode);
      if (!empty($settings_form)) {
        $form['displays']['settings'][$name] = array(
          '#type' => 'fieldset',
          '#title' => check_plain($formatter['label']),
          '#parents' => array('displays', $name, 'settings'),
          '#group' => 'display_settings',
          '#weight' => (isset($formatter['weight']) ? $formatter['weight'] : 0) + ($i / 1000),
        ) + $settings_form;
      }
    }
    $i++;
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Process file display settings form submissions.
 */
function file_display_form_submit($form, &$form_state) {
  $file_type = $form['#file_type'];
  $view_mode = $form['#view_mode'];
  $displays = isset($form_state['values']['displays']) ? $form_state['values']['displays'] : array();
  file_display_save($form['#file_type'], $form['#view_mode'], $displays);
  backdrop_set_message(t('Your settings have been saved.'));
}

/**
 * Menu callback; disable or enable a single file type.
 */
function file_type_status_confirm($form, &$form_state, $type, $operation) {
  $form['type'] = array('#type' => 'value', '#value' => $type);
  $form['name'] = array('#type' => 'value', '#value' => $type->name);
  $form['operation'] = array('#type' => 'value', '#value' => $operation);
  $title = ($operation == 'enable') ? t('Enable') : t('Disable');
  $message = t('Are you sure you want to @operation the file type %type?', array('%type' => $type->name, '@operation' => $operation));
  return confirm_form($form, $message, 'admin/structure/file-types', '', $title);
}

/**
 * Process file type status confirm submissions.
 */
function file_type_status_confirm_submit($form, &$form_state) {
  $operation = $form_state['values']['operation'];
  $type = $form_state['values']['type'];
  if ($operation == 'disable') {
    $type->disabled = TRUE;
  }
  else {
    $type->disabled = FALSE;
  }
  file_type_save($type);

  $title = ($operation == 'enable') ? t('enabled') : t('disabled');
  $t_args = array('%name' => $form_state['values']['name'], '@title' => $title);
  backdrop_set_message(t('The file type %name has been @title.', $t_args));
  watchdog('file_entity', '@title file type %name.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/structure/file-types';
}

/**
 * Menu callback; revert a single file type.
 */
function file_type_revert_confirm($form, &$form_state, $type) {
  $form['type'] = array('#type' => 'value', '#value' => $type->type);
  $form['name'] = array('#type' => 'value', '#value' => $type->name);
  $message = t('Are you sure you want to revert the file type %type?', array('%type' => $type->name));
  return confirm_form($form, $message, 'admin/structure/file-types', '', t('Revert'));
}

/**
 * Process file type revert confirm submissions.
 */
function file_type_revert_confirm_submit($form, &$form_state) {
  $type = $form_state['values']['type'];

  config('file.type.' . $type)->delete();
  config_install_default_config('file', 'file.type.' . $type);

  $t_args = array('%name' => $form_state['values']['name']);
  backdrop_set_message(t('The file type %name has been reverted.', $t_args));
  watchdog('file', 'Reverted file type %name.', $t_args, WATCHDOG_NOTICE);

  file_type_cache_reset();

  $form_state['redirect'] = 'admin/structure/file-types';
  return;
}


/**
 * Menu callback; delete a single file type.
 */
function file_type_delete_confirm($form, &$form_state, $type) {
  $type = (array) $type;
  $form['type'] = array('#type' => 'value', '#value' => $type['type']);
  $form['name'] = array('#type' => 'value', '#value' => $type['name']);

  $message = t('Are you sure you want to delete the file type %type?', array('%type' => $type['name']));
  $caption = '';

  $num_files = db_query("SELECT COUNT(*) FROM {file_managed} WHERE type = :type", array(':type' => $type['type']))->fetchField();
  if ($num_files) {
    $caption .= '<p>' . format_plural($num_files, '%type is used by 1 file on your site. If you remove this file type, you will not be able to edit the %type file and it may not display correctly.', '%type is used by @count pieces of file on your site. If you remove %type, you will not be able to edit the %type file and it may not display correctly.', array('%type' => $type['name'])) . '</p>';
  }

  $caption .= '<p>' . t('This action cannot be undone.') . '</p>';

  return confirm_form($form, $message, 'admin/structure/file-types', $caption, t('Delete'));
}

/**
 * Process file type delete confirm submissions.
 */
function file_type_delete_confirm_submit($form, &$form_state) {
  file_type_delete($form_state['values']['type']);

  $t_args = array('%name' => $form_state['values']['name']);
  backdrop_set_message(t('The file type %name has been deleted.', $t_args));
  watchdog('file', 'Deleted file type %name.', $t_args, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/structure/file-types';
  return;
}

/**
 * Updates an existing file type or creates a new one.
 *
 * This function can be called on its own, or via the CTools exportables
 * 'save callback' for {file_type} objects.
 */
function file_type_save($type) {
  $type_data = (array) $type;
  if (!isset($type->description)) {
    $type_data['description'] = '';
  }

  $config = config('file.type.' . $type->type);
  foreach ($type_data as $key => $value) {
    $config->set($key, $value);
  }
  $config->save();

  if (empty($type->is_new)) {
    module_invoke_all('file_type_update', $type);
    $status = SAVED_UPDATED;
  }
  else {
    module_invoke_all('file_type_insert', $type);
    $status = SAVED_NEW;
  }

  // Clear the necessary caches.
  file_type_cache_reset();

  return $status;
}

/**
 * Deletes a file type from the configuration.
 *
 * @param object|string $type
 *   Either a loaded file type object or the machine-name of the type.
 */
function file_type_delete($type) {
  $info = file_type_get_type($type);
  $config = config('file.type.' . $type);
  $config->delete();
  field_attach_delete_bundle('file', $type);
  module_invoke_all('file_type_delete', $info);

  // Clear the node type cache.
  file_type_cache_reset();
}

/**
 * Returns information about file formatters from hook_file_formatter_info().
 *
 * @param string $formatter_type
 *   (optional) A file formatter type name. If omitted, all file formatter
 *   will be returned.
 *
 * @return string|array
 *   Either a file formatter description, as provided by
 *   hook_file_formatter_info(), or an array of all existing file formatters,
 *   keyed by formatter type name.
 */
function file_info_formatter_types($formatter_type = NULL) {
  $info = &backdrop_static(__FUNCTION__);
  if (!isset($info)) {
    $info = module_invoke_all('file_formatter_info');
    backdrop_alter('file_formatter_info', $info);
    uasort($info, '_file_sort_weight_label');
  }
  if ($formatter_type) {
    if (isset($info[$formatter_type])) {
      return $info[$formatter_type];
    }
  }
  else {
    return $info;
  }
}

/**
 * User sort function to sort by weight, then label/name.
 */
function _file_sort_weight_label($a, $b) {
  $a_weight = isset($a['weight']) ? $a['weight'] : 0;
  $b_weight = isset($b['weight']) ? $b['weight'] : 0;
  if ($a_weight == $b_weight) {
    $a_name = isset($a['name']) ? $a['name'] : '';
    $b_name = isset($b['name']) ? $b['name'] : '';
    return strcasecmp($a_name, $b_name);
  }
  else {
    return $a_weight < $b_weight ? -1 : 1;
  }
}
