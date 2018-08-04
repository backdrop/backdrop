<?php

/**
 * @file
 * Definition of views_handler_field_file_uri.
 */

/**
 * Field handler to add rendering file paths as file URLs instead of as internal file URIs.
 */
class views_handler_field_file_uri extends views_handler_field_file {
  function option_definition() {
    $options = parent::option_definition();
    $options['file_download_path'] = array('default' => FALSE, 'bool' => TRUE);
    $options['thumbnail_style'] = array('default' => 'thumbnail');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['file_download_path'] = array(
      '#title' => t('Display download path instead of file storage URI'),
      '#description' => t('This will provide the full download URL for the file.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['file_download_path']),
    );
    $form['thumbnail_style'] = array(
      '#type' => 'select',
      '#options' => image_style_options(TRUE, PASS_THROUGH),
      '#default_value' => $this->options['thumbnail_style'],
      '#title' => t('For paths to image files: option to use an image style instead of original file.'),
      '#description' => t("Select an available image style and the URL will be modified. "),
    );
    parent::options_form($form, $form_state);
  }

  function render($values) {
    $data = $values->{$this->field_alias};
    if (!empty($this->options['file_download_path']) && $data !== NULL && $data !== '') {
      $data = file_create_url($data);
      if (!empty($this->options['thumbnail_style'])) {
      // $data contains url of image.
      // Find insertion position of '/files'.
        $insert_at = strpos($data, '/files');
      // Text to insert in order to modify url.
        $add_to_url = '/styles/' . $this->options['thumbnail_style'] . '/public';
        $data = substr_replace($data, $add_to_url, $insert_at + 6, 0);
      }
    }
    return $this->render_link($data, $values);
  }
}
