<?php
/**
 * @file
 * Supports file operations including Manage and Delete.
 */

/**
 * Form callback for adding a file via an upload form.
 *
 * This is a multi step form which has 1-4 pages:
 * - Upload file
 * - Choose filetype
 *   If there is only one candidate (based on mimetype) we will skip this step.
 * - Edit scheme
 * - Edit fields (Skip this step if there are no fields on this entity type.)
 */
function file_add_form($form, &$form_state, $options = array()) {
  if (!is_array($options)) {
    $options = array($options);
  }
  $step = (isset($form_state['step']) && in_array($form_state['step'], array(1, 2, 3, 4))) ? $form_state['step'] : 1;

  $form['#step'] = $step;
  $form['#options'] = $options + array(
    'types' => array(),
    'enabledPlugins' => array(),
    'schemes' => array(),
    'max_filesize' => '',
    'uri_scheme' => 'public',
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/content/files',
    '#weight' => 15,
  );

  switch ($step) {
    case 1:
      $form['upload'] = array(
        '#type' => 'managed_file',
        '#title' => t('Upload a new file'),
        '#upload_location' => file_upload_destination_uri($options),
        '#upload_validators' => file_get_upload_validators($options),
        '#progress_indicator' => 'bar',
        '#pre_render' => array('file_managed_file_pre_render', 'file_upload_validators_pre_render'),
        '#default_value' => isset($form_state['storage']['upload']) ? $form_state['storage']['upload'] : NULL,
      );

      $form['actions']['next'] = array(
        '#type' => 'submit',
        '#value' => t('Next'),
      );

      form_load_include($form_state, 'inc', 'file', 'file.pages');

      return $form;

    case 2:
      return file_add_upload_step_filetype($form, $form_state, $options);

    case 3:
      return file_add_upload_step_scheme($form, $form_state, $options);

    case 4:
      return file_add_upload_step_fields($form, $form_state, $options);
  }
}

/**
 * Generate form fields for the second step in the add file wizard.
 */
function file_add_upload_step_filetype($form, &$form_state, array $options = array()) {
  backdrop_set_title(t('Configure file type'));
  $file = file_load($form_state['storage']['upload']);
  $selected_files = $form['#options']['types'];

  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('File type'),
    '#options' => file_get_filetype_candidates($file, $selected_files),
    '#default_value' => isset($form_state['storage']['type']) ? $form_state['storage']['type'] : NULL,
    '#required' => TRUE,
  );

  $form['actions']['previous'] = array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#limit_validation_errors' => array(),
    '#submit' => array('file_add_form_submit'),
  );
  $form['actions']['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * Generate form fields for the third step in the add file wizard.
 */
function file_add_upload_step_scheme($form, &$form_state, array $options = array()) {
  backdrop_set_title(t('Configure file scheme'));
  $file = file_load($form_state['storage']['upload']);

  $schemes = array();
  foreach (file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE) as $scheme => $info) {
    $schemes[$scheme] = check_plain($info['description']);
  }

  // Remove any schemes not found in the instance settings.
  if (!empty($options['schemes'])) {
    $schemes = array_intersect_key($schemes, array_flip($options['schemes']));
  }

  // Determine which scheme to use as the default value.
  if (isset($form_state['storage']['scheme'])) {
    $fallback_scheme = $form_state['storage']['scheme'];
  }
  elseif (!empty($options['uri_scheme'])) {
    $fallback_scheme = $options['uri_scheme'];
  }
  else {
    $fallback_scheme = file_default_scheme();
  }

  $form['scheme'] = array(
    '#type' => 'radios',
    '#title' => t('Destination'),
    '#options' => $schemes,
    '#default_value' => $fallback_scheme,
    '#required' => TRUE,
  );

  $form['actions']['previous'] = array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#limit_validation_errors' => array(),
    '#submit' => array('file_add_form_submit'),
  );
  $form['actions']['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * Generate form fields for the fourth step in the add file wizard.
 */
function file_add_upload_step_fields($form, &$form_state, array $options = array()) {
  backdrop_set_title(t('Configure file fields'));
  // Load the file and overwrite the filetype set on the previous screen.
  $file = file_load($form_state['storage']['upload']);
  $file->type = $form_state['storage']['type'];

  // Let users modify the filename here.
  $form['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $file->filename,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#weight' => -10,
  );

  // Add fields.
  field_attach_form('file', $file, $form, $form_state);

  $form['actions']['previous'] = array(
    '#type' => 'submit',
    '#value' => t('Previous'),
    '#limit_validation_errors' => array(),
    '#submit' => array('file_add_form_submit'),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validation handler for file_add_form().
 */
function file_add_form_validate($form, &$form_state) {
  if ($form['#step'] === 1 && empty($form_state['values']['upload'])) {
    form_set_error('upload', t('You need to upload a file.'));
  }
}

/**
 * Get the candidate filetypes for a given file.
 *
 * Only filetypes for which the user has access to create entities are returned.
 *
 * @param File $file
 *   An upload file array from form_state.
 *
 * @return array
 *   An array of file type bundles that support the file's mime type.
 */
function file_get_filetype_candidates(File $file, $selected_files = array()) {
  $types = module_invoke_all('file_type', $file);
  backdrop_alter('file_type', $types, $file);

  // If no file types are selected in field instance settings, allow all
  // available types.
  if (!empty($selected_files)) {
    // Limit file type candidates to field allowed types.
    $types = array_intersect($types, $selected_files);
  }

  $candidates = array();
  foreach ($types as $type) {
    $file->type = $type;
    if (file_access('create', $file)) {
      $candidates[$type] = file_type_get_name($file);
    }
  }
  return $candidates;
}

/**
 * Submit handler for the add file form.
 */
function file_add_form_submit($form, &$form_state) {
  $config = config('file.settings');
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $form_state['storage'] = array_merge($form_state['storage'], $form_state['values']);

  // Field selected allowed types.
  $selected_files = $form['#options']['types'];

  // This var is set to TRUE when we are ready to save the file.
  $save = FALSE;
  $trigger = $form_state['triggering_element']['#id'];
  $triggered_next = $trigger == 'edit-next' || (strpos($trigger, 'edit-next--') === 0);
  $triggered_previous = $trigger == 'edit-previous' || (strpos($trigger, 'edit-previous--') === 0);
  $step_delta = ($triggered_previous) ? -1 : 1;

  $steps_to_check = array(2, 3);
  if ($triggered_previous) {
    // If the previous button was hit,
    // the step checking order should be reversed 3, 2.
    $steps_to_check = array_reverse($steps_to_check);
  }

  foreach ($steps_to_check as $step) {
    // Check if we can skip step 2 and 3.
    if (($form['#step'] == $step - 1 && $triggered_next) || ($form['#step'] == $step + 1 && $triggered_previous)) {
      $file = file_load($form_state['storage']['upload']);
      if ($step == 2) {
        // Check if we can skip step 2.
        $candidates = file_get_filetype_candidates($file, $selected_files);
        if (count($candidates) == 1) {
          $candidates_keys = array_keys($candidates);
          // There is only one possible filetype for this file.
          // Skip the second page.
          $form['#step'] += $step_delta;
          $form_state['storage']['type'] = reset($candidates_keys);
        }
        elseif (!$candidates || $config->get('upload_wizard_skip_file_type')) {
          // Do not assign the file a file type.
          $form['#step'] += $step_delta;
          $form_state['storage']['type'] = FILE_TYPE_NONE;
        }
      }
      else {
        // Check if we can skip step 3.
        $options = $form['#options'];
        $schemes = file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE);

        // Remove any schemes not found in the instance settings.
        if (!empty($options['schemes'])) {
          $schemes = array_intersect_key($schemes, $options['schemes']);
        }

        if (!file_is_writeable($file)) {
          // The file is read-only (remote) and must use its provided scheme.
          $form['#step'] += $step_delta;
          $form_state['storage']['scheme'] = file_uri_scheme($file->uri);
        }
        elseif (count($schemes) == 1) {
          // There is only one possible stream wrapper for this file.
          // Skip the third page.
          $form['#step'] += $step_delta;
          $form_state['storage']['scheme'] = key($schemes);
        }
        elseif ($config->get('upload_wizard_skip_scheme')) {
          $form['#step'] += $step_delta;

          // Fallback to the URI scheme specified in the field settings
          // otherwise use the default file scheme.
          if (!empty($options['uri_scheme'])) {
            $form_state['storage']['scheme'] = $options['uri_scheme'];
          }
          else {
            $form_state['storage']['scheme'] = file_default_scheme();
          }
        }
      }
    }
  }

  // We have the filetype, check if we can skip step 4.
  if ($form['#step'] == 3 && $triggered_next) {
    $file = file_load($form_state['storage']['upload']);
    $form_state['file'] = $file;
    if (!field_info_instances('file', $form_state['storage']['type'])) {
      // This filetype doesn't have fields, save the file.
      $save = TRUE;
    }
    elseif ($config->get('upload_wizard_skip_fields')) {
      // Save the file with blanks fields.
      $save = TRUE;
    }
  }

  // Form id's can vary depending on how many other forms are displayed, so we
  // need to do string comparisons. e.g edit-submit--2.
  if ($triggered_next) {
    $form_state['step'] = $form['#step'] + 1;
  }
  elseif ($triggered_previous) {
    $form_state['step'] = $form['#step'] - 1;
  }
  elseif (strpos($trigger, 'edit-submit') !== FALSE) {
    $save = TRUE;
  }

  if ($save) {
    $file = file_load($form_state['storage']['upload']);
    if ($file) {
      if (file_uri_scheme($file->uri) != $form_state['storage']['scheme']) {
        $file_destination = $form_state['storage']['scheme'] . '://' . file_uri_target($file->uri);
        $file_destination = file_stream_wrapper_uri_normalize($file_destination);
        if ($moved_file = file_move($file, $file_destination, FILE_EXISTS_RENAME)) {
          // Only re-assign the file object if file_move() did not fail.
          $file = $moved_file;
        }
      }
      $file->type = $form_state['storage']['type'];
      $file->display = TRUE;

      // Change the file from temporary to permanent.
      $file->status = FILE_STATUS_PERMANENT;

      // Save the form fields.
      // Keep in mind that the values for the Field API fields must be in
      // $form_state['values'] and not in ['storage']. This is true as long as
      // the fields are on the last page of the multi step form.
      entity_form_submit_build_entity('file', $file, $form, $form_state);

      file_save($file);
      // Record that the module (in this case, the File module) is using the
      // file. Without the call to file_usage_add, file_managed_file_validate()
      // produces an error upon saving the form, saying that the uploaded file
      // may not be referenced.
      file_usage_add($file, 'file', 'file', $file->fid);
      $form_state['file'] = $file;
      backdrop_set_message(t('@type %name was uploaded.', array('@type' => file_type_get_name($file), '%name' => $file->filename)));
    }
    else {
      backdrop_set_message(t('An error occurred and no file was uploaded.'), 'error');
      return;
    }

    // Figure out destination.
    if (user_access('administer files')) {
      $path = 'admin/content/files';
    }
    else {
      $path = 'file/' . $file->fid;
    }
    $form_state['redirect'] = $path;
  }
  else {
    $form_state['rebuild'] = TRUE;
  }

  // Clear the page and block caches.
  cache_clear_all();
}

/**
 * Determines the upload location for the file add upload form.
 *
 * @param array $params
 *   An array of parameters from the media browser.
 * @param array $data
 *   (optional) An array of token objects to pass to token_replace().
 *
 * @return string
 *   A file directory URI with tokens replaced.
 *
 * @see token_replace()
 */
function file_upload_destination_uri(array $params, array $data = array()) {
  $params += array(
    'uri_scheme' => file_default_scheme(),
    'file_directory' => '',
  );

  $destination = trim($params['file_directory'], '/');

  // Replace tokens.
  $destination = token_replace($destination, $data);

  return $params['uri_scheme'] . '://' . $destination;
}

/**
 * Menu callback; view a single file entity.
 */
function file_view_page($file) {
  backdrop_set_title($file->filename);

  $uri = entity_uri('file', $file);
  // Set the file path as the canonical URL to prevent duplicate content.
  backdrop_add_html_head_link(array('rel' => 'canonical', 'href' => url($uri['path'], $uri['options'])), TRUE);
  // Set the non-aliased path as a default shortlink.
  backdrop_add_html_head_link(array('rel' => 'shortlink', 'href' => url($uri['path'], array_merge($uri['options'], array('alias' => TRUE)))), TRUE);

  return file_view($file, 'full');
}

/**
 * Menu callback; download a single file entity.
 */
function file_download_page($file) {
  // If the file does not exist it can cause problems with file_transfer().
  if (!is_file($file->uri)) {
    return MENU_NOT_FOUND;
  }
  $headers = array(
    'Content-Type' => mime_header_encode($file->filemime),
    'Content-Disposition' => 'attachment; filename="' . mime_header_encode(basename($file->uri)) . '"',
    'Content-Length' => $file->filesize,
    'Content-Transfer-Encoding' => 'binary',
    'Pragma' => 'no-cache',
    'Cache-Control' => 'must-revalidate, post-check=0, pre-check=0',
    'Expires' => '0',
  );

  // Let other modules alter the download headers.
  backdrop_alter('file_download_headers', $headers, $file);

  // Let other modules know the file is being downloaded.
  module_invoke_all('file_transfer', $file->uri, $headers);

  if (file_is_local($file)) {
    // For local files, transfer the file and do not reveal the actual URL.
    file_transfer($file->uri, $headers);
  }
  else {
    // For remote files, just redirect the user to that file's actual URL.
    $headers['Location'] = file_create_url($file->uri);
    foreach ($headers as $name => $value) {
      backdrop_add_http_header($name, $value);
    }
    backdrop_send_headers();
    backdrop_exit();
  }
}

/**
 * Page callback: Form constructor for the file manage form.
 *
 * @param File $file
 *   A file object from file_load().
 */
function file_manage_form($form, &$form_state, File $file) {
  backdrop_set_title(t('Manage file %title', array('%title' => $file->filename)), PASS_THROUGH);

  $form_state['file'] = $file;
  $form_state['temporary_upload'] = NULL;

  $form['#attributes']['class'][] = 'file-form';
  if (!empty($file->type)) {
    $form['#attributes']['class'][] = 'file-' . $file->type . '-form';
  }

  // Basic file information.
  // These elements are just values so they are not even sent to the client.
  // @todo Add type w/ https://github.com/backdrop/backdrop-issues/issues/2632
  foreach (array('fid', 'uid', 'timestamp', 'type') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($file->$key) ? $file->$key : NULL,
    );
  }

  $form['name'] = array(
    '#type' => 'item',
    '#title' => t('Filename'),
    '#markup' => check_plain($file->filename),
    '#weight' => -11,
  );

  $form['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('File display title'),
    '#description' => t('This text will be used in links to the uploaded file.'),
    '#default_value' => $file->filename,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#weight' => -10,
  );

  // Add a 'replace this file' upload field if the file is writeable.
  if (file_is_writeable($file)) {
    // Set up replacement file validation.
    $replacement_options = array();

    // The replacement file must have the same extension as the original file.
    $replacement_options['file_extensions'] = pathinfo($file->uri, PATHINFO_EXTENSION);

    $form['replace_upload'] = array(
      '#type' => 'managed_file',
      '#title' => t('Replace file'),
      '#description' => t('This file will replace the existing file. This action cannot be undone.'),
      '#upload_validators' => file_get_upload_validators($replacement_options),
    );
  }

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  // File destination information for administrators.
  $form['destination'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('manage files'),
    '#title' => t('File location'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('file-form-destination'),
    ),
    '#attached' => array(
      'js' => array(
        backdrop_get_path('module', 'file') . '/js/file.js',
      ),
    ),
  );

  $options = array();
  foreach (file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE) as $scheme => $info) {
    $options[$scheme] = check_plain($info['name']);
  }

  $form['destination']['scheme'] = array(
    '#type' => 'radios',
    '#title' => t('Destination'),
    '#options' => $options,
    '#default_value' => file_uri_scheme($file->uri),
  );

  if (!empty($file->uri)) {
    $directory_path = file_stream_wrapper_get_instance_by_uri($file->uri)->getDirectoryPath();
    $filename = file_uri_target($file->uri);
    $filepath = $directory_path . '/' . $filename;
    $url = url($filepath, array('absolute' => TRUE));
    $form['destination']['url'] = array(
      '#type' => 'item',
      '#title' => t('File URL'),
      '#markup' => l($url, $url, array(
        'attributes' => array('class' => array('file-preview-link')),
      )),
    );
  }

  // File user information for administrators.
  $anonymous = config_get_translated('system.core', 'anonymous');
  $form['user'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('administer files'),
    '#title' => t('Authoring information'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('file-form-user'),
    ),
    '#attached' => array(
      'js' => array(
        backdrop_get_path('module', 'file') . '/js/file.js',
        array(
          'type' => 'setting',
          'data' => array('anonymous' => $anonymous),
        ),
      ),
    ),
    '#weight' => 90,
  );
  $form['user']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Authored by'),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => !empty($file->uid) ? user_load($file->uid)->name : '',
    '#weight' => -1,
    '#description' => t('Leave blank for %anonymous.', array('%anonymous' => $anonymous)),
  );

  // Add the buttons.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('file_manage_form_submit'),
    '#validate' => array('file_manage_form_validate'),
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#weight' => 10,
    '#submit' => array('file_delete_redirect_form'),
    '#access' => user_access('delete files'),
  );

  // Build the URL for the cancel button taking into account that there might be
  // a "destination" that includes query string variables.
  $parameters = backdrop_get_query_parameters();
  $destination = isset($parameters['destination']) ? $parameters['destination'] : 'admin/content/files';
  $url = backdrop_parse_url($destination);

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $url['path'],
    '#options' => array('query' => $url['query']),
    '#weight' => 15,
  );

  field_attach_form('file', $file, $form, $form_state, $file->langcode);

  return $form;
}

/**
 * Form validation handler for file_manage_form().
 */
function file_manage_form_validate($form, &$form_state) {
  // Validate the "associated user" field.
  if (!empty($form_state['values']['name']) && !($account = user_load_by_name($form_state['values']['name']))) {
    // The use of empty() is mandatory in the context of usernames
    // as the empty string denotes the anonymous user. In case we
    // are dealing with an anonymous user we set the user ID to 0.
    form_set_error('name', t('The username %name does not exist.', array('%name' => $form_state['values']['name'])));
  }

  // Run entity form validation.
  entity_form_field_validate('file', $form, $form_state);
}

/**
 * Form submission handler for the 'Save' button for file_manage_form().
 */
function file_manage_form_submit($form, &$form_state) {
  /* @var File $file */
  $file = $form_state['file'];
  /* @var File $temporary_upload */
  $temporary_upload = $form_state['values']['replace_upload'] ? file_load($form_state['values']['replace_upload']) : NULL;
  $orphaned_uri = '';

  // Check if a replacement file has been uploaded.
  if ($temporary_upload) {
    // If the extension is the same, install into the old file's location.
    if (pathinfo($temporary_upload->uri, PATHINFO_EXTENSION) == pathinfo($file->uri, PATHINFO_EXTENSION)) {
      file_unmanaged_move($temporary_upload->uri, $file->uri, FILE_EXISTS_REPLACE);
    }
    // If different extensions, copy the base file name but save the extension.
    else {
      $destination_uri = rtrim($file->uri, backdrop_basename($file->uri)) . backdrop_basename($temporary_upload->uri);
      $replace_mode = $destination_uri == $file->uri ? FILE_EXISTS_REPLACE : FILE_EXISTS_RENAME;
      if ($new_file_uri = file_unmanaged_move($temporary_upload->uri, $destination_uri, $replace_mode)) {
        // Update if the uri target has changed.
        if ($new_file_uri != $file->uri) {
          $orphaned_uri = $file->uri;
          $file->uri = $new_file_uri;
        }
      }
    }

    // Copy attributes from the temporary file to the original one.
    $file->filesize = $temporary_upload->filesize;
    $file->filemime = $temporary_upload->filemime;
    $file->timestamp = $temporary_upload->timestamp;

    // Clear any image styles for this file.
    $image_info = image_get_info($file->uri);
    if (!empty($image_info)) {
      image_path_flush($file->uri);
    }
  }

  // Run entity form submit handling and save the file.
  entity_form_submit_build_entity('file', $file, $form, $form_state);

  // A user might assign the associated user by entering a user name in the file
  // edit form, which we then need to translate to a user ID.
  if (isset($file->name)) {
    // The use of isset() is mandatory in the context of user IDs, because
    // user ID 0 denotes the anonymous user.
    if ($user = user_load_by_name($file->name)) {
      $file->uid = $user->uid;
    }
    else {
      // Anonymous user.
      $file->uid = 0;
    }
  }
  elseif ($file->uid) {
    $user = user_load($file->uid);
    $file->name = $user->name;
  }

  // Update the URI and copy the file if the schema has changed.
  if (file_uri_scheme($file->uri) != $form_state['values']['scheme']) {
    $file_destination = $form_state['values']['scheme'] . '://' . file_uri_target($file->uri);
    $file_destination = file_stream_wrapper_uri_normalize($file_destination);
    $file_destination_dirname = backdrop_dirname($file_destination);
    // Create the directory in case it doesn't exist.
    file_prepare_directory($file_destination_dirname, FILE_CREATE_DIRECTORY);
    if ($moved_file = file_move($file, $file_destination, FILE_EXISTS_RENAME)) {
      // Only re-assign the file object if file_move() did not fail.
      $file = $moved_file;
    }
  }

  // Save all the modifications to the file.
  $file->save();

  $args = array(
    '%title' => entity_label('file', $file),
  );
  watchdog('file', 'File: updated %title.', $args);
  backdrop_set_message(t('File: %title has been updated.', $args));

  // Delete the temporary file entity created during the upload.
  if (!empty($temporary_upload)) {
    $temporary_upload->delete();
  }

  // If a file was replaced and renamed, delete the old file left over.
  if (!empty($orphaned_uri)) {
    file_unmanaged_delete($orphaned_uri);
  }

  $form_state['redirect'] = 'admin/content/files';
}

/**
 * Submit handler for delete button on file manage forms.
 */
function file_delete_redirect_form($form, &$form_state) {
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  backdrop_goto('file/' . $form_state['file']->fid . '/delete');
}

/**
 * Page callback: Form constructor for the file deletion confirmation form.
 *
 * Path: file/%file/delete
 *
 * @param File $file
 *   A file object from file_load().
 *
 * @see file_menu()
 */
function file_delete_form($form, &$form_state, File $file) {
  $form_state['file'] = $file;

  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $file->fid,
  );

  $description = '';
  $known_count = 0;
  $unknown_count = 0;
  $entity_list = _file_usage_list_links($file, $known_count, $unknown_count);

  if ($known_count || $unknown_count) {
    $description .= format_plural($known_count + $unknown_count, 'This file is referenced by one piece of content.', 'This file is referenced by @count pieces of content.');
  }

  if ($entity_list) {
    if ($unknown_count) {
      $entity_list[] = format_plural($unknown_count, 'And one additional unknown piece of content.', 'And @count additional unknown pieces of content.');
    }
    $description .= ' ' . t('Content referencing this file includes:');
    $description = '<p>' . $description . '</p>';
    $description .= theme('item_list', array('items' => $entity_list));
  }
  elseif ($unknown_count) {
    $description .= ' ' . t('However, this content is either missing or you do not have access to it.');
    $description = '<p>' . $description . '</p>';
  }
  else {
    $description .= t('This file has no known content referencing it, although it may still be in use.');
    $description = '<p>' . $description . '</p>';
  }
  $description .= '<p>' . t('Deleting this file may cause content to display improperly.') . '</p>';

  return confirm_form($form,
    t('Are you sure you want to delete the file %title?', array(
      '%title' => entity_label('file', $file),
    )),
    'admin/content/files',
    $description,
    t('Delete')
  );
}

/**
 * Form submission handler for file_delete_form().
 */
function file_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm'] && ($file = file_load($form_state['values']['fid']))) {
    file_delete($file->fid);
    watchdog('file', 'Administrator deleted file %title.', array('%title' => $file->label()));
    backdrop_set_message(t('%title has been deleted.', array('%title' => $file->label())));
  }

  $form_state['redirect'] = 'admin/content/files';

  // Clear the page and block caches.
  cache_clear_all();
}

/**
 * Multiple file deletion confirmation form.
 *
 * @see file_multiple_delete_confirm_submit()
 *
 * @ingroup forms
 */
function file_multiple_delete_confirm($form, &$form_state) {
  if (isset($form_state['fids'])) {
    $fids = $form_state['fids'];
  }
  elseif (isset($_SESSION['file_delete_action']['timestamp']) && (REQUEST_TIME - $_SESSION['file_delete_action']['timestamp'] < 6000)) {
    $fids = $_SESSION['file_delete_action']['fids'];
    $form_state['fids'] = $fids;
    $form_state['cache'] = TRUE;
    unset($_SESSION['file_delete_action']);
  }
  else {
    $fids = array();
  }

  $form['#tree'] = TRUE;

  if (empty($fids)) {
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content/files';
    $form['empty']['#markup'] = '<p>' . t('Return to the <a href="!url">manage files administration page</a>.', array('!url' => url($destination))) . '</p>';
    backdrop_set_message(t('No files have been selected for deletion.'), 'error');
    return $form;
  }

  $form['file_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  $items = array();
  $files = file_load_multiple($fids);
  $usage_count_total = 0;
  foreach ($files as $fid => $file) {
    $usage_count = _file_usage_get_total($file);
    $usage_count_total += $usage_count;
    $form['files'][$fid] = array(
      '#type' => 'hidden',
      '#value' => $fid,
    );
    $uri = $file->uri();
    $url = file_create_url($uri['path']);
    $label = l($file->label(), $url);
    if ($usage_count) {
      $label .= ' (' . format_plural($usage_count, 'referenced 1 time', 'referenced @count times') . ')';
    }
    $items[] = $label;
  }

  $confirm_question = format_plural(count($files), 'Are you sure you want to delete this file?', 'Are you sure you want to delete these files?');

  if ($usage_count_total) {
    $description = '<p>' . format_plural($usage_count_total, 'One piece of content references the listed files.', '@count pieces of content reference the listed files.') . '</p>';
  }
  else {
    $description = '<p>' . format_plural(count($files), 'This file has no known content referencing it, although it may still be in use.', 'These files have no known content referencing them, although they may still be in use.') . '</p>';
  }

  $description .= theme('item_list', array('items' => $items));

  $description .= '<p>' . format_plural(count($files), 'Deleting this file may cause content to display improperly.', 'Deleting these files may cause content to display improperly.') . '</p>';

  return confirm_form($form, $confirm_question, 'admin/content/files', $description, t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for file_multiple_delete_confirm().
 */
function file_multiple_delete_confirm_submit($form, &$form_state) {
  file_delete_multiple(array_keys($form_state['values']['files']));
  cache_clear_all();
  $count = count($form_state['values']['files']);
  watchdog('content', 'Administrator deleted @count files.', array('@count' => $count));

  backdrop_set_message(format_plural($count, 'Deleted 1 file.', 'Deleted @count files.'));
  $form_state['redirect'] = 'admin/content/files';
}

/**
 * Build a list of links to content that references a file.
 *
 * @param File $file
 *   The file entity for which a list of links should be generated.
 * @param int $known_count
 *   The number of usages that are found and to which the user has access.
 * @param int $unknown_count
 *   The number of usages which cannot be displayed in the list of links.
 *
 * @return array
 *   An array of strings suitable for passing into theme('item_list').
 */
function _file_usage_list_links(File $file, &$known_count, &$unknown_count) {

  // Make a list of links showing where this file is in use. Files in use by
  // known entities are linked. Orphaned usages or ones not associated with an
  // entities are considered an "Unknown" usage. If the user does not have
  // access to the associated entity, it is considered unknown and no link is
  // shown.
  $known_count = 0;
  $unknown_count = 0;
  $entity_list = array();
  if ($file_usage_list = file_usage_list($file)) {
    foreach ($file_usage_list as $module_name => $module_usages) {
      foreach ($module_usages as $entity_type => $entity_usages) {
        $entity_type_info = entity_get_info($entity_type);
        $entity_ids = array();
        foreach ($entity_usages as $entity_id => $usage_count) {
          if ($entity_type_info) {
            $entity_ids[] = $entity_id;
          }
          else {
            $unknown_count += $usage_count;
          }
        }
        if ($entity_ids) {
          $entities = entity_load_multiple($entity_type, $entity_ids);

          // Perform an access check on all given entities.
          $query = new EntityFieldQuery();
          $query->entityCondition('entity_type', $entity_type);
          $query->entityCondition('entity_id', $entity_ids);
          $query->addTag($entity_type . '_access');
          $result = $query->execute();

          foreach ($entities as $entity) {
            // If the entity is in the EFQ result, access is allowed.
            if (isset($result[$entity_type][$entity->id()])) {
              $uri = $entity->uri();
              $entity_list[] = l($entity->label(), $uri['path'], $uri['options']);
              $known_count++;
            }
            // If not, do not show a link and consider it an unknown location.
            else {
              $unknown_count++;
            }
          }
        }
      }
    }
  }
  return $entity_list;
}

/**
 * Get the complete total number of known usages for a file.
 *
 * @param File $file
 *   A file entity object.
 * @return int
 *   The complete number of usages.
 */
function _file_usage_get_total(File $file) {
  $total = 0;
  if ($file_usage_list = file_usage_list($file)) {
    foreach ($file_usage_list as $module_name => $module_usages) {
      foreach ($module_usages as $entity_type => $entity_usages) {
        foreach ($entity_usages as $entity_id => $usage_count) {
          $total += $usage_count;
        }
      }
    }
  }
  return $total;
}

/**
 * Page callback to show file usage information.
 */
function file_usage_page($file) {
  $rows = array();
  $occurred_entities = array();

  // Determine all of the locations where a file is used, then loop through the
  // occurrences and filter out any duplicates.
  foreach (file_usage_list($file) as $module => $type) {
    foreach ($type as $entity_type => $entity_ids) {
      // There are cases where the actual entity doesn't exist.
      // We have to handle this.
      $entity_info = entity_get_info($entity_type);
      $entities = empty($entity_info) ? NULL : entity_load($entity_type, array_keys($entity_ids));

      foreach ($entity_ids as $entity_id => $count) {
        // If this entity has already been listed in the table, just add any
        // additional usage to the total count column in the table row and
        // continue on to the next iteration of the loop.
        if (isset($occurred_entities[$entity_type][$entity_id])) {
          $rows[$occurred_entities[$entity_type][$entity_id]][2] += $count;
          continue;
        }

        // Retrieve the label and the URI of the entity.
        $label = empty($entities[$entity_id]) ? $module : entity_label($entity_type, $entities[$entity_id]);
        $entity_uri = empty($entities[$entity_id]) ? NULL : entity_uri($entity_type, $entities[$entity_id]);

        // Link the label to the URI when possible.
        if (!empty($entity_uri['path'])) {
          $uri = $entity_uri['path'];
          $entity_label = l($label, $uri);
        }
        else {
          $entity_label = check_plain($label);
        }

        $rows[] = array($entity_label, $entity_type, $count);

        // Record the occurrence of the entity to ensure that it isn't listed in
        // the table again.
        $occurred_entities[$entity_type][$entity_id] = count($rows) - 1;
      }
    }
  }

  $header = array(t('Entity'), t('Entity type'), t('Use count'));
  $build['caption'] = array(
    '#type' => 'help',
    '#markup' => t('This table lists all of the places where @filename is used.', array('@filename' => $file->filename)),
  );
  $build['usage_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('This file is not currently used.'),
  );

  return $build;
}

/**
 * Retrieves the upload validators for a file.
 *
 * @param array $options
 *   (optional) An array of options for file validation.
 *
 * @return array
 *   An array suitable for passing to file_save_upload() or for a managed_file
 *   or upload element's '#upload_validators' property.
 */
function file_get_upload_validators(array $options = array()) {
  $config = config('file.settings');
  // Set up file upload validators.
  $validators = array();

  // Validate file extensions. If there are no file extensions in $params and
  // there are no Media defaults, there is no file extension validation.
  if (!empty($options['file_extensions'])) {
    $validators['file_validate_extensions'] = array($options['file_extensions']);
  }
  else {
    $validators['file_validate_extensions'] = array($config->get('default_allowed_extensions'));
  }

  // Cap the upload size according to the system or user defined limit.
  $max_filesize = parse_size(file_upload_max_size());
  $file_max_filesize = parse_size($config->get('max_filesize'));

  // If the user defined a size limit, use the smaller of the two.
  if (!empty($file_max_filesize)) {
    $max_filesize = min($max_filesize, $file_max_filesize);
  }

  if (!empty($options['max_filesize']) && $options['max_filesize'] < $max_filesize) {
    $max_filesize = parse_size($options['max_filesize']);
  }

  // There is always a file size limit due to the PHP server limit.
  $validators['file_validate_size'] = array($max_filesize);

  // Add image validators.
  $options += array('min_resolution' => 0, 'max_resolution' => 0);
  if ($options['min_resolution'] || $options['max_resolution']) {
    $validators['file_validate_image_resolution'] = array($options['max_resolution'], $options['min_resolution']);
  }

  // Add other custom upload validators from options.
  if (!empty($options['upload_validators'])) {
    $validators += $options['upload_validators'];
  }

  return $validators;
}
