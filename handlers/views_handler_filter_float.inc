<?php
// $Id$

/**
 * Simple filter to handle greater than/less than filters.
 * It based on views_handler_filter_numeric but deals with
 * float numbers.
 */
class views_handler_filter_float extends views_handler_filter_numeric {
  function op_between($field) {
    if ($this->operator == 'between') {
      $this->query->add_where($this->options['group'], $field, array($this->value['min'], $this->value['max']), 'BETWEEN');
    }
    else {
      $this->query->add_where($this->options['group'], db_or()->condition($field, $this->value['min'], '<=')->condition($field, $this->value['max'], '>='));
    }
  }

  function op_simple($field) {
    $this->query->add_where($this->options['group'], $field, $this->value['value'], $this->operator);
  }
}
