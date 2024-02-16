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
    backdrop_set_title(t('Edit image details'));
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
    'alt_required' => FALSE,
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
      'file_validate_extensions' => array(implode(' ', image_get_supported_extensions())),
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
    '#maxlength' => 256,
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
    '#description' => t('The alt attribute may be used by screen readers, search engines, and when the image cannot be loaded.'),
    '#parents' => array('attributes', 'alt'),
    '#required' => $upload_settings['alt_required'],
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

  // This button is hidden from display and clicked via JavaScript in
  // Backdrop.behaviors.editorImageDialog in filter.js.
  $form['library_open'] = array(
    '#type' => 'submit',
    '#value' => t('Select from library'),
    '#name' => 'library_open',
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
    '#ajax' => array(
      'callback' => '_filter_image_library_ajax',
      'event' => 'click',
    ),
    '#limit_validation_errors' => array(),
    '#validate' => array(),
    '#submit' => array(),
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
 * Render the image library.
 */
function _filter_image_library_ajax() {
  $view_name = 'image_library';
  $view = views_embed_view($view_name);
  if (empty($view)) {
    $view = t('The view "@view_name" is not available.', array('@view_name' => $view_name));
    if (user_access('administer views')) {
      $view .= ' ' . t('Check the <a href="!url">Views administration page</a> to enable or create the "@view_name" view as needed.', array('!url' => url('admin/structure/views'), '@view_name' => $view_name));
    }
  }

  $rendered_library = '<div class="library-view" data-editor-library-view="true">' . $view . '</div>';
  $error_messages = theme('status_messages');

  $commands = array();
  $commands[] = ajax_command_prepend('.editor-image-library', $error_messages . $rendered_library);

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
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
    '#title' => t('Link destination'),
    '#type' => 'textfield',
    '#element_validate' => array('_filter_format_editor_link_url_validate'),
    '#placeholder' => ' ',
    '#default_value' => isset($values['href']) ? $values['href'] : NULL,
    '#parents' => array('attributes', 'href'),
    '#wrapper_attributes' => array(
      'data-editor-image-toggle' => t('Link to existing content'),
    ),
    '#weight' => -10,
    '#maxlength' => 256,
    '#autocomplete_path' => 'path-autocomplete',
    '#description' => t('Start typing to see a list of suggestions, enter the relative path to another page, or enter an external URL.<br>Examples: <em>About Us</em>, <em>/about</em>, or <em>http://backdropcms.org</em>'),
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

  $form['more'] = array(
    '#title' => t('More link options'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['more']['target'] = array(
    '#title' => t('Open the link destination in a new window'),
    '#type' => 'checkbox',
    '#return_value' => '_blank',
    '#default_value' => isset($values['target']) ? $values['target'] : FALSE,
    '#parents' => array('attributes', 'target'),
  );

  $form['more']['class'] = array(
    '#title' => t('CSS classes'),
    '#description' => t('Separate multiple CSS classes with spaces.'),
    '#type' => 'textfield',
    '#default_value' => isset($values['class']) ? $values['class'] : '',
    '#parents' => array('attributes', 'class'),
  );

  $fragment_info_link = l(t('fragment'), 'https://en.wikipedia.org/wiki/Fragment_identifier', array('attributes' => array('target' => '_blank')));
  $form['more']['id'] = array(
    '#title' => t('Unique Identifier'),
    '#description' => t('Setting an ID allows linking to this part of the page using a URL !fragment.', array('!fragment' => $fragment_info_link)),
    '#type' => 'textfield',
    '#default_value' => isset($values['id']) ? $values['id'] : '',
    '#parents' => array('attributes', 'id'),
  );

  $form['more']['title'] = array(
    '#title' => t('Link title'),
    '#description' => t('The title attribute of the link. Usually shown as a small tooltip on hover.'),
    '#type' => 'textfield',
    '#default_value' => isset($values['title']) ? $values['title'] : '',
    '#parents' => array('attributes', 'title'),
  );

  $form['more']['rel'] = array(
    '#title' => t('Relationship (rel)'),
    '#description' => t('Specifies the relationship between the current page and the linked page.'),
    '#type' => 'textfield',
    '#default_value' => isset($values['rel']) ? $values['rel'] : '',
    '#parents' => array('attributes', 'rel'),
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
    $form_state['values']['attributes']['href'] = $uri;
    // Remove any reference to a data file.
    unset($form_state['values']['fid']);
    $form_state['values']['attributes']['data-file-id'] = NULL;
  }

  // Clean up optional attributes, if not specified, remove.
  $more = array('class', 'target', 'id', 'title', 'rel', 'data-file-id');
  foreach ($more as $attribute) {
    if (isset($form_state['values']['attributes'][$attribute])) {
      $value = trim($form_state['values']['attributes'][$attribute]);
      if (strlen($value) === 0 || ($attribute === 'target' && empty($value))) {
        unset($form_state['values']['attributes'][$attribute]);
      }
      else {
        $form_state['values']['attributes'][$attribute] = $value;
      }
    }
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
    $commands[] = ajax_command_replace('.editor-dialog form.filter-format-editor-image-form', $rendered_form);
    $commands[] = ajax_command_prepend('.editor-dialog .ui-dialog-content', $error_messages);
    // Check whether this is an image upload, a library selection, or a link to
    // another non-library file. Note that classes "first" and "last" are
    // dynamically assigned, so are not consistently on the same link.
    if (empty($form_state['input']['fid']['fid'])) {
      // Custom link to a non-library file.
      $commands[] = ajax_command_invoke('.editor-dialog form.filter-format-editor-image-form a.last', 'click');
    }
    else {
      // Library file has been selected.
      $commands[] = ajax_command_invoke('.editor-dialog form.filter-format-editor-image-form a.first', 'click');
    }
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
 *
 * Clean up a URL and see if it will pass validation. Instead of stripping
 * dangerous protocols we flag them as invalid to make the user aware.
 *
 * @see filter_format_editor_link_form()
 * @see filter_format_editor_image_form()
 */
function _filter_format_editor_link_url_validate(&$element, &$form_state) {
  $value = trim($element['#value']);

  // Allow empty values, since this will not result in a link tag anyway.
  if ($value === '') {
    form_set_value($element, $value, $form_state);
    return;
  }

  // Clean up the URL so it's more likely to pass validation.
  $value = _filter_cleanup_url($value);

  form_set_value($element, $value, $form_state);

  // Flag if it has any dangerous protocols.
  $stripped_url = backdrop_strip_dangerous_protocols($value);
  $dangerous_protocols = $stripped_url !== $value ? TRUE : FALSE;

  // If it has dangerous protocols the URL is invalid. Or if it isn't either
  // a valid relative nor a valid absolute path.
  if ($dangerous_protocols || (!valid_url($value, TRUE) && !valid_url($value, FALSE))) {
    form_error($element, t('The URL %url is not valid.', array('%url' => $value)));
  }
}

/**
 * Clean up the URL for validation.
 *
 * Attempts to create a URL that will pass validation: add a protocol if it
 * looks like a domain but the protocol is missing; or create a valid, encoded
 * internal alias.
 *
 * @param string $url
 *   A URL before it is cleaned-up.
 *
 * @return string
 *   A cleaned-up URL that is either a properly formatted external URL or an
 *   internal path.
 *
 * @see _filter_format_editor_link_url_validate()
 */
function _filter_cleanup_url($url) {
  // Will add a default protocol if missing on an external url.
  $url = _filter_create_valid_external_url($url);

  // If it's an external URL with a protocol we can skip the rest of the
  // clean up.
  if (_filter_url_has_protocol($url)) {
    return $url;
  }

  // Return anchors as is.
  if (substr($url, 0, 1) == '#') {
    return $url;
  }

  // Return if it begins with a slash. Avoids issues such as:
  // - extra language prefixes if it's a multilingual site;
  // - adding extra prefixes if being re-edited;
  // - a url getting double-encoded;
  // - or domains with relative protocols.
  // E.g. /fr/node/1, //domain.tld.
  if (substr($url, 0, 1) == '/') {
    return $url;
  }

  // Break up the components so we can build up an aliased internal path.
  $url_parts = backdrop_parse_url($url);

  return url($url_parts['path'], $url_parts);
}

/**
 * Forms a valid external URL if possible.
 *
 * Automatically adds an http:// to addresses that appear to be external URLs
 * but have no specified protocol.
 *
 * @param string $url
 *   The URL entered by the user.
 *
 * @return string
 *   The URL with the protocol prepended if needed.
 *
 * @see link_cleanup_url()
 */
function _filter_create_valid_external_url($url) {
  // Skip if it's already a valid external URL with a protocol.
  if (_filter_url_has_protocol($url)) {
    return $url;
  }

  // If it looks like a domain name but it's missing a protocol, add a protocol.
  // The will catch domains like archive.org but will skip a valid external
  // domains with a relative protocol like //archive.org.
  $domain_match = preg_match('/^(([a-z0-9]([a-z0-9\-_]*\.){1,63})([a-z]{2,63}))/i', $url);
  if (!empty($domain_match)) {
    // We add a relative protocol since we don't know if the address requires
    // http or https.
    $url = '//' . $url;
  }

  return $url;
}

/**
 * Checks if a URL has a protocol.
 *
 * Checks for a protocol as a sign that the URL is external. We want to include
 * all external URLs even if they have dangerous protocols.
 *
 * @param string $url
 *   The URL entered by the user.
 *
 * @return bool
 *   TRUE if URL has protocol.
 */
function _filter_url_has_protocol($url) {
  // If it has a scheme then it has a protocol.
  if (parse_url($url, PHP_URL_SCHEME)) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Find a managed file ID from a url.
 */
function _filter_get_file_id($href) {
  $result = 0;

  // Make sure this is a file, either in public or in private path.
  global $base_path;
  $public_path = config_get('system.core', 'file_public_path');
  // Private files have "system/files/" in their URL.
  // @see BackdropPrivateStreamWrapper::getExternalUrl()
  $pattern = '#^' . $base_path . '(\?q=)?(' . $public_path . '|system/files/' . ')#';
  if (preg_match($pattern, $href) === FALSE) {
    return $result;
  }

  // Extract file name and directory from $href.
  $pos = strrpos($href, "/");
  $filename = substr($href, $pos + 1);

  // Now find this file name in uri field to get fid.
  if (!empty($filename)) {
    $results = db_select('file_managed')
      ->fields('file_managed', array('fid', 'uri'))
      ->condition('uri', '%/' . $filename, 'LIKE')
      ->orderBy('fid', 'DESC')
      ->execute();

    foreach ($results as $found) {
      $result = $found->fid;
    }
  }

  return $result;
}
