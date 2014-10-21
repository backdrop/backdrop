<?php
/**
 * @file
 * Plugin to provide access control based upon role membership.
 */
class UserRoleLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'rids' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    $rids = backdrop_map_assoc($this->settings['rids']);
    $roles = array_intersect_key(user_roles(), $rids);
    if (empty($roles)) {
      return t('User can have any role');
    }
    return format_plural(count($roles), 'User has the role @role', 'User has one of the following roles: @role', array('@role' => implode(', ', $roles)));
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $account = $this->contexts['user']->data;
    $rids = array_keys($account->roles);
    return count(array_intersect($this->settings['rids'], $rids)) > 0;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['rids'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Role'),
      '#default_value' => $this->settings['rids'],
      '#options' => user_roles(),
      '#description' => t('Only the checked roles will be granted access.'),
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['rids'] = array_keys(array_filter($form_state['values']['rids']));
  }
}
