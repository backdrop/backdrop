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
        $message = 'The user account from the path (in position @position)';
        $substitutions = array('@position' => $position = $source + 1);
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

    // The xor returns false if the two booleans are the same, and true if they
    // are not. For example: if we asked for equality and they are equal, return
    // true. If we asked for INequality and they are equal, return false.
    return ($account1->uid == $account2->uid) xor empty($this->settings['equality']);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    // Move logged-in user account to be the first option in the first list.
    if (isset($form['contexts']['user1']['#options']['current_user'])) {
      $option_label = $form['contexts']['user1']['#options']['current_user'];
      $form['contexts']['user1']['#options'] = array_merge(
        array('current_user' => $option_label),
        $form['contexts']['user1']['#options'],
      );
    }
    // If there are only two options, remove the second one.
    if (count($form['contexts']['user1']['#options']) === 2) {
      array_pop($form['contexts']['user1']['#options']);
    }

    // Remove logged-in user account from second list.
    if (isset($form['contexts']['user2']['#options']['current_user'])) {
      unset($form['contexts']['user2']['#options']['current_user']);
    }

    $form['equality'] = array(
      '#type' => 'radios',
      '#title' => t('Comparison mode'),
      '#options' => array(
        1 => t('Display if user accounts are the same'),
        0 => t('Display if user accounts are different'),
      ),
      '#default_value' => $this->settings['equality'],
    );
  }
}
