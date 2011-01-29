<?php
// $Id: views_handler_filter_system_type.inc,v 1.1.4.2 2011/01/22 22:49:09 dereine Exp $

/**
 * Filter by system type.
 */
class views_handler_filter_system_type extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Type');
      // Enable filtering by type.
      $types = array();
      $types = db_query('SELECT DISTINCT(type) FROM {system} ORDER BY type')->fetchAllKeyed(0, 0);
      $this->value_options = $types;
    }
  }
}
