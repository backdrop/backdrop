<?php
// $Id: views_handler_field_node_link_edit.inc,v 1.1.6.1 2010/01/17 19:56:43 dereine Exp $
/**
 * Field handler to present a link node edit.
 */
class views_handler_field_node_link_edit extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['type'] = 'type';
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->type = $values->{$this->aliases['type']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return l($text, "node/$node->nid/edit", array('query' => drupal_get_destination()));
  }
}

