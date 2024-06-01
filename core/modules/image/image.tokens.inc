<?php
/**
 * @file
 * Builds placeholder replacement tokens for image styles.
 */

/**
 * Implements hook_token_info().
 */
function image_token_info() {
  $styles = image_styles();
  $field_tokens = array();
  $style_tokens = array();

  foreach ($styles as $style => $definition) {
    $field_tokens[$style] = array(
      'name' => $style,
      'description' => t('@style image style', array('@style' => $style)),
      'type' => 'image-style',
    );
    foreach (_image_style_token_image_attributes() as $attribute => $none) {
      $style_tokens[$attribute] = array(
        'name' => $attribute,
        'description' => t('@style image style attribute: @attribute', array('@style' => $style, '@attribute' => $attribute)),
      );
    }
  }

  return array(
    'types' => array(
      'image-field' => array(
        'name' => 'Image field',
        'description' => 'Image field',
        'needs-data' => 'image-field',
      ),
      'image-style' => array(
        'name' => 'Image style',
        'description' => 'Image style',
        'needs-data' => 'image-style',
      ),
    ),
    'tokens' => array(
      'image-field' => $field_tokens,
      'image-style' => $style_tokens,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function image_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  if (isset($data[$type]) && $entity = $data[$type]) {
    $supported_types = array();
    $entity_info = entity_get_info();
    foreach ($entity_info as $machine => $details) {
      if ($details['fieldable']) {
        $supported_types[] = $machine;
      }
    }

    if (in_array($type, $supported_types) && !empty($entity)) {
      $fields = field_info_field_map();

      // Check to see which fields are supported.
      foreach ($fields as $field_name => $field) {
        $field['field_name'] = $field_name;
        if ($field['type'] == 'image') {
          $image_field_tokens = token_find_with_prefix($tokens, $field['field_name']);

          $field_object = field_get_items($type, $entity, $field['field_name']);
          if ($image_field_tokens && $field_object) {
            $replacements += token_generate('image-field', $image_field_tokens, array('image-field' => $field_object), $options);
          }
        }
      }
    }

    $images_style_data = &backdrop_static(__FUNCTION__);

    if ($type == 'image-field' && !empty($data['image-field'])) {
      foreach ($tokens as $token => $original) {
        $output = array();

        // The token will be in the format "[style]:[attribute]" so split into
        // the individual pieces.
        $explode = explode(':', $token);
        $type = isset($explode[0]) ? $explode[0] : FALSE;
        $attribute = isset($explode[1]) ? $explode[1] : FALSE;

        // Special handling for the 'first', 'second', 'third', and 'last' tokens.
        // Support for index of multi-value fields.
        if (in_array($attribute, array('first', 'second', 'third', 'last')) || is_numeric($attribute)) {
          switch ($attribute) {
            case 'first':
              $index = 0;
              break;

            case 'second':
              $index = 1;
              break;

            case 'third':
              $index = 2;
              break;

            case 'last':
              $index = sizeof($data['image-field']) - 1;
              break;

            default:
              $index = $attribute;
          }

          if (isset($data['image-field'][$index]) && !empty($data['image-field'][$index]['uri'])) {
            if (isset($explode[3]) && $explode[3] === 'relative') {
              $url  = parse_url(image_style_url($type, $data['image-field'][$index]['uri']));
              $output[] = $url['path'];
            }
            else {
              $output[] = image_style_url($type, $data['image-field'][$index]['uri']);
            }

            // Generate image if does not exist. Because other attributes like
            // width and height are dependent on the image existing, we must
            // generate the image so that subsequent tokens can be populated.
            $styled_uri = image_style_path($type, $data['image-field'][$index]['uri']);
            _image_token_generate_image($styled_uri, $type, $data['image-field'][$index]['uri']);
          }

          $replacements[$original] = implode('', $output);
        }

        // Other tokens.
        else {
          foreach ($data['image-field'] as $field) {
            if (isset($attribute) && !empty($attribute)) {
              // Generate image if does not exist.
              $styled_uri = image_style_path($type, $field['uri']);
              _image_token_generate_image($styled_uri, $type, $field['uri']);

              switch ($attribute) {
                case 'render':
                  $output[] = theme('image_formatter', array('item' => $field, 'image_style' => $type));
                  break;

                case 'mimetype':
                case 'width':
                case 'height':
                case 'extension':
                case 'filesize':
                  // If the styled image's information wasn't loaded before. then
                  // load it now. Since this process is a little heavy, the
                  // information will be statically cached to avoid needing to
                  // reload the details for another token.
                  if (empty($images_style_data[$styled_uri])) {
                    $images_style_data[$styled_uri] = image_get_info($styled_uri);
                  }

                  switch ($attribute) {
                    case 'width':
                      $output[] = $images_style_data[$styled_uri]['width'];
                      break;

                    case 'height':
                      $output[] = $images_style_data[$styled_uri]['height'];
                      break;

                    case 'mimetype':
                      $output[] = $images_style_data[$styled_uri]['mime_type'];
                      break;

                    case 'extension':
                      $output[] = $images_style_data[$styled_uri]['extension'];
                      break;

                    case 'filesize':
                      $output[] = $images_style_data[$styled_uri]['file_size'];
                      break;
                  }
                  break;

                case 'path':
                  if (isset($explode[2]) && $explode[2] === 'relative') {
                    $url  = parse_url(image_style_url($type, $field['uri']));
                    $output[] = $url['path'];
                  }
                  else {
                    $output[] = image_style_url($type, $field['uri']);
                  }
                  break;

                default:
                  if (isset($field[$attribute])) {
                    $output[] = $field[$attribute];
                  }
              }
            }
            else {
              $output[] = image_style_url($type, $field['uri']);
            }
          }

          // Compile each of the elements into a comma separated list, per
          // standard Token practice.
          $replacements[$original] = implode(', ', $output);
        }
      }
    }
  }

  return $replacements;
}

/**
 * Helper function for getting all available image attributes.
 *
 * @return array
 *   All of the available image attributes.
 */
function _image_style_token_image_attributes() {
  return array(
    'style_name' => NULL,
    'uri' => '',
    'path' => NULL,
    'path:relative' => NULL,
    'width' => NULL,
    'height' => NULL,
    'extension' => NULL,
    'filesize' => NULL,
    'alt' => '',
    'title' => NULL,
    'mimetype' => NULL,
    'first' => '',
    'second' => '',
    'third' => '',
    'last' => '',
  );
}

/**
 * Helper function to generate image style if does not exist.
 *
 * @param string $styled_uri
 *   URI for image style.
 * @param string $type
 *   Machine name for image style.
 * @param string $field_uri
 *   URI of field to be styled.
 */
function _image_token_generate_image($styled_uri, $type, $field_uri) {
  // Only generate the image if it doesn't already exist.
  if (!file_exists($styled_uri)) {
    $style = image_style_load($type);
    // Create a derivative of the image for this style.
    image_style_create_derivative($style, $field_uri, $styled_uri);
  }
}

/**
 * Helper function to find all image fields.
 *
 * @return array
 *   An array containing all image fields. Keys of the array is an identifier
 *   made up of field_type:field_name:entity_type:bundle. Values are the
 *   human-readable name of the field.
 */
function _image_token_get_image_fields() {
  $image_fields = array();

  $fields = field_info_fields();
  foreach ($fields as $field) {
    foreach ($field['bundles'] as $entity_type => $bundles) {
      foreach ($bundles as $bundle) {
        $key = $field['type'] . ':' . $field['field_name'] . ':' . $entity_type . ':' . $bundle;
        $label = $entity_type . ': ' . $bundle . ': ' . $field['field_name'];
        if ($field['type'] == 'image') {
          $image_fields[$key] = $label;
        }
      }
    }
  }

  return $image_fields;
}

/**
 * Implements hook_token_info_alter().
 */
function image_token_info_alter(&$data) {
  $fields = field_info_field_map();
  $entity_info = entity_get_info();

  // Check to see which fields are supported.
  $supported_fields = _image_token_get_image_fields();
  $supported_fields = array_filter($supported_fields);

  foreach ($fields as $field_name => $field) {
    $field['field_name'] = $field_name;
    if ($field['type'] == 'image' || $field['type'] == 'file') {
      foreach ($field['bundles'] as $entity_type => $bundles) {
        foreach ($bundles as $bundle) {
          // For 'file' field types, it's possible that some fields won't
          // actually contain images. In these scenarios, the fields must be
          // enabled on the settings page.
          if ($field['type'] == 'file') {
            $key = $field['type'] . ':' . $field['field_name'] . ':' . $entity_type . ':' . $bundle;
            if (empty($supported_fields[$key])) {
              continue;
            }
          }
          $token_type = $entity_info[$entity_type]['token type'];

          if (!empty($data['tokens'][$token_type][$field['field_name']])) {
            $data['tokens'][$token_type][$field['field_name']]['type'] = 'image-field';
          }
        }
      }
    }
  }
}
