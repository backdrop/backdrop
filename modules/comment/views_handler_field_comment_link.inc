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
    return $this->render_link(check_plain($values->{$this->aliases['nid']}), $values);
  }

  function render_link($data, $values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "comment/" . $values->{$this->aliases['cid']};
    $this->options['alter']['html'] = TRUE;
    $this->options['alter']['fragment'] = "comment-" . $values->{$this->aliases['cid']};

    return $text;
  }
}
