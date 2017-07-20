<?php
/**
 * @file
 * Supports file operations including View, Edit, and Delete.
 */

/**
 * @file
 * Supports file operations including View, Edit, and Delete.
 */

/**
 * Menu callback; view a single file entity.
 */
function file_view_page($file) {
  drupal_set_title($file->filename);

  $uri = entity_uri('file', $file);
  // Set the file path as the canonical URL to prevent duplicate content.
  drupal_add_html_head_link(array('rel' => 'canonical', 'href' => url($uri['path'], $uri['options'])), TRUE);
  // Set the non-aliased path as a default shortlink.
  drupal_add_html_head_link(array('rel' => 'shortlink', 'href' => url($uri['path'], array_merge($uri['options'], array('alias' => TRUE)))), TRUE);

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
  // @todo Remove this once drupal_basename is fixed for PHP versions greater
  //  than '5.3.29'.
  $basename_function = version_compare(PHP_VERSION,'5.3.29','>') ? 'basename' : 'drupal_basename';
  $headers = array(
    'Content-Type' => mime_header_encode($file->filemime),
    'Content-Disposition' => 'attachment; filename="' . mime_header_encode($basename_function($file->uri)) . '"',
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
 * Page callback: Form constructor for the file deletion confirmation form.
 *
 * Path: file/%file/delete
 *
 * @param object $file
 *   A file object from file_load().
 *
 * @see file_menu()
 */
function file_delete_form($form, &$form_state, $file) {
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

/**
 * Page callback to show file usage information.
 */
function file_usage_page($file) {
  $rows = array();
  $occured_entities = array();

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
        if (isset($occured_entities[$entity_type][$entity_id])) {
          $rows[$occured_entities[$entity_type][$entity_id]][2] += $count;
          continue;
        }

        // Retrieve the label and the URI of the entity.
        $label = empty($entities[$entity_id]) ? $module : entity_label($entity_type, $entities[$entity_id]);
        $entity_uri = empty($entities[$entity_id]) ? NULL : entity_uri($entity_type, $entities[$entity_id]);

        // Link the label to the URI when possible.
        if (!empty($entity_uri['path'])) {
          $entity_label = l($label, $entity_uri['path']);
        }
        else {
          $entity_label = check_plain($label);
        }

        $rows[] = array($entity_label, $entity_type, $count);

        // Record the occurrence of the entity to ensure that it isn't listed in
        // the table again.
        $occured_entities[$entity_type][$entity_id] = count($rows) - 1;
      }
    }
  }

  $header = array(t('Entity'), t('Entity type'), t('Use count'));
  $build['usage_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#caption' => t('This table lists all of the places where @filename is used.', array('@filename' => $file->filename)),
    '#empty' => t('This file is not currently used.'),
  );

  return $build;
}

/**
 * Page callback: Form constructor for the file edit form.
 *
 * Path: file/%file/edit
 *
 * @param object $file
 *   A file object from file_load().
 *
 * @see file_menu()
 *
 * @todo Rename this form to file_edit_form to ease into core.
 */
function file_edit($form, &$form_state, $file) {
  backdrop_set_title(t('<em>Edit @type</em> @title', array('@type' => $file->type, '@title' => $file->filename)), PASS_THROUGH);

  $form_state['file'] = $file;

  $form['#attributes']['class'][] = 'file-form';
  if (!empty($file->type)) {
    $form['#attributes']['class'][] = 'file-' . $file->type . '-form';
  }

  // Basic file information.
  // These elements are just values so they are not even sent to the client.
  foreach (array('fid', 'type', 'uid', 'timestamp') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($file->$key) ? $file->$key : NULL,
    );
  }

  $form['filename'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $file->filename,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#weight' => -10,
  );

  // Add a 'replace this file' upload field if the file is writeable.
  if (file_is_writeable($file)) {
    // Set up replacement file validation.
    $replacement_options = array();

    // The replacement file must have an extension valid for the original type.
    $file_extensions = array();
    $file_type_name = isset($file->type) ? $file->type : file_get_type($file);
    if ($file_type_name && ($file_type = file_type_load($file_type_name))) {
      $file_extensions = file_type_get_valid_extensions($file_type);
    }

    // Set allowed file extensions.
    if (!empty($file_extensions)) {
      // Set to type based file extensions.
      $replacement_options['file_extensions'] = implode(' ', $file_extensions);
    }
    else {
      // Fallback to the extension of the current file.
      $replacement_options['file_extensions'] = pathinfo($file->uri, PATHINFO_EXTENSION);
    }

    $form['replace_upload'] = array(
      '#type' => 'file',
      '#title' => t('Replace file'),
      '#description' => t('This file will replace the existing file. This action cannot be undone.'),
      '#upload_validators' => file_get_upload_validators($replacement_options),
      '#pre_render' => array('file_upload_validators_pre_render'),
    );
  }

  $form['preview'] = file_view_file($file, 'preview');

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 99,
  );

  // File destination information for administrators.
  $form['destination'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('administer files') && file_is_writeable($file),
    '#title' => t('Destination'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('file-form-destination'),
    ),
    '#attached' => array(
      'js' => array(
        backdrop_get_path('module', 'file') . '/js/file_entity.js',
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
  $form['user'] = array(
    '#type' => 'fieldset',
    '#access' => user_access('administer files'),
    '#title' => t('User information'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array(
      'class' => array('file-form-user'),
    ),
    '#attached' => array(
      'js' => array(
        backdrop_get_path('module', 'file') . '/js/file_edit.js',
        array(
          'type' => 'setting',
          'data' => array('anonymous' => variable_get('anonymous', t('Anonymous'))),
        ),
      ),
    ),
    '#weight' => 90,
  );
  $form['user']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Associated with'),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => !empty($file->uid) ? user_load($file->uid)->name : '',
    '#weight' => -1,
    '#description' => t('Leave blank for %anonymous.', array('%anonymous' => variable_get('anonymous', t('Anonymous')))),
  );

  // Add the buttons.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('file_edit_submit'),
    '#validate' => array('file_edit_validate'),
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#weight' => 10,
    '#submit' => array('file_edit_delete_submit'),
    '#access' => file_access('delete', $file),
  );

  // Build the URL for the cancel button taking into account that there might be
  // a "destination" that includes query string variables.
  $parameters = backdrop_get_query_parameters();
  $destination = isset($parameters['destination']) ? $parameters['destination'] : 'file/' . $file->fid;
  $url = backdrop_parse_url($destination);

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $url['path'],
    '#options' => array('query' => $url['query']),
    '#weight' => 15,
  );

  $langcode = function_exists('entity_language') ? entity_language('file', $file) : NULL;
  field_attach_form('file', $file, $form, $form_state, $langcode);

  return $form;
}

/**
 * Form validation handler for file_edit().
 */
function file_edit_validate($form, &$form_state) {
  $file = (object) $form_state['values'];

  // Validate the "associated user" field.
  if (!empty($file->name) && !($account = user_load_by_name($file->name))) {
    // The use of empty() is mandatory in the context of usernames
    // as the empty string denotes the anonymous user. In case we
    // are dealing with an anonymous user we set the user ID to 0.
    form_set_error('name', t('The username %name does not exist.', array('%name' => $file->name)));
  }

  // Handle the replacement file if uploaded.
  if (isset($form_state['values']['replace_upload'])) {
    // Save the file as a temporary file.
    $file = file_save_upload('replace_upload', $form['replace_upload']['#upload_validators']);
    if (!empty($file)) {
      // Put the temporary file in form_values so we can save it on submit.
      $form_state['values']['replace_upload'] = $file;
    }
    elseif ($file === FALSE) {
      // File uploaded failed.
      form_set_error('replace_upload', t('The replacement file could not be uploaded.'));
    }
  }

  // Run entity form validation.
  entity_form_field_validate('file', $form, $form_state);
}

/**
 * Form submission handler for the 'Save' button for file_edit().
 */
function file_edit_submit($form, &$form_state) {
  $file = $form_state['file'];
  $orphaned_uri = '';

  // Check if a replacement file has been uploaded.
  if (!empty($form_state['values']['replace_upload'])) {
    $replacement = $form_state['values']['replace_upload'];
    // Move file from temp to permanent home.
    if (pathinfo($replacement->uri, PATHINFO_EXTENSION) == pathinfo($file->uri, PATHINFO_EXTENSION)) {
        file_unmanaged_copy($replacement->uri, $file->uri, FILE_EXISTS_REPLACE);
    } else {
      $destination_uri = rtrim($file->uri, backdrop_basename($file->uri)) . backdrop_basename($replacement->uri);
      $replace_mode = $destination_uri == $file->uri ? FILE_EXISTS_REPLACE : FILE_EXISTS_RENAME;
      if ($new_file_uri = file_unmanaged_copy($replacement->uri, $destination_uri, $replace_mode)) {
        // @todo Add watchdog() about replaced file here?

        // Remove temporary file.
        file_delete($replacement);

        // Update if the uri target has changed.
        if ($new_file_uri != $file->uri) {
         // Store the original file uri to delete if save is successful.
         $orphaned_uri = $file->uri;

          // Update file entity uri.
          $file->uri = $new_file_uri;
        }
      }
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

  file_save($file);

  $args = array(
    '@type' => file_type_get_name($file),
    '%title' => entity_label('file', $file),
  );
  watchdog('file', '@type: updated %title.', $args);
  backdrop_set_message(t('@type %title has been updated.', $args));

  // Clean up orphaned file.
  if (!empty($orphaned_uri)) {
    file_unmanaged_delete($orphaned_uri);

    $args['@orphaned'] = file_uri_target($orphaned_uri);
    watchdog('file', '@type: deleted orphaned file @orphaned for %title.', $args);
    backdrop_set_message(t('The replaced @type @orphaned has been deleted.', $args));
  }

  $form_state['redirect'] = 'file/' . $file->fid;

  // Clear the page and block caches.
  cache_clear_all();
}

/**
 * Form submission handler for the 'Delete' button for file_entity_edit().
 */
function file_edit_delete_submit($form, &$form_state) {
  $fid = $form_state['values']['fid'];
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = backdrop_get_destination();
    unset($_GET['destination']);
  }
  $form_state['redirect'] = array('file/' . $fid . '/delete', array('query' => $destination));

  // Clear the page and block caches.
  cache_clear_all();
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

  // Validate file extensions. If there are no file extensions in $params and
  // there are no Media defaults, there is no file extension validation.
  if (!empty($options['file_extensions'])) {
    $validators['file_validate_extensions'] = array($options['file_extensions']);
  }
  else {
    $validators['file_validate_extensions'] = array(variable_get('file_default_allowed_extensions', 'jpg jpeg gif png txt doc docx xls xlsx pdf ppt pptx pps ppsx odt ods odp mp3 mov mp4 m4a m4v mpeg avi ogg oga ogv weba webp webm'));
  }

  // Cap the upload size according to the system or user defined limit.
  $max_filesize = parse_size(file_upload_max_size());
  $file_entity_max_filesize = parse_size(variable_get('file_entity_max_filesize', ''));

  // If the user defined a size limit, use the smaller of the two.
  if (!empty($file_entity_max_filesize)) {
    $max_filesize = min($max_filesize, $file_entity_max_filesize);
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