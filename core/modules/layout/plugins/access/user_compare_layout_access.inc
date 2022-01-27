<?php
/**
 * @file
 * Plugin to provide access if two user account contexts are equal.
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
        $labels[$key] = t('Logged in user account');
      }
      elseif (is_numeric($source)) {
        $position = $source +1;
        $labels[$key] = t('User account in path (position ' . $position . ')');
      }
      else {
        $labels[$key] = t('Unknown user account');
      }
    }

    $comparison = !empty($this->settings['equality']) ? 'is' : 'is not';
    return t('@id1 @comp @id2', array(
      '@comp' => $comparison,
      '@id1' => $labels['user1'],
      '@id2' => $labels['user2'],
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

    // xor returns false if the two bools are the same, and true if they are not.
    // i.e, if we asked for equality and they are equal, return true.
    // If we asked for INequality and they are equal, return false.
    return ($account1->uid == $account2->uid) xor empty($this->settings['equality']);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['helptext'] = array(
      '#type' => 'markup',
      '#value' => '<div>' . t('Compare two user accounts (logged-in account and account being viewed, for example).') . '</div>',
    );
    $form['equality'] = array(
      '#type' => 'radios',
      '#title' => t('Grant access if the two are'),
      '#options' => array(
        1 => t('The same user account'),
        0 => t('Different user accounts'),
      ),
      '#default_value' => $this->settings['equality'],
    );
  }
}
