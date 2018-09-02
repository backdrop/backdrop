<?php

/**
 * @file
 * Definition of views_handler_filter_node_hidden_path.
 */

/**
 * Filter by content type's hidden path setting.
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_node_hidden_path extends views_handler_filter_boolean_operator {
  function admin_summary() { }
  function operator_form(&$form, &$form_state) { }
  function can_expose() { return FALSE; }

  function query() {
    $table = $this->ensure_my_table();
    $node_types = node_type_get_types();

    $enabled_types = array_filter($node_types, function($val) {
      return $val->settings['hidden_path'];
    });

    $diff_types = $enabled_types;
    if (!$this->value) {
      $diff_types = array_diff_key($node_types, $enabled_types);
    }

    $types_names = array_map(function($val) {
      return $val->type;
    }, $diff_types);

    $this->query->add_where_expression($this->options['group'], "$table.type IN (:types)", array(':types' => $types_names));
  }
}
