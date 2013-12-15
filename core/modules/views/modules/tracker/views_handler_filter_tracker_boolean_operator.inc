<?php

/**
 * @file
 * Contains views_handler_filter_tracker_boolean_operator
 */

  /**
   * Filter handler for boolean values to use = 1 instead of <> 0.
   */
class views_handler_filter_tracker_boolean_operator extends views_handler_filter_boolean_operator {

  /**
   * Overrides views_handler_filter_boolean_operator::query().
   */
  function query() {
    $this->ensure_my_table();
    $where = "$this->table_alias.$this->real_field ";
    if (empty($this->value)) {
      $where .= '= 0';
      if ($this->accept_null) {
        $where = '(' . $where . " OR $this->table_alias.$this->real_field IS NULL)";
      }
    }
    else {
      $where .= '= 1';
    }
    $this->query->add_where_expression($this->options['group'], $where);
  }

}
