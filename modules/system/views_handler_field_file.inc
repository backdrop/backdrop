<?php
// $Id$
/**
 * Field handler to provide simple renderer that allows linking to a file.
 */
class views_handler_field_file extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_file'])) {
      $this->additional_fields['uri'] = 'uri';
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_file'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Provide link to file option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#description' => t('This will override any other link you have set.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_file']) && $data !== NULL && $data !== '') {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = file_create_url($values->{$this->aliases['uri']});
    }

    return $data;
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

/**
 * Field handler to add rendering MIME type images as an option on the filemime field.
 */
class views_handler_field_file_filemime extends views_handler_field_file {
  function option_definition() {
    $options = parent::option_definition();
    $options['filemime_image'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['filemime_image'] = array(
      '#title' => t('Display an image representing the MIME type instead of the MIME text.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['filemime_image']),
    );
  }

  function render($values) {
    $data = $values->{$this->field_alias};
    if (!empty($this->options['filemime_image']) && $data !== NULL && $data !== '') {
      $fake_file = (object) array('filemime' => $data);
      $data = theme('file_icon', array('file' => $fake_file));
    }

    return $this->render_link($data, $values);
  }
}

/**
 * Field handler to add rendering file paths as file URLs instead of as internal file URIs.
 */
class views_handler_field_file_uri extends views_handler_field_file {
  function option_definition() {
    $options = parent::option_definition();
    $options['file_download_path'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['file_download_path'] = array(
      '#title' => t('Display download path instead of file storage URI'),
      '#description' => t('This will provide the full download URL rather than the internal filestream address.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['file_download_path']),
    );
  }

  function render($values) {
    $data = $values->{$this->field_alias};
    if (!empty($this->options['file_download_path']) && $data !== NULL && $data !== '') {
      $data = file_create_url($data);
    }
    return $this->render_link($data, $values);
  }
}
