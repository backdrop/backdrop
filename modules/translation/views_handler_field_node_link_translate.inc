<?php
/**
 * Field handler to present a link node translate.
 */
class views_handler_field_node_link_translate extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['type'] = 'type';
    $this->additional_fields['language'] = 'language';
    $this->additional_fields['format'] = array('table' => 'node_revisions', 'field' => 'format');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $this->get_value($values, 'nid');
    $node->uid = $this->get_value($values, 'uid');
    $node->type = $this->get_value($values, 'type');
    $node->format = $this->get_value($values, 'format');
    $node->language = $this->get_value($values, 'language');
    $node->status = 1; // unpublished nodes ignore access control
    if (empty($node->language) || !translation_supported_type($node->type) || !node_access('view', $node) || !user_access('translate content')) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('translate');
    return l($text, "node/$node->nid/translate", array('query' => drupal_get_destination()));
  }
}

