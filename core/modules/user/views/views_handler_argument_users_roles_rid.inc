<?php

/**
 * @file
 * Definition of views_handler_argument_users_roles_rid.
 */

/**
 * Allow role ID(s) as argument.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_users_roles_rid extends views_handler_argument_many_to_one {
  function title_query() {
    $role_names = backdrop_map_assoc((array) $this->value);
    $roles = user_roles();
    $roles = array_diff_key($roles, $role_names);
    return $roles;
  }
}
