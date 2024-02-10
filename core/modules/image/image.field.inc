<?php
/**
 * @file
 * Implement an image field, based on the file module's file field.
 */

/**
 * Implements hook_field_info().
 */
function image_field_info() {
  return array(
    'image' => array(
      'label' => t('Image'),
      'description' => t('This field stores the ID of an image file as an integer value.'),
      'settings' => array(
        'uri_scheme' => config_get('system.core', 'file_default_scheme'),
        'default_image' => '',
      ),
      'instance_settings' => array(
        'file_extensions' => implode(' ', image_get_supported_extensions()),
        'file_directory' => '',
        'max_filesize' => '',
        'alt_field' => 1,
        'alt_field_required' => 0,
        'title_field' => 0,
        'max_dimensions' => '',
        'min_dimensions' => '',
        'default_image' => '',
        'orientate' => 0,
      ),
      'default_widget' => 'image_image',
      'default_formatter' => 'image',
      'default_token_formatter' => 'file_url_plain',
    ),
  );
}

/**
 * Implements hook_field_settings_form().
 */
function image_field_settings_form($field, $instance) {
  $defaults = field_info_field_settings($field['type']);
  $settings = array_merge($defaults, $field['settings']);

  $scheme_options = array();
  foreach (file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE) as $scheme => $stream_wrapper) {
    $scheme_options[$scheme] = $stream_wrapper['name'];
  }
  $form['uri_scheme'] = array(
    '#type' => 'radios',
    '#title' => t('Upload destination'),
    '#options' => $scheme_options,
    '#default_value' => $settings['uri_scheme'],
    '#description' => t('Select where the final files should be stored. Private file storage has significantly more overhead than public files, but allows restricted access to files within this field.'),
  );

  $form['default_image'] = array(
    '#element_validate' => array('_image_field_default_image_validate'),
    '#default_scheme' => $field['settings']['uri_scheme'],
    '#target' => array('field', 'settings', 'default_image'),
  );
  $form['default_image']['file'] = array(
    '#title' => t('Default image'),
    '#type' => 'file',
    '#description' => t('If no image is uploaded, this image will be shown on display.'),
  );
  if ($field['settings']['default_image']) {
    $link = l(basename($field['settings']['default_image']), file_create_url($field['settings']['default_image']), array('attributes' => array('target' => '_blank')));
    $form['default_image']['file']['#description'] .= '<br />' . t('Current image: !link', array('!link' => $link));

    $form['default_image']['remove'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remove default image'),
    );
  }

  return $form;
}

/**
 * Implements hook_field_instance_settings_form().
 */
function image_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];

  // Use the file field instance settings form as a basis.
  $form = file_field_instance_settings_form($field, $instance);

  $form['#element_validate'] = isset($form['#element_validate']) ? $form['#element_validate'] : array();
  $form['#element_validate'][] = 'image_field_instance_settings_form_validate';

  // Add maximum and minimum resolution settings.
  $max_dimensions = explode('x', $settings['max_dimensions']) + array('', '');
  $max_dimensions[0] = empty($max_dimensions[0]) ? NULL : $max_dimensions[0];
  $max_dimensions[1] = empty($max_dimensions[1]) ? NULL : $max_dimensions[1];
  $form['max_dimensions'] = array(
    '#type' => 'item',
    '#title' => t('Maximum image dimensions'),
    '#element_validate' => array('_image_field_resolution_validate'),
    '#weight' => 4.1,
    '#field_prefix' => '<div class="container-inline">',
    '#field_suffix' => '</div>',
    '#description' => t('Images larger than these dimensions (width × height) will be resized to these limits. This will cause the loss of <a href="http://en.wikipedia.org/wiki/Exchangeable_image_file_format">EXIF data</a> in the image. Leave blank for no restriction.'),
  );
  $form['max_dimensions']['x'] = array(
    '#type' => 'number',
    '#title' => t('Maximum width'),
    '#title_display' => 'invisible',
    '#default_value' => $max_dimensions[0],
    '#min' => 0,
    '#max' => 100000,
    '#field_suffix' => ' × ',
    '#placeholder' => t('Width'),
  );
  $form['max_dimensions']['y'] = array(
    '#type' => 'number',
    '#title' => t('Maximum height'),
    '#title_display' => 'invisible',
    '#default_value' => $max_dimensions[1],
    '#min' => 0,
    '#max' => 100000,
    '#field_suffix' => ' ' . t('pixels'),
    '#placeholder' => t('Height'),
  );

  $min_dimensions = explode('x', $settings['min_dimensions']) + array('', '');
  $min_dimensions[0] = empty($min_dimensions[0]) ? NULL : $min_dimensions[0];
  $min_dimensions[1] = empty($min_dimensions[1]) ? NULL : $min_dimensions[1];
  $form['min_dimensions'] = array(
    '#type' => 'item',
    '#title' => t('Minimum image dimensions'),
    '#element_validate' => array('_image_field_resolution_validate'),
    '#weight' => 4.2,
    '#field_prefix' => '<div class="container-inline">',
    '#field_suffix' => '</div>',
    '#description' => t('Images smaller than these dimensions (width × height) will be rejected. Leave blank for no restriction. '),
  );
  $form['min_dimensions']['x'] = array(
    '#type' => 'number',
    '#title' => t('Minimum width'),
    '#title_display' => 'invisible',
    '#default_value' => $min_dimensions[0],
    '#min' => 0,
    '#max' => 100000,
    '#field_suffix' => ' × ',
    '#placeholder' => t('Width'),
  );
  $form['min_dimensions']['y'] = array(
    '#type' => 'number',
    '#title' => t('Minimum height'),
    '#title_display' => 'invisible',
    '#default_value' => $min_dimensions[1],
    '#min' => 0,
    '#max' => 100000,
    '#field_suffix' => ' ' . t('pixels'),
    '#placeholder' => t('Height'),
  );

  // Remove the description option.
  unset($form['description_field']);

  // Add title and alt configuration options.
  $form['alt_field'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable <em>Alt</em> field'),
    '#default_value' => $settings['alt_field'],
    '#description' => t('The alt attribute may be used by search engines, screen readers, and when the image cannot be loaded.'),
    '#weight' => 10,
  );
  $form['alt_field_required'] = array(
    '#type' => 'checkbox',
    '#title' => t('<em>Alt</em> field required'),
    '#default_value' => !isset($settings['alt_field_required']) ? 0 : $settings['alt_field_required'],
    '#description' => t('Requiring alternative text is strongly encouraged to improve accessibility.'),
    '#indentation' => 1,
    '#weight' => 11,
    '#states' => array(
      'visible' => array(
        'input[name="instance[settings][alt_field]"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['title_field'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable <em>Title</em> field'),
    '#default_value' => $settings['title_field'],
    '#description' => t('The title attribute is used as a tooltip when the mouse hovers over the image. Enabling this field is not recommended, as it can cause problems with screen readers.'),
    '#weight' => 12,
  );
  $form['orientate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable image re-orientation'),
    '#default_value' => $settings['orientate'],
    '#weight' => 13,
    '#description' => t('If EXIF data indicates the need, the image will be rotated appropriately before being saved.'),
  );
  // Add a second line of description.
  $form['orientate']['#description'] .= '<br />' . t('Note that as with resizing, rotating images on upload will cause the loss of EXIF data in the image.');

  // Add the default image to the instance.
  $form['default_image'] = array(
    '#element_validate' => array('_image_field_default_image_validate'),
    '#default_scheme' => $field['settings']['uri_scheme'],
    '#target' => array('instance', 'settings', 'default_image'),
  );
  $form['default_image']['file'] = array(
    '#title' => t('Default image'),
    '#type' => 'file',
    '#description' => t("If no image is uploaded, this image will be shown on display and will override the field's default image."),
  );
  if ($settings['default_image']) {
    $link = l(basename($settings['default_image']), file_create_url($settings['default_image']), array('attributes' => array('target' => '_blank')));
    $form['default_image']['file']['#description'] .= '<br />' . t('Current image: !link', array('!link' => $link));

    $form['default_image']['remove'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remove default image'),
    );
  }

  return $form;
}

/**
 * Element validate handler for image_field_instance_settings_form().
 */
function image_field_instance_settings_form_validate($element, &$form_state, $complete_form) {
  $min_dimensions = explode('x', $form_state['values']['instance']['settings']['min_dimensions']) + array('', '');
  $max_dimensions = explode('x', $form_state['values']['instance']['settings']['max_dimensions']) + array('', '');

  $min_dimensions_x = (int) $min_dimensions[0];
  $min_dimensions_y = (int) $min_dimensions[1];
  $max_dimensions_x = (int) $max_dimensions[0];
  $max_dimensions_y = (int) $max_dimensions[1];

  // Ensure that min dimensions are not set to be bigger than max dimensions,
  // but only if max dimensions have been specified. This allows to set min
  // dimensions, without having to restrict max dimensions.
  if ((!empty($max_dimensions_x) && $min_dimensions_x > $max_dimensions_x) || (!empty($max_dimensions_y) && $min_dimensions_y > $max_dimensions_y)) {
    form_set_error('instance][settings][min_dimensions', t('The minimum image dimensions cannot be bigger than its maximum dimensions.'));
  }
}

/**
 * Element validate function for resolution fields.
 */
function _image_field_resolution_validate($element, &$form_state) {
  if (empty($element['x']['#value'])) {
    $element['x']['#value'] = 0;
  }
  if (empty($element['y']['#value'])) {
    $element['y']['#value'] = 0;
  }
  if (!empty($element['x']['#value']) || !empty($element['y']['#value'])) {
    form_set_value($element, $element['x']['#value'] . 'x' . $element['y']['#value'], $form_state);
  }
  else {
    form_set_value($element, '', $form_state);
  }
}

/**
 * Element validation function for setting a default image.
 */
function _image_field_default_image_validate($element, &$form_state) {
  // Save the file into the temporary directory.
  $file = file_save_upload(implode('_', $element['file']['#parents']), array('file_validate_is_image' => array()));

  if ($file) {
    // Move the file to the final location, and don't manage it in the database.
    // Because field configurations should not be tied to database auto-increment
    // IDs, we only track the URI.
    $directory = $element['#default_scheme'] . '://default_images/';
    file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
    $uri = file_unmanaged_move($file->uri, $directory);

    // Delete the file entity record.
    file_delete($file->fid);
  }
  if (isset($element['remove']) && $element['remove']['#value']) {
    $uri = '';
  }

  if (isset($uri)) {
    backdrop_array_set_nested_value($form_state['values'], $element['#target'], $uri, TRUE);
  }
  else {
    backdrop_array_unset_nested_value($form_state['values'], $element['#target']);
  }
}

/**
 * Implements hook_field_load().
 */
function image_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  file_field_load($entity_type, $entities, $field, $instances, $langcode, $items, $age);
}

/**
 * Implements hook_field_prepare_view().
 */
function image_field_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items) {
  // If there are no files specified at all, use the default.
  foreach ($entities as $id => $entity) {
    if (empty($items[$id])) {
      $default_uri = '';
      // Use the default for the instance if one is available.
      if (!empty($instances[$id]['settings']['default_image'])) {
        $default_uri = $instances[$id]['settings']['default_image'];
      }
      // Otherwise, use the default for the field.
      elseif ($field['settings']['default_image']) {
        $default_uri = $field['settings']['default_image'];
      }

      // Add the default image if one is found.
      if ($default_uri) {
        $items[$id][0] = (array) array(
          'uri' => $default_uri,
          'is_default' => TRUE,
          'alt' => '',
          'title' => '',
        );
      }
    }
  }
}

/**
 * Implements hook_field_presave().
 */
function image_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {

  // Determine the dimensions if necessary.
  foreach ($items as &$item) {
    if (!isset($item['width']) || !isset($item['height'])) {
      $info = image_get_info(file_load($item['fid'])->uri);

      if (is_array($info)) {
        $item['width'] = $info['width'];
        $item['height'] = $info['height'];
      }
    }
  }
}

/**
 * Implements hook_field_insert().
 */
function image_field_insert($entity_type, $entity, $field, $instance, $langcode, &$items) {
  file_field_insert($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implements hook_field_update().
 */
function image_field_update($entity_type, $entity, $field, $instance, $langcode, &$items) {
  file_field_update($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implements hook_field_delete().
 */
function image_field_delete($entity_type, $entity, $field, $instance, $langcode, &$items) {
  file_field_delete($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implements hook_field_delete_revision().
 */
function image_field_delete_revision($entity_type, $entity, $field, $instance, $langcode, &$items) {
  file_field_delete_revision($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implements hook_field_is_empty().
 */
function image_field_is_empty($item, $field) {
  return file_field_is_empty($item, $field);
}

/**
 * Implements hook_field_widget_info().
 */
function image_field_widget_info() {
  return array(
    'image_image' => array(
      'label' => t('Image'),
      'field types' => array('image'),
      'settings' => array(
        'browser_view' => 'image_library',
        'progress_indicator' => 'throbber',
        'preview_image_style' => 'thumbnail',
      ),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_settings_form().
 */
function image_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];

  // Use the file widget settings form.
  $form = file_field_widget_settings_form($field, $instance);

  $form['browser_view'] = array(
    '#title' => t('Enable image browser'),
    '#type' => 'checkbox',
    '#return_value' => 'image_library',
    '#default_value' => $settings['browser_view'],
    '#description' => t('Enable the image browser as an alternative to the normal upload.'),
    '#weight' => -3,
  );

  $form['preview_image_style'] = array(
    '#title' => t('Preview image style'),
    '#type' => 'select',
    '#options' => image_style_options(FALSE, PASS_THROUGH),
    '#empty_option' => '<' . t('no preview') . '>',
    '#default_value' => $settings['preview_image_style'],
    '#description' => t('The preview image will be shown while editing the content.'),
    '#weight' => 15,
  );

  return $form;
}

/**
 * Implements hook_field_widget_form().
 */
function image_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  // Add display_field setting to field because file_field_widget_form() assumes it is set.
  $field['settings']['display_field'] = 0;

  $elements = file_field_widget_form($form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
  $settings = $instance['settings'];

  foreach (element_children($elements) as $delta) {
    // Add adjustment of orientation before resizing because EXIF data is needed.
    if ($settings['orientate']) {
      $elements[$delta]['#upload_validators']['file_validate_image_orientation'] = array($settings['orientate']);
    }
    // Add upload resolution validation.
    if ($settings['max_dimensions'] || $settings['min_dimensions']) {
      $elements[$delta]['#upload_validators']['file_validate_image_resolution'] = array($settings['max_dimensions'], $settings['min_dimensions']);
    }

    // If not using custom extension validation, ensure this is an image.
    $supported_extensions = image_get_supported_extensions();
    backdrop_alter('image_supported_extensions', $supported_extensions);
    $extensions = isset($elements[$delta]['#upload_validators']['file_validate_extensions'][0]) ? $elements[$delta]['#upload_validators']['file_validate_extensions'][0] : implode(' ', $supported_extensions);
    $extensions = array_intersect(explode(' ', $extensions), $supported_extensions);
    $elements[$delta]['#upload_validators']['file_validate_extensions'][0] = implode(' ', $extensions);

    // Set any view used as a browser.
    if (!empty($instance['widget']['settings']['browser_view'])) {
      $elements[$delta]['#browser_view'] = $instance['widget']['settings']['browser_view'];
    }

    // Add all extra functionality provided by the image widget.
    $elements[$delta]['#process'][] = 'image_field_widget_process';
  }

  if ($field['cardinality'] == 1) {
    // If there's only one field, return it as delta 0.
    if (empty($elements[0]['#default_value']['fid'])) {
      $elements[0]['#description'] = theme('file_upload_help', array('description' => field_filter_xss($instance['description']), 'upload_validators' => $elements[0]['#upload_validators']));
    }
  }
  else {
    $elements['#file_upload_description'] = theme('file_upload_help', array('upload_validators' => $elements[0]['#upload_validators']));
  }
  return $elements;
}

/**
 * An element #process callback for the image_image field type.
 *
 * Expands the image_image type to include the alt and title fields.
 */
function image_field_widget_process($element, &$form_state, $form) {
  $item = $element['#value'];
  $item['fid'] = $element['fid']['#value'];

  $instance = field_widget_instance($element, $form_state);

  $settings = $instance['settings'];
  $widget_settings = $instance['widget']['settings'];

  $element['#theme'] = 'image_widget';
  $element['#attached']['css'][] = backdrop_get_path('module', 'image') . '/css/image.css';

  // Add the image preview.
  if ($element['#file'] && $widget_settings['preview_image_style']) {
    $variables = array(
      'style_name' => $widget_settings['preview_image_style'],
      'uri' => $element['#file']->uri,
    );

    // Determine image dimensions.
    if (isset($element['#value']['width']) && isset($element['#value']['height'])) {
      $variables['width'] = $element['#value']['width'];
      $variables['height'] = $element['#value']['height'];
    }
    else {
      $info = image_get_info($element['#file']->uri);

      if (is_array($info)) {
        $variables['width'] = $info['width'];
        $variables['height'] = $info['height'];
      }
      else {
        $variables['width'] = $variables['height'] = NULL;
      }
    }

    $element['preview'] = array(
      '#type' => 'markup',
      '#markup' => theme('image_style', $variables),
    );

    // Store the dimensions in the form so the file doesn't have to be accessed
    // again. This is important for remote files.
    $element['width'] = array(
      '#type' => 'hidden',
      '#value' => $variables['width'],
    );
    $element['height'] = array(
      '#type' => 'hidden',
      '#value' => $variables['height'],
    );
  }

  // Add the additional alt and title fields.
  $element['alt'] = array(
    '#title' => t('Alternate text'),
    '#type' => 'textfield',
    '#default_value' => isset($item['alt']) ? $item['alt'] : '',
    '#description' => t('This text will be used by screen readers, search engines, or when the image cannot be loaded.'),
    // @see http://www.gawds.org/show.php?contentid=28
    '#maxlength' => 512,
    '#weight' => -2,
    '#access' => (bool) $item['fid'] && $settings['alt_field'],
  );
  $element['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => isset($item['title']) ? $item['title'] : '',
    '#description' => t('The title is used as a tool tip when the user hovers the mouse over the image.'),
    '#maxlength' => 1024,
    '#weight' => -1,
    '#access' => (bool) $item['fid'] && $settings['title_field'],
  );

  if ($settings['alt_field_required']) {
    // Adding #required directly to the element would trigger unnecessary
    // validation errors when uploading the image before submitting the form.
    // Rather than adding manual markup to the #title to mimic the required
    // asterisk indication, we are adding an element #pre_render attribute,
    // and then adding the #required property to the function that it calls.
    // That way, it is rendered, but it doesn't actually trigger any validation.
    $element['alt']['#pre_render'] = array('_image_field_widget_alt_pre_render');
    $element['alt']['#element_validate'] = array('_image_field_widget_alt_validate');
    // Pass the alt required status as a data-* attribute on the form element,
    // in case other modules need access to it.
    $element['alt']['#attributes'] = array('data-alt-required' => 'true');
  }

  return $element;
}

/**
 * Pre-render callback for alt field.
 *
 * Mark a field as #required for display only.
 */
function _image_field_widget_alt_pre_render($element) {
  $element['#required'] = TRUE;
  return $element;
}

/**
 * Validate callback for alt field.
 *
 * This is separated in a validate function instead of a #required flag to avoid
 * being validated on the process callback.
 */
function _image_field_widget_alt_validate($element, &$form_state) {
  // Only do validation if the function is triggered from other places than
  // the image process form.
  if (isset($form_state['triggering_element']['#submit']) && !in_array('file_managed_file_submit', $form_state['triggering_element']['#submit'])) {
    // If the image is not there, we do not check for empty values.
    $parents = $element['#parents'];
    $field = array_pop($parents);
    $image_field = backdrop_array_get_nested_value($form_state['input'], $parents);
    if ($image_field == NULL) {
      $image_field = array();
    }
    // We check for the array key, so that it can be NULL (like if the user
    // submits the form without using the "upload" button).
    if (!array_key_exists($field, $image_field)) {
      return;
    }
    // Check if the field is left empty.
    elseif (empty($image_field[$field])) {
      form_error($element, t('The %title field is required.', array('%title' => $element['#title'])));
      return;
    }
  }
}

/**
 * Implements hook_field_formatter_info().
 */
function image_field_formatter_info() {
  $formatters = array(
    'image' => array(
      'label' => t('Image'),
      'field types' => array('image'),
      'settings' => array(
        'image_style' => '',
        'image_link' => '',
        'image_float' => '',
        'image_load' => 'auto',
      ),
	  'description' => t('Format the file as an image. The image can be displayed using an image style and can optionally be linked to the image file itself or its parent content.'),
	  'file formatter' => array(
        'mime types' => array('image/*'),
      ),
    ),
  );

  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function image_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $image_styles = image_style_options(FALSE, PASS_THROUGH);
  $element['image_style'] = array(
    '#title' => t('Image style'),
    '#type' => 'select',
    '#default_value' => $settings['image_style'],
    '#empty_option' => t('None (original image)'),
    '#options' => $image_styles,
  );

  $link_types = array(
    'content' => t('Content'),
    'file' => t('File'),
  );
  $element['image_link'] = array(
    '#title' => t('Link image to'),
    '#type' => 'select',
    '#default_value' => $settings['image_link'],
    '#empty_option' => t('Nothing'),
    '#options' => $link_types,
  );

  $element['image_float'] = array(
    '#title' => t('Float image'),
    '#type' => 'select',
    '#description' => t('Display the image on the left or right, with the rest of the content wrapping around it.'),
    '#default_value' => $settings['image_float'],
    '#empty_option' => t('No'),
    '#options' => array(
      'left' => t('Left'),
      'right' => t('Right'),
    ),
  );

  if (!isset($settings['image_load'])) {
    $settings['image_load'] = 'auto';
  }

  $element['image_load'] = array(
    '#title' => t('Image loading'),
    '#type' => 'radios',
    '#default_value' => $settings['image_load'],
    '#options' => array(
      'lazy' => t('Lazy'),
      'auto' => t('Automatic'),
      'eager' => t('Eager'),
    ),
  );
  $element['image_load']['lazy']['#description'] = t('Defer loading offscreen images until viewers scroll nearby. "Nearby" is determined by each browser.');
  $element['image_load']['auto']['#description'] = t('Each browser will determine whether or not to lazily load the image.');
  $element['image_load']['eager']['#description'] = t('Load this image immediately, regardless of where it is located on the page.');

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function image_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $summary = array();

  $image_styles = image_style_options(FALSE, PASS_THROUGH);
  // Unset possible 'No defined styles' option.
  unset($image_styles['']);
  // Styles could be lost because of enabled/disabled modules that defines
  // their styles in code.
  if (isset($image_styles[$settings['image_style']])) {
    $summary[] = t('Image style: @style', array('@style' => $image_styles[$settings['image_style']]));
  }
  else {
    $summary[] = t('Original image');
  }

  $link_types = array(
    'content' => t('Linked to content'),
    'file' => t('Linked to file'),
  );
  // Display this setting only if image is linked.
  if (isset($link_types[$settings['image_link']])) {
    $summary[] = $link_types[$settings['image_link']];
  }

  // Display this setting only if image is floated.
  if (!empty($settings['image_float'])) {
    $summary[] = t('Floated: @side', array('@side' => $settings['image_float']));
  }

  if (!empty($settings['image_load']) && $settings['image_load'] != 'auto') {
    $summary[] = t('Loading: @speed', array('@speed' => $settings['image_load']));
  }

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function image_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  // Check if the formatter involves a link.
  if ($display['settings']['image_link'] == 'content') {
    $uri = entity_uri($entity_type, $entity);
  }
  elseif ($display['settings']['image_link'] == 'file') {
    $link_file = TRUE;
  }

  foreach ($items as $delta => $item) {
    // Add class for floating the image
    if (!empty($display['settings']['image_float'])) {
      $item['attributes']['class'][] = 'align-' . $display['settings']['image_float'];
    }

    if (!empty($display['settings']['image_load'])) {
      // Although available in Chromium, the 'auto' value is not mentioned in
      // the specification. Since it may be subject to change, we recommend not
      // to use it until it gets officially included.
      if ($display['settings']['image_load'] != 'auto') {
        $item['attributes']['loading'] = $display['settings']['image_load'];
      }
    }

    if (isset($link_file)) {
      $uri = array(
        'path' => file_create_url($item['uri']),
        'options' => array(),
      );
    }
    $element[$delta] = array(
      '#theme' => 'image_formatter',
      '#item' => $item,
      '#image_style' => $display['settings']['image_style'],
      '#path' => isset($uri) ? $uri : '',
    );
  }

  return $element;
}
