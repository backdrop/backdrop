<?php

/**
 * @file
 * Definition of views_handler_field_file_rendered.
 */

/**
 * Field handler to render a file with a view mode using file_view_file().
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_file_rendered extends views_handler_field_entity {
  function option_definition() {
    $options = parent::option_definition();
    $options['file_view_mode'] = array('default' => 'default');
    return $options;
  }

  /**
   * Provide file_view_mode option for file display.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $entity_info = entity_get_info('file');
    $options = array('default' => t('Default'));
    foreach ($entity_info['view modes'] as $file_view_mode => $file_view_mode_info) {
      $options[$file_view_mode] = $file_view_mode_info['label'];
    }

    $form['file_view_mode'] = array(
      '#title' => t('File view mode'),
      '#description' => t('Select a view mode. Note that only the file will be rendered and not any of its fields.'),
      '#type' => 'select',
      '#default_value' => $this->options['file_view_mode'],
      '#options' => $options,
    );
  }

  function render($values) {
    if ($file = $this->get_value($values)) {
      return file_view_file($file, $this->options['file_view_mode']);
    }
  }
}
