<?php

/**
 * @file
 * Definition of views_handler_field_comment_link_disapprove.
 */

/**
 * Field handler to provide a comment disapprove link.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_disapprove extends views_handler_field_comment_link {
  function access() {
    // Needs permission to administer comments in general.
    return user_access('administer comments');
  }

  function render_link($data, $values) {
    $status = $this->get_value($values, 'status');

    $text = !empty($this->options['text']) ? $this->options['text'] : t('disapprove');
    $cid = $this->get_value($values, 'cid');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = 'comment/' . $cid . '/disapprove';
    $this->options['alter']['query'] = drupal_get_destination() + array('token' => drupal_get_token($this->options['alter']['path']));

    return $text;
  }
}
