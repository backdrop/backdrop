<?php
/**
 * Field handler to display the depth of a comment.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_depth extends views_handler_field {
  /**
   * Work out the depth of this comment
   */
  function render($values) {
    return count(explode('.', $values->comments_thread)) - 1;
  }
}
