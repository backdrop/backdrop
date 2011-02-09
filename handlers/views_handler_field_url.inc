<?php
// $Id: views_handler_field_url.inc,v 1.1.6.1 2011/02/04 12:29:01 dereine Exp $

/**
 * Field handler to provide simple renderer that turns a URL into a clickable link.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_url extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();

    $options['display_as_link'] = array('default' => TRUE);

    return $options;
  }

  /**
   * Provide link to the page being visited.
   */
  function options_form(&$form, &$form_state) {
    $form['display_as_link'] = array(
      '#title' => t('Display as link'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['display_as_link']),
    );
    parent::options_form($form, $form_state);
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['display_as_link'])) {
      return l(check_plain($value), $value, array('html' => TRUE));
    }
    else {
      return check_url($value);
    }
  }
}
