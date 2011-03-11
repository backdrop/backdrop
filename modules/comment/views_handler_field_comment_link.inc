<?php
/**
 * Base field handler to present a link.
 */
class views_handler_field_comment_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['cid'] = 'cid';
    $this->additional_fields['nid'] = 'nid';
  }

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
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $value = $this->get_value($values, 'nid');
    return $this->render_link($this->sanitize_value($value), $values);
  }

  function render_link($data, $values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    $nid = $this->get_value($values, 'nid');
    $cid = $this->get_value($values, 'cid');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "comment/" . $cid;
    $this->options['alter']['html'] = TRUE;
    $this->options['alter']['fragment'] = "comment-" . $cid;

    return $text;
  }
}
