<?php
// $Id: views_handler_field_term_link_edit.inc,v 1.1.4.2 2010/02/22 19:34:22 dereine Exp $

/**
 * Field handler to present a term edit link .
 */
class views_handler_field_term_link_edit extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['tid'] = 'tid';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    if (user_access('administer taxonomy')) {
      $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
      return l($text, 'taxonomy/term/'. $values->{$this->aliases['tid']} .'/edit', array('query' => drupal_get_destination()));
    }
  }
}

