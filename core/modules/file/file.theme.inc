<?php
/**
 * @file
 * Theme functions for the File module.
 */

/**
 * Returns HTML for a managed file element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the file.
 *
 * @ingroup themeable
 */
function theme_file_managed_file($variables) {
  $element = $variables['element'];

  $attributes = array();
  if (isset($element['#id'])) {
    $attributes['id'] = $element['#id'];
  }
  if (!empty($element['#attributes']['class'])) {
    $attributes['class'] = (array) $element['#attributes']['class'];
  }
  $attributes['class'][] = 'form-managed-file';

  // This wrapper is required to apply JS behaviors and CSS styling.
  $output = '';
  $output .= '<div' . backdrop_attributes($attributes) . '>';
  $output .= backdrop_render_children($element);
  $output .= '</div>';
  return $output;
}

/**
 * Returns HTML for a link to a file.
 *
 * @param $variables
 *   An associative array containing:
 *   - file: A file object to which the link will be created.
 *   - icon_directory: (optional) A path to a directory of icons to be used for
 *     files. Defaults to the value of the "file_icon_directory" variable.
 *
 * @ingroup themeable
 */
function theme_file_link($variables) {
  $file = $variables['file'];
  $icon_directory = $variables['icon_directory'];
  $attributes = $variables['attributes'];

  $url = file_create_url($file->uri);

  // Human-readable names, for use as text-alternatives to icons.
  $mime_name = array(
    'application/msword' => t('Microsoft Office document icon'),
    'application/vnd.ms-excel' => t('Office spreadsheet icon'),
    'application/vnd.ms-powerpoint' => t('Office presentation icon'),
    'application/pdf' => t('PDF icon'),
    'video/quicktime' => t('Movie icon'),
    'audio/mpeg' => t('Audio icon'),
    'audio/wav' => t('Audio icon'),
    'image/jpeg' => t('Image icon'),
    'image/png' => t('Image icon'),
    'image/gif' => t('Image icon'),
    'application/zip' => t('Package icon'),
    'text/html' => t('HTML icon'),
    'text/plain' => t('Plain text icon'),
    'application/octet-stream' => t('Binary Data'),
  );

  $mimetype = file_get_mimetype($file->uri);

  $icon = theme('file_icon', array(
    // Requires a file entity, make sure it gets one.
    'file' => ($file instanceof File) ? $file : file_load($file->fid),
    'icon_directory' => $icon_directory,
    'alt' => !empty($mime_name[$mimetype]) ? $mime_name[$mimetype] : t('File'),
  ));

  // Set "type" attribute per anchor format described at
  // http://microformats.org/wiki/file-format-examples
  if (!isset($attributes['type'])) {
    $attributes['type'] = $file->filemime . '; length=' . $file->filesize;
  }

  $options = array(
    'attributes' => array(
      'type' => $file->filemime . '; length=' . $file->filesize,
    ),
    'attributes' => $attributes,
  );

  // Use the description as the link text if available.
  if (empty($file->description)) {
    $link_text = $file->filename;
  }
  else {
    $link_text = $file->description;
    $options['attributes']['title'] = check_plain($file->filename);
  }

  return '<span class="file">' . $icon . ' ' . l($link_text, $url, $options) . '</span>';
}

/**
 * Returns HTML for an image with an appropriate icon for the given file.
 *
 * @param $variables
 *   An associative array containing:
 *   - file: A file entity for which to make an icon.
 *   - alt: (optional) The alternative text to represent the icon in text-based
 *     browsers. Defaults to an empty string.
 *   - icon_directory: (optional) A path to a directory of icons to be used for
 *     files. Defaults to the value of the "file_icon_directory" variable.
 *
 * @ingroup themeable
 */
function theme_file_icon($variables) {
  $file = $variables['file'];
  $alt = $variables['alt'];
  $icon_directory = $variables['icon_directory'];

  $mime = check_plain($file->filemime);
  $icon_url = file_icon_url($file, $icon_directory);
  return '<img class="file-icon" alt="' . check_plain($alt) . '" title="' . $mime . '" src="' . $icon_url . '" />';
}

/**
 * Returns HTML for an individual file upload widget.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the widget.
 *
 * @ingroup themeable
 */
function theme_file_widget($variables) {
  $element = $variables['element'];
  $output = '';

  // The "form-managed-file" class is required for proper Ajax functionality.
  $output .= '<div class="file-widget form-managed-file clearfix">';
  if ($element['fid']['#value'] != 0) {
    // Add the file size after the file name.
    $element['filename']['#markup'] .= ' <span class="file-size">(' . format_size($element['#file']->filesize) . ')</span> ';
  }
  $output .= backdrop_render_children($element);
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for a group of file upload widgets.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the widgets.
 *
 * @ingroup themeable
 */
function theme_file_widget_multiple($variables) {
  $element = $variables['element'];

  // Special ID and classes for draggable tables.
  $weight_class = $element['#id'] . '-weight';
  $table_id = $element['#id'] . '-table';

  // Build up a table of applicable fields.
  $headers = array();
  $headers[] = t('File information');
  if ($element['#display_field']) {
    $headers[] = array(
      'data' => t('Display'),
      'class' => array('checkbox'),
    );
  }
  $headers[] = t('Weight');
  $headers[] = t('Operations');

  // Get our list of widgets in order (needed when the form comes back after
  // preview or failed validation).
  $widgets = array();
  foreach (element_children($element) as $key) {
    $widgets[] = &$element[$key];
  }
  usort($widgets, '_field_sort_items_value_helper');

  $rows = array();
  foreach ($widgets as $key => &$widget) {
    // Save the uploading row for last.
    if ($widget['#file'] == FALSE) {
      $widget['#title'] = $element['#file_upload_title'];
      $widget['#description'] = $element['#file_upload_description'];
      continue;
    }

    // Delay rendering of the buttons, so that they can be rendered later in the
    // "operations" column.
    $operations_elements = array();
    foreach (element_children($widget) as $sub_key) {
      if (isset($widget[$sub_key]['#type']) && $widget[$sub_key]['#type'] == 'submit') {
        hide($widget[$sub_key]);
        $operations_elements[] = &$widget[$sub_key];
      }
    }

    // Delay rendering of the "Display" option and the weight selector, so that
    // each can be rendered later in its own column.
    if ($element['#display_field']) {
      hide($widget['display']);
    }
    hide($widget['_weight']);

    // Render everything else together in a column, without the normal wrappers.
    $widget['#theme_wrappers'] = array();
    $information = backdrop_render($widget);

    // Render the previously hidden elements, using render() instead of
    // backdrop_render(), to undo the earlier hide().
    $operations = '';
    foreach ($operations_elements as $operation_element) {
      $operations .= render($operation_element);
    }
    $display = '';
    if ($element['#display_field']) {
      unset($widget['display']['#title']);
      $display = array(
        'data' => render($widget['display']),
        'class' => array('checkbox'),
      );
    }
    $widget['_weight']['#attributes']['class'] = array($weight_class);
    $weight = render($widget['_weight']);

    // Arrange the row with all of the rendered columns.
    $row = array();
    $row[] = $information;
    if ($element['#display_field']) {
      $row[] = $display;
    }
    $row[] = $weight;
    $row[] = $operations;
    $rows[] = array(
      'data' => $row,
      'class' => isset($widget['#attributes']['class']) ? array_merge($widget['#attributes']['class'], array('draggable')) : array('draggable'),
    );
  }

  backdrop_add_tabledrag($table_id, 'order', 'sibling', $weight_class);

  $output = '';
  $output = empty($rows) ? '' : theme('table', array('header' => $headers, 'rows' => $rows, 'attributes' => array('id' => $table_id)));
  $output .= backdrop_render_children($element);
  return $output;
}


/**
 * Returns HTML for help text based on file upload validators.
 *
 * @param $variables
 *   An associative array containing:
 *   - description: The normal description for this field, specified by the
 *     user.
 *   - upload_validators: An array of upload validators as used in
 *     $element['#upload_validators'].
 *
 * @ingroup themeable
 */
function theme_file_upload_help($variables) {
  $description = (string) $variables['description'];
  $upload_validators = $variables['upload_validators'];

  $descriptions = array();

  if (strlen($description)) {
    $descriptions[] = token_replace($description);
  }
  if (isset($upload_validators['file_validate_size'])) {
    $descriptions[] = t('Files must be less than !size.', array('!size' => '<strong>' . format_size($upload_validators['file_validate_size'][0]) . '</strong>'));
  }
  if (isset($upload_validators['file_validate_extensions'])) {
    $descriptions[] = t('Allowed file types: !extensions.', array('!extensions' => '<strong>' . check_plain($upload_validators['file_validate_extensions'][0]) . '</strong>'));
  }
  if (isset($upload_validators['file_validate_image_resolution'])) {
    $max = $upload_validators['file_validate_image_resolution'][0];
    $min = $upload_validators['file_validate_image_resolution'][1];
    // Check for cases where only one dimension is specified.
    if ($min) {
      list($width, $height) = explode('x', $min);
      if (empty($height)) {
        $descriptions[] = t('Images must be at least !width pixels wide.', array('!width' => '<strong>' . $width . '</strong>'));
      }
      elseif (empty($width)) {
        $descriptions[] = t('Images must be at least !height pixels tall.', array('!height' => '<strong>' . $height . '</strong>'));
      }
      else {
        $descriptions[] = t('Images must be at least !min pixels.', array('!min' => '<strong>' . $min . '</strong>'));
      }
    }
    if ($max) {
      list($width, $height) = explode('x', $max);
      if (empty($height)) {
        $descriptions[] = t('Images wider than !width pixels will be scaled down.', array('!width' => '<strong>' . $width . '</strong>'));
      }
      elseif (empty($width)) {
        $descriptions[] = t('Images taller than !height pixels will be scaled down.', array('!height' => '<strong>' . $height . '</strong>'));
      }
      else {
        $descriptions[] = t('Images larger than !size pixels will be scaled down.', array('!size' => '<strong>' . $max . '</strong>'));
      }
    }
  }
  if (isset($upload_validators['file_validate_image_orientation'])) {
    $descriptions[] = t('If the need is indicated by EXIF data, the image will be rotated appropriately.');
  }
  return implode('<br />', $descriptions);
}

/**
 * Returns HTML for a file attachments table.
 *
 * @param $variables
 *   An associative array containing:
 *   - items: An array of file attachments.
 *
 * @ingroup themeable
 */
function theme_file_formatter_table($variables) {
  $header = array(t('Attachment'), t('Size'));
  $rows = array();
  foreach ($variables['items'] as $delta => $item) {
    $rows[] = array(
      theme('file_link', array('file' => (object) $item)),
      format_size($item['filesize']),
    );
  }

  return empty($rows) ? '' : theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Returns HTML for the file type overview page.
 *
 * Specifically, this returns HTML for a file type name and description.
 *
 * @deprecated as of 1.19.4. Labels are now output using the
 * theme_label_machine_name() function, and descriptions are displayed in a
 * separate column.
 */
function theme_file_type_overview($variables) {
  watchdog_deprecated_function('theme', __FUNCTION__);
  return check_plain($variables['name']) . '<div class="description">' . filter_xss_admin($variables['description']) . '</div>';
}

/**
 * Returns HTML for a file display's display order table.
 */
function theme_file_display_order($variables) {
  $element = $variables['element'];

  $rows = array();
  foreach (element_children($element, TRUE) as $name) {
    $element[$name]['weight']['#attributes']['class'][] = 'file-display-order-weight';
    $rows[] = array(
      'data' => array(
        backdrop_render($element[$name]['label']),
        backdrop_render($element[$name]['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $output = backdrop_render_children($element);
  $output .= theme('table', array('rows' => $rows, 'attributes' => array('id' => 'file-displays-order')));
  backdrop_add_tabledrag('file-displays-order', 'order', 'sibling', 'file-display-order-weight', NULL, NULL, TRUE);

  return $output;
}

/**
 * Copy of theme_file_link() for linking to the file download URL.
 *
 * @see theme_file_link()
 */
function theme_file_download_link($variables) {
  $file = $variables['file'];
  $uri = file_download_uri($file);

  // Human-readable names, for use as text-alternatives to icons.
  $mime_name = array(
    'application/msword' => t('Microsoft Office document icon'),
    'application/vnd.ms-excel' => t('Office spreadsheet icon'),
    'application/vnd.ms-powerpoint' => t('Office presentation icon'),
    'application/pdf' => t('PDF icon'),
    'video/quicktime' => t('Movie icon'),
    'audio/mpeg' => t('Audio icon'),
    'audio/wav' => t('Audio icon'),
    'image/jpeg' => t('Image icon'),
    'image/png' => t('Image icon'),
    'image/gif' => t('Image icon'),
    'application/zip' => t('Package icon'),
    'text/html' => t('HTML icon'),
    'text/plain' => t('Plain text icon'),
    'application/octet-stream' => t('Binary Data'),
  );

  $mimetype = file_get_mimetype($file->uri);

  $icon = theme('file_icon', array(
    'file' => $file,
    'icon_directory' => $variables['icon_directory'],
    'alt' => !empty($mime_name[$mimetype]) ? $mime_name[$mimetype] : t('File'),
  ));

  // Set options as per anchor format described at
  // http://microformats.org/wiki/file-format-examples
  $uri['options']['attributes']['type'] = $file->filemime . '; length=' . $file->filesize;

  // Provide the default link text.
  if (!isset($variables['text'])) {
    $variables['text'] = t('Download [file:name]');
  }

  // Perform unsanitized token replacement if $uri['options']['html'] is empty
  // since then l() will escape the link text.
  $variables['text'] = token_replace($variables['text'], array('file' => $file), array('clear' => TRUE, 'sanitize' => !empty($uri['options']['html'])));

  $output = '<span class="file">' . $icon . ' ' . l($variables['text'], $uri['path'], $uri['options']);
  $output .= ' <span class="file-size">(' . format_size($file->filesize) . ')</span>';
  $output .= '</span>';

  return $output;
}

/**
 * Returns HTML for displaying an HTML5 <audio> tag.
 *
 * @param array $variables
 *   An associative array containing:
 *   - file: Associative array of file data, which must include "uri".
 *   - controls: Boolean indicating whether or not controls should be displayed.
 *   - autoplay: Boolean indicating whether or not the audio should start
 *     playing automatically.
 *   - loop: Boolean indicating whether or not the audio should loop.
 *
 * @ingroup themeable
 */
function theme_file_audio($variables) {
  $files = $variables['files'];
  $output = '';

  $audio_attributes = array();
  if ($variables['controls']) {
    $audio_attributes['controls'] = 'controls';
  }
  if ($variables['autoplay']) {
    $audio_attributes['autoplay'] = 'autoplay';
  }
  if ($variables['loop']) {
    $audio_attributes['loop'] = 'loop';
  }
  if (!empty($variables['preload'])) {
    $audio_attributes['preload'] = $variables['preload'];
  }

  $output .= '<audio' . backdrop_attributes($audio_attributes) . '>';
  foreach ($files as $delta => $file) {
    $source_attributes = array(
      'src' => file_create_url($file['uri']),
      'type' => $file['filemime'],
    );
    $output .= '<source' . backdrop_attributes($source_attributes) . ' />';
  }
  $output .= '</audio>';
  return $output;
}

/**
 * Returns HTML for displaying an HTML5 <video> tag.
 *
 * @param array $variables
 *   An associative array containing:
 *   - file: Associative array of file data, which must include "uri".
 *   - controls: Boolean indicating whether or not controls should be displayed.
 *   - autoplay: Boolean indicating whether or not the video should start
 *     playing automatically.
 *   - loop: Boolean indicating whether or not the video should loop.
 *   - muted: Boolean indicating whether or not the sound should be muted.
 *   - width: Width, in pixels, of the video player.
 *   - height: Height, in pixels, of the video player.
 *
 * @ingroup themeable
 */
function theme_file_video($variables) {
  $files = $variables['files'];
  $output = '';

  $video_attributes = array();
  if ($variables['controls']) {
    $video_attributes['controls'] = 'controls';
  }
  if ($variables['autoplay']) {
    $video_attributes['autoplay'] = 'autoplay';
  }
  if ($variables['loop']) {
    $video_attributes['loop'] = 'loop';
  }
  if ($variables['muted']) {
    $video_attributes['muted'] = 'muted';
  }
  if ($variables['width']) {
    $video_attributes['width'] = $variables['width'];
  }
  if ($variables['height']) {
    $video_attributes['height'] = $variables['height'];
  }
  if (!empty($variables['preload'])) {
    $video_attributes['preload'] = $variables['preload'];
  }

  $output .= '<video' . backdrop_attributes($video_attributes) . '>';
  foreach ($files as $delta => $file) {
    $source_attributes = array(
      'src' => file_create_url($file['uri']),
      'type' => $file['filemime'],
    );
    $output .= '<source' . backdrop_attributes($source_attributes) . ' />';
  }
  $output .= '</video>';
  return $output;
}
