<?php

/**
 * @file
 * Contains views_handler_argument_tracker_comment_user_uid
 */

/**
 * UID argument to check for nodes that user posted or commented on.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_tracker_comment_user_uid extends views_handler_argument_comment_user_uid {

  /**
   * Overrides views_handler_argument_comment_user_uid::query().
   */
  function query($group_by = FALSE) {
    // Because this handler thinks it's an argument for a field on the {node}
    // table, we need to make sure {tracker_user} is JOINed and use its alias
    // for the WHERE clause.
    $tracker_user_alias = $this->query->ensure_table('tracker_user');
    $this->query->add_where(0, "$tracker_user_alias.uid", $this->argument);
  }

}
