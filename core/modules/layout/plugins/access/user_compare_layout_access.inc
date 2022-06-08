<?php
/**
 * @file
 * Plugin to provide access if two user account contexts are the same.
 */
class UserCompareLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'equality' => 1,
    );
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
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
        $message = 'The user account from the path (in position @position)';
        $substitutions = array('@position' => $position = $source +1);
        $labels[$key] = t($message, $substitutions);
      }
      else {
        $labels[$key] = t('An unknown user account');
      }
    }

    $comparison = !empty($this->settings['equality']) ? 'is the same as' : 'is different from';
    return t('@id2 @comp the Logged-in user account.', array(
      '@id2' => $labels['user2'],
      '@comp' => $comparison,
    ));
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

    // xor returns false if the two booleanss are the same, and true if they are
    // not. For example: if we asked for equality and they are equal, return
    // true. If we asked for INequality and they are equal, return false.
    return ($account1->uid == $account2->uid) xor empty($this->settings['equality']);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['helptext'] = array(
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
      '#title' => t('Display this block if the above user account is'),
      '#options' => array(
        1 => t('the same as the logged-in user account.'),
        0 => t('different from the logged-in user account.'),
      ),
      '#default_value' => $this->settings['equality'],
    );
  }
}
