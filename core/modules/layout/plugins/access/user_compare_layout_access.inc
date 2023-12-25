<?php
/**
 * @file
 * Contains UserCompareLayoutAccess class.
 */

/**
 * Plugin to provide access if two user account contexts are the same.
 */
class UserCompareLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  public function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'equality' => 1,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    $contexts = $this->settings['contexts'];
    if (empty($contexts) || count($contexts) != 2 || empty($contexts['user1']) || empty($contexts['user2'])) {
      return t('Compares two user accounts');
    }

    $labels = array();
    foreach ($this->settings['contexts'] as $key => $source) {
      if ($source == 'current_user') {
        $labels[$key] = t('The logged-in user account');
      }
      elseif (is_numeric($source)) {
        $substitutions = array('@position' => $source + 1);
        $labels[$key] = t('The user account from the path (in position @position)', $substitutions);
      }
      else {
        $labels[$key] = t('A user account from the layout contexts');
      }
    }

    $substitutions = array('@user' => $labels['user2']);
    if (!empty($this->settings['equality'])) {
      return t('@user is the same as the Logged-in user account.', $substitutions);
    }
    else {
      return t('@user is different from the Logged-in user account.', $substitutions);
    }
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (empty($this->contexts) || count($this->contexts) != 2 || empty($this->contexts['user1']->data) || empty($this->contexts['user2']->data)) {
      return FALSE;
    }

    $account1 = $this->contexts['user1']->data;
    $account2 = $this->contexts['user2']->data;

    // The xor returns false if the two booleans are the same, and true if they
    // are not. For example: if we asked for equality and they are equal, return
    // true. If we asked for inequality and they are equal, return false.
    return ($account1->uid == $account2->uid) xor empty($this->settings['equality']);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['help_text'] = array(
      '#type' => 'help',
      '#value' => t('Compare a user account to the currently logged-in user account. For someone viewing their own profile page, for example, choose "User account being viewed" and say "Display if the above user account is" and "the same as the logged-in user account".'),
    );
    // Always assume the first context is the logged-in user account.
    $form['contexts']['user1'] = array(
      '#type' => 'value',
      '#value' => 'current_user',
    );
    // Remove logged-in user account from list of comparison options.
    if (isset($form['contexts']['user2']['#options']['current_user'])) {
      unset($form['contexts']['user2']['#options']['current_user']);
    }
    $form['equality'] = array(
      '#type' => 'radios',
      '#title' => t('Comparison mode '),
      '#options' => array(
        1 => t('Display if this account is the logged-in user account.'),
        0 => t('Display if this account is different from the logged-in user account.'),
      ),
      '#default_value' => $this->settings['equality'],
    );
  }
}
