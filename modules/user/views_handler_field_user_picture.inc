<?php

/**
 * Field handler to provide simple renderer that allows using a themed user link
 */
class views_handler_field_user_picture extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['name'] = 'name';
    $this->additional_fields['mail'] = 'mail';   
  }

  function element_type($none_supported = FALSE, $default_empty = FALSE, $inline = FALSE) {
    if ($inline) {
      return 'span';
    }

    return 'div';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_photo_to_profile'] = array('default' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_photo_to_profile'] = array(
      '#title' => t("Link to user's profile"),
      '#description' => t("Link the user picture to the user's profile"),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_photo_to_profile'],
    );
  }

  function render($values) {
    // Fake an account object.
    $account = new stdClass();
    if ($this->options['link_photo_to_profile']) {
      // Prevent template_preprocess_user_picture from adding a link
      // by not setting the uid.
      $account->uid = $this->get_value($values, 'uid');
    }
    $account->name = $this->get_value($values, 'name');
    $account->mail = $this->get_value($values, 'mail');
    $account->picture = $this->get_value($values);

    return theme('user_picture', array('account' => $account));
  }
}
