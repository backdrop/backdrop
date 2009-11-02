<?php
// $Id$

/**
 * Field handler to translate a node type into its readable form.
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_type_get_name($values->{$this->field_alias});
    return $this->render_link(t(check_plain($value)), $values);
  }
}

