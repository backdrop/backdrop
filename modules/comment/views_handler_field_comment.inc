<?php
/**
 * Field handler to allow linking to a comment
 */
class views_handler_field_comment extends views_handler_field {
  /**
   * Override init function to provide generic option to link to comment.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($this->options['link_to_comment'])) {
      $this->additional_fields['cid'] = 'cid';
      $this->additional_fields['nid'] = 'nid';
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_comment'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Provide link-to-comment option
   */
  function options_form(&$form, &$form_state) {
    $form['link_to_comment'] = array(
      '#title' => t('Link this field to its comment'),
      '#description' => t("Enable to override this field's links."),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_comment'],
    );
    parent::options_form($form, $form_state);
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_comment']) && $data !== NULL && $data !== '') {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = "node/". $this->get_value($values, 'nid');
      $this->options['alter']['fragment'] = "comment-" . $this->get_value($values, 'cid');
    }

    return $data;
  }

  function render($values) {
    $value = $this->get_value($values);
    return $this->render_link($this->sanitize_value($value), $values);
  }
}
