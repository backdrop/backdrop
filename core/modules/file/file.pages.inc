<?php
/**
 * @file
 * Supports file operations including Manage and Delete.
 */

/**
 * Page callback: Form constructor for the file manage form.
 *
 * @param File $file
 *   A file object from file_load().
 */
function file_manage_form($form, &$form_state, File $file) {
  backdrop_set_title(t('<em>Manage file</em> @title', array('@title' => $file->filename)), PASS_THROUGH);

  $form_state['file'] = $file;
  $form_state['temporary_upload'] = NULL;

  $form['#attributes']['class'][] = 'file-form';
  if (!empty($file->type)) {
    $form['#attributes']['class'][] = 'file-' . $file->type . '-form';
  }

  // Basic file information.
  // These elements are just values so they are not even sent to the client.
  // @todo Add type w/ https://github.com/backdrop/backdrop-issues/issues/2632
  foreach (array('fid', 'uid', 'timestamp') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($file->$key) ? $file->$key : NULL,
    );
  }

  $form['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('File name'),
    '#default_value' => $file->filename,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#weight' => -10,
  );

  // Set up replacement file validation.
  $replacement_options = array();

  // The replacement file must have the same extension as the original file.
  $replacement_options['file_extensions'] = pathinfo($file->uri, PATHINFO_EXTENSION);

  $form['replace_upload'] = array(
    '#type' => 'managed_file',
    '#title' => t('File'),
    '#description' => t('This file will replace the existing file. This action cannot be undone.'),
    '#upload_validators' => file_get_upload_validators($replacement_options),
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  // File destination information for administrators.
  $form['destination'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('administer files'),
    '#title' => t('Destination'),
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

  // File user information for administrators.
  $anonymous = config_get('system.core', 'anonymous');
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
          'data' => array('anonymous' => t($anonymous)),
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
    '#submit' => array('file_delete_form_submit'),
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
  // Set up file upload validators.
  $validators = array();

  // Validate file extensions. If there are no file extensions in $params there
  // is no file extension validation.
  if (!empty($options['file_extensions'])) {
    $validators['file_validate_extensions'] = array($options['file_extensions']);
  }
  else {
    $validators['file_validate_extensions'] = array(config_get('system.core', 'file_default_allowed_extensions'));
  }

  // Cap the upload size according to the system or user defined limit.
  $max_filesize = parse_size(file_upload_max_size());
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

  $form_state['redirect'] = '<front>';

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
