<?php

/**
 * @file
 * Definition of views_handler_field_user_roles.
 */

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_roles extends views_handler_field_prerender_list {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'users', 'field' => 'uid');
  }

  function query() {
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['uid'];
  }

  function pre_render(&$values) {
    $uids = array();
    $this->items = array();

    foreach ($values as $result) {
      $uids[] = $this->get_value($result, NULL, TRUE);
    }

    if ($uids) {
      $result = db_query("SELECT uid, rid FROM {users_roles} WHERE uid IN (:uids)", array(':uids' => $uids));
      $role_names = user_roles();
      foreach ($result as $role) {
        $this->items[$role->uid][$role->rid]['role_label'] = $role_names[$role->rid];
        $this->items[$role->uid][$role->rid]['role_name'] = $role->rid;
      }
    }
  }

  function render_item($count, $item) {
    return check_plain($item['role_label']);
  }

  function document_self_tokens(&$tokens) {
    $tokens['[' . $this->options['id'] . '-role' . ']'] = t('The name of the role.');
    $tokens['[' . $this->options['id'] . '-rid' . ']'] = t('The role ID of the role.');
  }

  function add_self_tokens(&$tokens, $item) {
    if (!empty($item['role_label'])) {
      $tokens['[' . $this->options['id'] . '-role' . ']'] = $item['role_label'];
      $tokens['[' . $this->options['id'] . '-rid' . ']'] = $item['role_name'];
    }
  }
}
