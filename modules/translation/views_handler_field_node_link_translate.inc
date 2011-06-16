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
  }

  function render_link($data, $values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $this->get_value($values, 'nid');
    $node->uid = $this->get_value($values, 'uid');
    $node->type = $this->get_value($values, 'type');
    $node->language = $this->get_value($values, 'language');
    $node->status = 1; // unpublished nodes ignore access control
    if (empty($node->language) || !translation_supported_type($node->type) || !node_access('view', $node) || !user_access('translate content')) {
      return;
    }

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "node/$node->nid/translate";
    $this->options['alter']['query'] = drupal_get_destination();

    $text = !empty($this->options['text']) ? $this->options['text'] : t('translate');
    return $text;
  }
}

