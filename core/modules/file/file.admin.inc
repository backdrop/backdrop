<?php
/**
 * @file
 * Admin page callbacks for the File module.
 */

/**
 * Displays the file type admin overview page.
 */
function file_list_types_page() {
  $file_entity_info = entity_get_info('file');
  $types = file_type_get_types();
  $names = file_type_get_names();
  $field_ui = module_exists('field_ui') && user_access('administer fields');
  $header = array(t('Name'), t('Description'), t('Storage state'), t('Operations'));
  $rows_enabled = array();
  $rows_disabled = array();
  $weight = 0;

  // Sort the list of content types by name instead of machine name.
  natcasesort($names);

  foreach ($names as $key => $type) {
    $links = array();
    $type = $types[$key];
    $weight++;
    $row = array(
      array(
        'data' => theme('label_machine_name__file', array(
          'label' => $type->name,
          'machine_name' => $type->type,
        )),
      ),
      filter_xss_admin($type->description),
    );


    if ($type->storage === FILE_TYPE_STORAGE_OVERRIDE) {
      $storage_state = t('Overridden');
    }
    elseif ($type->storage === FILE_TYPE_STORAGE_DEFAULT) {
      $storage_state = t('Default (module-provided)');
    }
    else {
      $storage_state = t('Custom');
    }
    $row[] = $storage_state;


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
      'weight' => 11,
    );

    if ($type->disabled) {
      $links['enable'] = array(
        'title' => t('Enable'),
        'href' => $path . '/enable',
        'query' => array('token' => backdrop_get_token($type->type)),
        'weight' => -10,
      );
    }
    else {
      $links['disable'] = array(
        'title' => t('Disable'),
        'href' => $path . '/disable',
        'query' => array('token' => backdrop_get_token($type->type)),
        'weight' => 20,
      );
    }

    if ($type->storage == FILE_TYPE_STORAGE_OVERRIDE) {
      $links['revert'] = array(
        'title' => t('Revert'),
        'href' => $path . '/revert',
        'weight' => 15,
      );
    }
    elseif ($type->storage == FILE_TYPE_STORAGE_NORMAL) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => $path . '/delete',
        'weight' => 25,
      );
    }

    backdrop_sort($links);
    $row[] = array(
      'data' => array(
        '#type' => 'operations',
        '#links' => $links,
      ),
    );

    if ($type->disabled) {
      $rows_disabled[] = array(
        'data' => $row,
        'class' => array('disabled'),
      );
    }
    else {
      $rows_enabled[] = array(
        'data' => $row,
        'class' => array('enabled'),
      );
    }
  }

  $rows = array_merge($rows_enabled, $rows_disabled);

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

  $display = file_display_load($file_type->type, $view_mode);

  // Formatter type.
  $form['display']['formatter'] = array(
    '#type' => 'radios',
    '#title' => t('Display type'),
    '#options' => array(),
    '#default_value' => config_get('file_display.' . $file_type->type, $view_mode . '.formatter'),
  );

  $formatters = array_reverse($formatters);

  foreach ($formatters as $name => $formatter) {
    $form['display']['formatter']['#options'][$name] = check_plain($formatter['label']);
    $form['display']['formatter'][$name]['#description'] = isset($formatter['description']) ? filter_xss($formatter['description']) : NULL;

    if (isset($formatter['settings callback']) && ($function = $formatter['settings callback']) && function_exists($function)) {

      $defaults = !empty($formatter['default settings']) ? $formatter['default settings'] : array();
      $settings = !empty($display['settings'][$name]) ? $display['settings'][$name] : array();
      $settings += $defaults;

      $settings_form = $function($form, $form_state, $settings, $name, $file_type->type, $view_mode);

      if (!empty($settings_form)) {
        $form['display']['settings'][$name] = array(
          '#type' => 'fieldset',
          '#title' => check_plain($formatter['label']) . ' ' . t('settings'),
          '#parents' => array('display', 'settings', $name),
          '#group' => 'display_settings',
          '#states' => array(
            'visible' => array(
              ':input[name="display[formatter]"]' => array('value' => $name),
            ),
          ),
        ) + $settings_form;
      }
    }
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

  $display = array(
    'formatter' => $form_state['values']['display']['formatter'],
    'settings' => $form_state['values']['display']['settings'],
  );

  file_display_save($form['#file_type'], $form['#view_mode'], $display);
  backdrop_set_message(t('Your settings have been saved.'));
}

/**
 * Menu callback; disable or enable a single file type.
 */
function file_type_toggle_enable($type, $status) {
  // Validate the token for this page.
  if (empty($_GET['token']) || $_GET['token'] !== backdrop_get_token($type->type)) {
    return MENU_ACCESS_DENIED;
  }
  $type->disabled = !$status;
  file_type_save($type);
  if ($status) {
    $message = t('The file type %name has been enabled.', array('%name' => $type->name));
  }
  else {
    $message = t('The file type %name has been disabled.', array('%name' => $type->name));
  }
  backdrop_set_message($message);
  backdrop_goto('admin/structure/file-types');
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
  watchdog('file', '@title file type %name.', $t_args, WATCHDOG_NOTICE);
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
 * Form callback for file settings.
 */
function file_settings_form($form, &$form_state) {
  $config = config('file.settings');

  $form = array('#config' => 'file.settings');

  $php_limit = format_size(file_upload_max_size());
  $form['max_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size'),
    '#default_value' => $config->get('max_filesize'),
    '#description' => t('Enter a value like "512" (bytes), "80 KB" (kilobytes) or "50 MB" (megabytes). If left empty, the file sizes will be limited only by PHP\'s maximum post and file upload sizes.'),
    '#size' => 10,
    '#element_validate' => array('_file_generic_settings_max_filesize'),
    '#attributes' => array(
      'placeholder' => $php_limit,
    ),
  );

  $form['default_file_directory'] = array(
    '#type' => 'textfield',
    '#title' => t('Default file directory'),
    '#description' => t('Subdirectory where files will be stored if the file is uploaded through the file/add page. This field supports tokens.'),
    '#default_value' => $config->get('default_file_directory'),
    '#maxlength' => NULL,
    '#field_prefix' => 'files/',
  );
  $form['default_file_directory']['tokens'] = array(
    '#theme' => 'token_tree',
    '#dialog' => TRUE,
  );

  $form['default_allowed_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Default allowed file extensions'),
    '#default_value' => $config->get('default_allowed_extensions'),
    '#description' => t('Separate extensions with a space and do not include the leading dot.'),
    '#maxlength' => NULL,
  );

  $form['file_upload_wizard'] = array(
    '#type' => 'fieldset',
    '#title' => t('File upload wizard'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Configure the steps available when uploading a new file.'),
  );
  $form['file_upload_wizard']['upload_wizard_skip_file_type'] = array(
    '#type' => 'checkbox',
    '#title' => t('Skip filetype selection.'),
    '#default_value' => $config->get('upload_wizard_skip_file_type'),
    '#description' => t('The file type selection step is only available if the uploaded file falls into two or more file types. If this step is skipped, files with no available file type or two or more file types will not be assigned a file type.'),
  );
  $form['file_upload_wizard']['upload_wizard_skip_scheme'] = array(
    '#type' => 'checkbox',
    '#title' => t('Skip scheme selection.'),
    '#default_value' => $config->get('upload_wizard_skip_scheme'),
    '#description' => t('The scheme selection step is only available if two or more file destinations, such as public local files served by the webserver and private local files served by Drupal, are available. If this step is skipped, files will automatically be saved using the default download method.'),
  );
  $form['file_upload_wizard']['upload_wizard_skip_fields'] = array(
    '#type' => 'checkbox',
    '#title' => t('Skip available fields.'),
    '#default_value' => $config->get('upload_wizard_skip_fields'),
    '#description' => t('The field selection step is only available if the file type the file belongs to has any available fields. If this step is skipped, any fields on the file will be left blank.'),
  );
  $form['file_replace_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('File replace optons'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Default settings for how to handle file name changes during replace.'),
  );
  $form['file_replace_options']['keep_original_filename'] = array(
    '#type' => 'checkbox',
    '#title' => t('Keep original file name'),
    '#default_value' => $config->get('keep_original_filename'),
    '#description' => t('Rename the newly uploaded file to the name of the original file. This action cannot be undone.'),
  );

  $form['protect_repeated_render'] = array(
    '#type' => 'checkbox',
    '#title' => t('Protect against repeat rendering'),
    '#default_value' => $config->get('protect_repeated_render'),
    '#description' => t('Avoid rendering the same file more than 10 times. This can be a sign of an image getting caught in a recursive render, but it can also be triggered when the same image is rendered more than 20 times, e.g. in an long content list or data feed.'),
  );

  return system_settings_form($form);
}

/**
 * Validation handler for file_settings_form().
 */
function file_settings_form_validate(&$form, &$form_state) {
  if (isset($form_state['values']['default_file_directory'])) {
    $form_state['values']['default_file_directory'] = trim($form_state['values']['default_file_directory'], '/');
  }
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
