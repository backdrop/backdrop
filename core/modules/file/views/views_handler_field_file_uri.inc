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
    $options['image_style'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['file_download_path'] = array(
      '#title' => t('Use full URL instead of file storage URI'),
      '#description' => t('An internal file path is usually <code>public://example.png</code>. Enabling this option will display the URL instead, such as <code>http://example.com/files/example.png</code>.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['file_download_path']),
    );
    $form['image_style'] = array(
      '#type' => 'select',
      '#options' => image_style_options(TRUE, PASS_THROUGH),
      '#default_value' => $this->options['image_style'],
      '#title' => t('Apply image style to URL'),
      '#description' => t('Select an available image style and the URL will be modified.'),
      '#states' => array(
        'visible' => array(
          '[name="options[file_download_path]"]' => array('checked' => TRUE),
        ),
      ),
    );
    parent::options_form($form, $form_state);
  }

  function render($values) {
    $data = $values->{$this->field_alias};
    if (!empty($this->options['file_download_path']) && $data !== NULL && $data !== '') {
      $url = file_create_url($data);
      if (!image_is_svg($url) && !empty($this->options['image_style'])) {
        $url = image_style_url($this->options['image_style'], $data);
      }
    }
    return $this->render_link($data, $values);
  }
}
