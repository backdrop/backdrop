<?php
/**
 * @file
 * Class providing forms and settings for the Message layout block style.
 */
class LayoutStyleMessage extends LayoutStyle {
  /**
   * Constructor for a Layout style message class.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'message_type' => '',
    );
  }

  /**
   * Specifies the settings form for configuring the style.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!$this->is_region) {
      $form['message_type'] = array(
        '#title' => t('Message type'),
        '#type' => 'radios',
        '#options' => backdrop_get_message_types(),
        '#weight' => -1,
        '#default_value' => $this->settings['message_type'],
      );
      unset($form['classes']);
    }
  }
}
