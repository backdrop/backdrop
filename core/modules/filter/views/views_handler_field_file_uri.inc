<?php

/**
 * @file
 * Definition of views_handler_field_file_uri.
 * Variant for use in Filter module for Image Library.
 */

/**
 * Field handler to add rendering file paths as file URLs instead of as internal file URIs.
 */
class views_handler_field_file_uri extends views_handler_field_file {
  function option_definition() {
    $options = parent::option_definition();
    $options['file_download_path'] = array('default' => FALSE, 'bool' => TRUE);
    $options['image_thumbnail'] = array('default' => FALSE, 'bool' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['file_download_path'] = array(
      '#title' => t('Display download path instead of file storage URI'),
      '#description' => t('This will provide the full download URL rather than the internal filestream address.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['file_download_path']),
    );

    $form['image_thumbnail'] = array(
      '#type' => 'checkbox',
      '#title' => t('Adapt URI for use of thumbnail'),
      '#description' => t('Modify the URI to reference the thumbnail image'),
      '#default_value' => !empty($this->options['image_thumbnail']),
    );
    parent::options_form($form, $form_state);
  }

  function render($values) {
    $data = $values->{$this->field_alias};
    if (!empty($this->options['file_download_path']) && $data !== NULL && $data !== '') {
      $data = file_create_url($data);
      if (!empty($this->options['image_thumbnail'])) {
      // $data contains URI of image.
      // Find insertion position of '/files'.
        $insert_at = strpos($data, '/files');
      // Text to insert.
        $add_to_uri = '/styles/thumbnail/public';
        $data = substr_replace($data, $add_to_uri, $insert_at + 6, 0);
      }
    }
    return $this->render_link($data, $values);
  }
}
