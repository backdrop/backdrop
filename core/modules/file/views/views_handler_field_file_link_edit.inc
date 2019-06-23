<?php
/**
 * @file
 * Definition of views_handler_field_file_link_edit.
 */

/**
 * Field handler to present a link to manage a file.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_file_link_edit extends views_handler_field_entity {

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow to set it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
  }

  function render($values) {
    if ($entity = $this->get_value($values)) {
      return $this->render_link($entity, $values);
    }
  }

  /**
   * Renders the link.
   */
  function render_link(File $file, $values) {
    // Ensure user has access to edit this file.
    if (!$file->access('update')) {
      return;
    }

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "file/$file->fid/manage";
    $this->options['alter']['query'] = backdrop_get_destination();

    $text = !empty($this->options['text']) ? $this->options['text'] : t('Manage');
    return $text;
  }
}
