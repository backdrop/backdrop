<?php
/**
 * A field that links to the user contact page, if access is permitted.
 */
class views_handler_field_contact_link extends views_handler_field_user_link {

  function option_definition() {
    $options = parent::option_definition();
    $options['link_display'] = array('default' => 'link', 'translatable' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['link_display'] = array(
      '#title' => t('Type of link'),
      '#default_value' => $this->options['link_display'],
      '#type' => 'select',
      '#options' => array(
        'link' => t('Link'),
        'icon' => t('Icon'),
      ),
    );
    $form['text']['#title'] = t('Link label');
    $form['text']['#required'] = TRUE;
    $form['text']['#default_value'] = empty($this->options['text']) ? t('contact') : $this->options['text'];
    parent::options_form($form, $form_state);
  }

  // An example of field level access control.
  // We must override the access method in the parent class, as that requires
  // the 'access user profiles' permission, which the contact form does not.
  function access() {
    global $user;

    // Only registered users can view other registered user's contact page.
    if (empty($user->uid)) {
      return FALSE;
    }

    return TRUE;
  }

  function render_link($data, $values) {
    global $user;
    $uid = $this->get_value($values, 'uid');

    if (empty($uid)) {
      return;
    }

    $account = user_load($uid);
    if (empty($account)) {
      return;
    }

    // Check access when we pull up the user account so we know
    // if the user has made the contact page available.
    $menu_item = menu_get_item("user/$uid/contact");
    if (!$menu_item['access'] || empty($account->contact)) {
      return;
    }

    $this->options['alter']['path'] = 'user/'. $account->uid .'/contact';
    $this->options['alter']['attributes'] = array('title' => t('Contact %user', array('%user' => $account->name)));

    if ($this->options['link_display'] == 'icon') {
      $text = theme('image', array('path' => 'misc/forum-new.png'));
      $this->options['alter']['html'] = TRUE;
    }
    else {
      $text = $this->options['text'];
    }

    return $text;
  }
}
