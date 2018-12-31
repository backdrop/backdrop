<?php
/**
 * @file
 * User page callbacks for the Filter module.
 */

/**
 * Page callback: Displays a page with long filter tips.
 *
 * @return string
 *   An HTML-formatted string.
 *
 * @see filter_menu()
 * @see theme_filter_tips()
 */
function filter_tips_long($format = NULL) {
  if (!empty($format)) {
    $output = theme('filter_tips', array(
      'tips' => _filter_tips($format->format, TRUE),
      'long' => TRUE
    ));
  }
  else {
    $output = theme('filter_tips', array(
      'tips' => _filter_tips(-1, TRUE),
      'long' => TRUE
    ));
  }
  return $output;
}

/**
 * Autocomplete callback for link textfield.
 */
function _filter_link_autocomplete($string) {
  $matches = array();
  $match_count = 0;
  if ($string) {
    // Get node matches.
    $results = db_select('node')
      ->fields('node', array('nid', 'title', 'status'))
      ->condition('title', db_like($string) . '%', 'LIKE')
      ->condition('status', 1)
      ->addTag('node_access')
      ->orderBy('nid', 'DESC')
      ->range(0, 10)
      ->execute();
    foreach ($results as $result) {
      $path = backdrop_get_path_alias('node/' . $result->nid);
      $matches['/' . $path] = check_plain($result->title) . ' (' . t('Content') . ')';
      $match_count++;
    }

    // Get file matches.
    if (user_access('manage files') && $match_count < 10) {
      $results = db_select('file_managed')
        ->fields('file_managed', array('fid', 'filename', 'uri', 'filemime', 'status'))
        ->condition('filename', db_like($string) . '%', 'LIKE')
        ->condition('status', 1)
        ->condition('filemime', 'image%', 'NOT LIKE')
        ->orderBy('fid', 'DESC')
        ->range(0, 10 - $match_count)
        ->execute();
      foreach ($results as $result) {
        // Try to make a local path if possible for better portability.
        $absolute_path = parse_url($GLOBALS['base_url'], PHP_URL_PATH) . '/';

        $url = file_create_url($result->uri);
        $path = str_replace($GLOBALS['base_url'] . '/', $absolute_path, $url);

        $matches[$path] = check_plain($result->filename) . ' (' . t('File')  . ')';
        $match_count++;
      }
    }

    // Get user matches.
    if (user_access('access user profiles') && $match_count < 10) {
      $results = db_select('users')
        ->fields('users', array('uid', 'name'))
        ->condition('name', db_like($string) . '%', 'LIKE')
        ->range(0, 10 - $match_count)
        ->orderBy('uid', 'DESC')
        ->execute();
      // We need to load the User object to user user_format_name().
      $uids = array();
      foreach ($results as $result) {
        $uids[] = $result->uid;
      }
      if (!empty($uids)) {
        $users = user_load_multiple($uids);
        foreach ($users as $user) {
          $path = backdrop_get_path_alias('user/' . $user->uid);
          $matches['/' . $path] = user_format_name($user) . ' (' . t('User') . ')';
          $match_count++;
        }
      }
    }

    // Get taxonomy term matches.
    if ($match_count < 10) {
      $results = db_select('taxonomy_term_data')
        ->fields('taxonomy_term_data', array('tid', 'name'))
        ->condition('name', db_like($string) . '%', 'LIKE')
        ->addTag('taxonomy_term_access')
        ->range(0, 10 - $match_count)
        ->orderBy('tid', 'DESC')
        ->execute();
      foreach ($results as $result) {
        $path = backdrop_get_path_alias('taxonomy/term/' . $result->tid);
        $matches['/' . $path] = check_plain($result->name) . ' (' . t('Taxonomy term') . ')';
        $match_count++;
      }
    }

    // Get Views page matches.
    if ($match_count < 10) {
      $views = views_get_enabled_views();
      foreach ($views as $view) {
        foreach ($view->display as $display_id => $display) {
          if ($view->access($display_id) && $display->display_plugin == 'page' && !empty($display->display_options['path'])) {
            $path = backdrop_get_path_alias($display->display_options['path']);
            if (!path_is_admin($path)) {
              if (stripos($view->human_name, $string) !== FALSE || (!empty($display->title) && stripos($display->title, $string) !== FALSE)) {
                $view_display_title = (!empty($display->title)) ? $display->title : check_plain($view->human_name) . ' : ' . $display_id;
                $matches['/' . $path] = $view_display_title . ' (' . t('Views page') . ')';
                $match_count++;
              }
            }
          }
          if ($match_count == 10) {
            break 2;
          }
        }
      }
    }

    // Load layout paths
    if ($match_count < 10) {
      $menu_items = layout_get_all_configs('menu_item');
      foreach ($menu_items as $menu_item) {
        if (stripos($menu_item['name'], $string) !== FALSE && $match_count < 10) {
          $matches['/' . $menu_item['path']] = check_plain($menu_item['name']) . ' (' . t('Layout path') . ')';
          $match_count++;
        }
      }
    }
  }
  // Allow modules to alter the matches.
  backdrop_alter('link_autocomplete', $matches, $string);
  backdrop_json_output($matches);
}

/**
 * Form callback: Display a form for inserting/editing an image.
 */
function filter_format_editor_image_form($form, &$form_state, $format) {
  $form_state['format'] = $format;

  // Record the dialog selector that needs to be closed if present.
  if (isset($form_state['input']['dialogOptions']['target'])) {
    $form_state['storage']['dialog_selector'] = $form_state['input']['dialogOptions']['target'];
  }

  // Record if image uploading is requested by the calling element.
  $element_supports_uploads = !isset($form_state['input']['dialogOptions']['uploads']) || (bool) $form_state['input']['dialogOptions']['uploads'];

  // Pull in any default values set by the editor.
  $values = array();
  if (isset($form_state['input']['editor_object'])) {
    $values = $form_state['input']['editor_object'];
  }

  // Set the dialog title and submit button label.
  if (!empty($values['src']) || !empty($form_state['storage']['editing_image'])) {
    // Remember this is editing, not inserting. In the event the image is
    // removed and re-uploaded.
    $form_state['storage']['editing_image'] = TRUE;
    backdrop_set_title(t('Edit image'));
    $submit_button_label = t('Update');
  }
  else {
    backdrop_set_title(t('Insert image'));
    $submit_button_label = t('Insert');
  }

  // Construct strings to use in the upload validators.
  $upload_settings = isset($format->editor_settings['image_upload']) ? $format->editor_settings['image_upload'] : array();
  $upload_settings += array(
    'status' => 0,
    'dimensions' => array('max_width' => '', 'max_height' => ''),
    'max_size' => NULL,
    'scheme' => 'public',
    'directory' => 'inline-images',
  );

  if (!empty($upload_settings['max_dimensions']['width']) || !empty($upload_settings['max_dimensions']['height'])) {
    $max_dimensions = $upload_settings['max_dimensions']['width'] . 'x' . $upload_settings['max_dimensions']['height'];
  }
  else {
    $max_dimensions = 0;
  }
  $max_filesize = !empty($upload_settings['max_size']) ? min(parse_size($upload_settings['max_size']), file_upload_max_size()) : file_upload_max_size();
  $orientation = !empty($upload_settings['orientate']) ? $upload_settings['orientate'] : 0;
  $existing_file = !empty($values['data-file-id']) ? file_load($values['data-file-id']) : NULL;
  $fid = $existing_file ? $existing_file->fid : NULL;
  $form['image']['fid'] = array(
    '#title' => t('Upload an image'),
    '#type' => 'managed_file',
    '#upload_location' => $upload_settings['scheme'] . '://' . $upload_settings['directory'],
    '#default_value' => $fid ? $fid : NULL,
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
      'file_validate_image_orientation' => array($orientation),
      'file_validate_size' => array($max_filesize),
      'file_validate_image_resolution' => array($max_dimensions),
    ),
    '#wrapper_attributes' => array(
      'data-editor-image-toggle' => t('Upload an image'),
    ),
    '#parents' => array('fid'),
    '#weight' => -10,
    '#access' => $element_supports_uploads && $upload_settings['status'] && user_access('upload editor images'),
  );

  // Use a "textfield" rather than "url" to allow relative paths.
  $form['image']['src'] = array(
    '#title' => t('Select from library'),
    '#type' => 'textfield',
    '#element_validate' => array('_filter_format_editor_link_url_validate'),
    '#placeholder' => '/example/image.jpg',
    '#default_value' => isset($values['src']) ? $values['src'] : NULL,
    '#parents' => array('attributes', 'src'),
    '#wrapper_attributes' => array(
      'data-editor-image-toggle' => t('Select from library'),
    ),
    '#weight' => -8,
  );

  // If no current value or an existing file exists, default to showing
  // the uploading interface.
  if ($fid || empty($form['image']['src']['#default_value'])) {
    $form['image']['src']['#default_value'] = '';
  }

  $form['alt'] = array(
    '#title' => t('Alternative text'),
    '#type' => 'textfield',
    '#default_value' => isset($values['alt']) ? $values['alt'] : NULL,
    '#parents' => array('attributes', 'alt'),
  );
  $form['size'] = array(
    '#title' => t('Image size'),
    '#wrapper_attributes' => array('class' => array('editor-image-size')),
    '#theme_wrappers' => array('form_element'),
  );
  $form['size']['width'] = array(
    '#title' => t('Width'),
    '#title_display' => 'attribute',
    '#type' => 'number',
    '#default_value' => !empty($values['width']) ? $values['width'] : '',
    '#max' => 99999,
    '#attributes' => array('placeholder' => t('width')),
    '#parents' => array('attributes', 'width'),
    '#field_suffix' => ' &times; ',
  );
  $form['size']['height'] = array(
    '#title' => t('Height'),
    '#title_display' => 'attribute',
    '#type' => 'number',
    '#default_value' => !empty($values['height']) ? $values['height'] : '',
    '#max' => 99999,
    '#attributes' => array('placeholder' => t('height')),
    '#parents' => array('attributes', 'height'),
    '#field_suffix' => ' ' . t('pixels')
  );
  $form['align'] = array(
    '#title' => t('Align'),
    '#type' => 'radios',
    '#default_value' => isset($values['data-align']) ? $values['data-align'] : 'none',
    '#options' => array(
      'none' => t('None'),
      'left' => t('Left'),
      'center' => t('Center'),
      'right' => t('Right'),
    ),
    '#wrapper_attributes' => array('class' => array('editor-image-align')),
    '#parents' => array('attributes', 'data-align'),
    '#access' => !empty($format->filters['filter_image_align']->status),
  );
  $form['caption'] = array(
    '#title' => t('Add a caption'),
    '#type' => 'checkbox',
    '#default_value' => (isset($values['data-has-caption']) && strcmp($values['data-has-caption'], 'false') !== 0) ? (bool) $values['data-has-caption'] : FALSE,
    '#parents' => array('attributes', 'data-has-caption'),
    '#access' => !empty($format->filters['filter_image_caption']->status),
  );

  // Incorporate the Library View in the editor dialog form.
  $form['library_view'] = array(
    '#markup' => '<div class="library-view" data-editor-library-view="true">' . views_embed_view('image_library') . '</div>',
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_button_label,
    '#ajax' => array(
      'callback' => 'filter_format_editor_dialog_save',
      'event' => 'click',
    ),
  );

  return $form;
}

/**
 * Submit handler for filter_format_editor_image_form().
 */
function filter_format_editor_image_form_submit($form, &$form_state) {
  // Convert any uploaded files from the FID values to the src attribute.
  if (!empty($form_state['values']['fid'])) {
    $fid = $form_state['values']['fid'];
    $file = file_load($fid);

    // Try to make a local path if possible for better portability.
    $absolute_path = parse_url($GLOBALS['base_url'], PHP_URL_PATH) . '/';
    $url = file_create_url($file->uri);
    $url = str_replace($GLOBALS['base_url'] . '/', $absolute_path, $url);

    $form_state['values']['attributes']['src'] = $url;
    $form_state['values']['attributes']['data-file-id'] = $fid;
    unset($form_state['values']['fid']);
  }
}

/**
 * Form callback: Display a form for inserting/editing a link or uploading a file.
 */
function filter_format_editor_link_form($form, &$form_state, $format) {
  $form_state['format'] = $format;
  // Record the dialog selector that needs to be closed if present.
  if (isset($form_state['input']['dialogOptions']['target'])) {
    $form_state['storage']['dialog_selector'] = $form_state['input']['dialogOptions']['target'];
  }

  // Record if file uploading is requested by the calling element.
  $element_supports_uploads = !isset($form_state['input']['dialogOptions']['uploads']) || (bool) $form_state['input']['dialogOptions']['uploads'];

  // Pull in any default values set by the editor.
  $values = array();
  if (isset($form_state['input']['editor_object'])) {
    $values = $form_state['input']['editor_object'];
  }

  // Set the dialog title.
  if (!empty($values['href'])) {
    backdrop_set_title(t('Edit link'));
  }
  else {
    backdrop_set_title(t('Insert link'));
  }

  // Use a "textfield" rather than "url" to allow relative paths.
  $form['href'] = array(
    '#title' => t('Link value'),
    '#type' => 'textfield',
    '#element_validate' => array('_filter_format_editor_link_url_validate'),
    '#placeholder' => ' ',
    '#default_value' => isset($values['href']) ? $values['href'] : NULL,
    '#parents' => array('attributes', 'href'),
    '#wrapper_attributes' => array(
      'data-editor-image-toggle' => t('Link to existing content'),
    ),
    '#weight' => -10,
    '#autocomplete_path' => 'filter-link/autocomplete',
    '#description' => t('Enter a known path, or start typing to get suggestions'),
  );


  // Construct strings to use in the upload validators.
  $upload_settings = isset($format->editor_settings['file_upload']) ? $format->editor_settings['file_upload'] : array();

  $upload_settings += array(
    'max_size' => '',
    'file_extensions' => 'txt',
    'directory' => 'inline-files',
    'data-file-id' => NULL,
    'status' => 0,
    'scheme' => 'public',
  );

  $max_filesize = !empty($upload_settings['max_size']) ? min(parse_size($upload_settings['max_size']), file_upload_max_size()) : file_upload_max_size();
  $file_extensions = $upload_settings['file_extensions'];
  $upload_validators = array(
    'file_validate_extensions' => array($file_extensions),
    'file_validate_size' => array($max_filesize),
  );
  $file_directory = $upload_settings['directory'];
  $existing_file = !empty($values['data-file-id']) ? file_load($values['data-file-id']) : NULL;
  $fid = $existing_file ? $existing_file->fid : NULL;
  $form['file']['fid'] = array(
    '#title' => t('Upload a file and link to it'),
    '#type' => 'managed_file',
    '#upload_location' => $upload_settings['scheme'] . '://' . $file_directory,
    '#default_value' => $fid ? $fid : NULL,
    '#upload_validators' => $upload_validators,
    '#wrapper_attributes' => array(
      'data-editor-image-toggle' => t('Upload a file and link to it'),
    ),
    '#parents' => array('fid'),
    '#weight' => -2,
    '#access' => $element_supports_uploads && $upload_settings['status'] && user_access('upload editor files'),
    '#description' => theme('file_upload_help', array('upload_validators' => $upload_validators)),
  );


  // If no current value or an existing file exists, default to showing
  // the uploading interface.
  if ($fid || empty($form['file']['src']['#default_value'])) {
    $form['file']['fid']['#weight'] = -10;
    $form['file']['src']['#default_value'] = '';
  }
  // Otherwise if editing an unmanaged file, show the raw URL field.
  else {
    $form['file']['src']['#weight'] = -10;
  }

  $form['text'] = array(
    '#title' => t('Link text'),
    '#type' => 'textfield',
    '#default_value' => isset($values['text']) ? $values['text'] : '',
    '#parents' => array('attributes', 'text'),
  );

  $form['target'] = array(
    '#title' => t('Open in new window'),
    '#type' => 'checkbox',
    '#return_value' => '_blank',
    '#default_value' => isset($values['target']) ? $values['target'] : FALSE,
    '#parents' => array('attributes', 'target'),
  );

  $form['class'] = array(
    '#title' => t('CSS classes'),
    '#description' => t('Separate each additional CSS class with a space.'),
    '#type' => 'textfield',
    '#default_value' => isset($values['class']) ? $values['class'] : '',
    '#parents' => array('attributes', 'class'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#ajax' => array(
      'callback' => 'filter_format_editor_dialog_save',
      'event' => 'click',
    ),
  );

  return $form;
}

/**
 * Validate handler for filter_format_editor_link_form().
 */
function filter_format_editor_link_form_validate($form, &$form_state) {
  if (isset($form_state['values']['attributes']['class'])) {
    $classes = explode(' ', $form_state['values']['attributes']['class']);
    $clean_classes = array();
    foreach ($classes as $class_name) {
      $clean_classes[] = backdrop_clean_css_identifier($class_name, array());
    }
    $form_state['values']['attributes']['class'] = implode(' ', $clean_classes);
  }
}

/**
 * Submit handler for filter_format_editor_link_form().
 */
function filter_format_editor_link_form_submit($form, &$form_state) {
  // Look for file and get fid from uri.
  $fid = 0;
  $uri = '';
  if (isset($form_state['values']['attributes']['href']) && (strlen($form_state['values']['attributes']['href']) > 1)){
    $uri = $form_state['values']['attributes']['href'];
    // Check for FID for this URI.
    $fid = _filter_get_file_id($uri);
  }
  // File upload will not have provided href but provides fid.
  elseif (isset($form_state['values']['fid'])) {
    $fid = $form_state['values']['fid'];
  }
  // If file is found get its URI.
  if ($fid > 0) {
    $file = file_load($fid);
    $uri = file_create_url($file->uri);
    // Try to make a local path if possible for better portability.
    // Absolute path is needed if Backdrop is installed in subdirectory.
    $absolute_path = parse_url($GLOBALS['base_url'], PHP_URL_PATH) . '/';

    $url = str_replace($GLOBALS['base_url'] . '/', $absolute_path, $uri);
    $form_state['values']['attributes']['data-file-id'] = $fid;
    $form_state['values']['attributes']['href'] = $url;
    unset($form_state['values']['fid']);

    // If no text was set, use the file name.
    if (strlen($form_state['values']['attributes']['text']) === 0) {
      $form_state['values']['attributes']['text'] = basename($url);
    }
  }
  else {
    // Modify references to other URIs if not an external link.
    $url = (substr($uri, 0, 4) == 'http') ? $uri : substr(base_path(), 0, -1) . $uri;
    $form_state['values']['attributes']['href'] = $url;
    // Remove any reference to a data file.
    unset($form_state['values']['fid']);
    $form_state['values']['attributes']['data-file-id'] = NULL;
  }
}

/**
 * Form AJAX callback. Sends the save editor AJAX command and closes the dialog.
 *
 * @see filter_format_editor_link_form()
 * @see filter_format_editor_image_form()
 */
function filter_format_editor_dialog_save($form, &$form_state) {
  $dialog_selector = '#backdrop-dialog';
  if (isset($form_state['storage']['dialog_selector'])) {
    $dialog_selector = $form_state['storage']['dialog_selector'];
  }

  $commands = array();
  $errors = form_get_errors();
  if (!empty($errors)) {
    $error_messages = theme('status_messages');
    $rendered_form = backdrop_render($form);
    $commands[] = ajax_command_remove('.editor-dialog .messages');
    $commands[] = ajax_command_replace('.editor-dialog form', $rendered_form);
    $commands[] = ajax_command_prepend('.editor-dialog .ui-dialog-content', $error_messages);
  }
  else {
    $commands[] = array(
      'command' => 'editorDialogSave',
      'values' => $form_state['values'],
    );
    $commands[] = ajax_command_close_dialog($dialog_selector);
  }
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Element validation function.
 */
function _filter_format_editor_link_url_validate(&$element, &$form_state) {
  $value = trim($element['#value']);

  // If Link module is available, automatically clean up URL, automatically
  // adding http:// or a prefixing slash if needed.
  if (module_exists('link') && $type = link_validate_url($value)) {
    $value = link_cleanup_url($value);
    $form_state['values']['type'] = $type;
    if ($type === LINK_INTERNAL && strpos($value, '/') !== 0) {
      $value = '/' . $value;
    }
  }

  form_set_value($element, $value, $form_state);

  // Unlike #type = 'url' validation, we allow both relative and absolute paths.
  if ($value !== '' && !valid_url($value, TRUE) && !valid_url($value, FALSE)) {
    form_error($element, t('The URL %url is not valid.', array('%url' => $value)));
  }
}

/**
 * Find a managed file ID from a url.
 */
function _filter_get_file_id($href) {
  $result = 0;

  // Extract file name and directory from $href.
  $pos = strrpos($href, "/");
  $filename = substr($href,$pos + 1);

  // Now find this file name in uri field to get fid.
  if (!empty($filename)) {
    $results = db_select('file_managed')
      ->fields('file_managed', array('fid', 'uri'))
      ->condition('uri', '%' . $filename, 'LIKE')
      ->orderBy('fid', 'DESC')
      ->execute();

    foreach ($results as $found) {
      $result = $found->fid;
    }
  }

  return $result;
}
