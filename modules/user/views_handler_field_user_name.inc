<?php
/**
 * Field handler to provide simple renderer that allows using a themed user link
 */
class views_handler_field_user_name extends views_handler_field_user {
  /**
   * Add uid in the query so we can test for anonymous if needed.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (!empty($this->options['overwrite_anonymous'])) {
      $this->additional_fields['uid'] = 'uid';
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['overwrite_anonymous'] = array('default' => FALSE);
    $options['anonymous_text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['overwrite_anonymous'] = array(
      '#title' => t('Overwrite the value to display for anonymous users'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['overwrite_anonymous']),
      '#description' => t('Enable to display different text for anonymous users.'),
      '#fieldset' => 'more',
    );
    $form['anonymous_text'] = array(
      '#title' => t('Text to display for anonymous users'),
      '#type' => 'textfield',
      '#default_value' => $this->options['anonymous_text'],
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array(
        'edit-options-overwrite-anonymous' => array(1),
      ),
      '#fieldset' => 'more',
    );

    parent::options_form($form, $form_state);
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_user']) || !empty($this->options['overwrite_anonymous'])) {
      $account = new stdClass();
      $account->uid = $this->get_value($values, 'uid');
      if (!empty($this->options['overwrite_anonymous']) && !$account->uid) {
        // This is an anonymous user, and we're overriting the text.
        return check_plain($this->options['anonymous_text']);
      }
      elseif (!empty($this->options['link_to_user'])) {
        $account->name = $this->get_value($values);
        return theme('username', array('account' => $account));
      }
    }
    // Otherwise, there's no special handling, so return the data directly.
    return $data;
  }
}

