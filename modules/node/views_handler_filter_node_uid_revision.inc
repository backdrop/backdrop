<?php
// $Id$
/**
 * @file
 * Filter handler to check for revisions a certain user has created.
 */
class views_handler_filter_node_uid_revision extends views_handler_filter_user_name {
  function query() {
    $this->ensure_my_table();
    $replace = array_fill(0, sizeof($this->value), '%d');
    $this->operator = strtoupper($this->operator);
    $condition = $this->operator . ' (' . implode(", ", $replace) . ')';

    // We include the WHERE condition twice in the query, so we need two
    // copies of these argument values.
    $args = array_values($this->value);
    $args = array_merge($args, $args);

    $this->query->add_where($this->options['group'], "$this->table_alias.uid " . $condition . " OR
      ((SELECT COUNT(*) FROM {node_revisions} nr WHERE nr.uid " . $condition . " AND nr.nid = $this->table_alias.nid) > 0)",
      $args);
  }
}
