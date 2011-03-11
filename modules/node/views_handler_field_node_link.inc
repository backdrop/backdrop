<?php
/**
 * Field handler to present a link to the node.
 */
class views_handler_field_node_link extends views_handler_field {
  function construct() {
    parent::construct();
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
    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "node/$data";
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    return $text;
  }
}
