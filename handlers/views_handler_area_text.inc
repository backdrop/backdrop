<?php
// $Id: views_handler_area_text.inc,v 1.1.4.6 2010/10/16 05:43:01 dereine Exp $

class views_handler_area_text extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();
    $options['content'] = array('default' => '', 'translatable' => TRUE);
    $options['format'] = array('default' => variable_get('filter_default_format', 1));
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['content'] = array(
      '#type' => 'text_format',
      '#default_value' => $this->options['content'],
      '#rows' => 6,
      '#format' => $this->options['format'],
    );
  }

  function options_submit(&$form, &$form_state) {
    $form_state['values']['options']['format'] = $form_state['values']['options']['content']['format'];
    $form_state['values']['options']['content'] = $form_state['values']['options']['content']['value'];
    parent::options_submit($form, $form_state);
  }

  function render($empty = FALSE) {
    if (!$empty || !empty($this->options['empty'])) {
      return $this->render_textarea($this->options['content'], $this->options['format']);
    }
    return '';
  }

  /**
   * Render a text area, using the proper format.
   */
  function render_textarea($value, $format) {
    static $formats = array();

    if (!array_key_exists($format, $formats)) {
      $filter = filter_format_load($format);
      $formats[$format] = $filter->name;
    }

    if (!isset($formats[$format])) {
      return;
    }

    if ($value) {
      return check_markup($value, $format, '', FALSE);
    }
  }

}

