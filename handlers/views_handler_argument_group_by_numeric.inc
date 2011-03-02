<?php

/**
 * Simple handler for arguments using group by.
 */
class views_handler_argument_group_by_numeric extends views_handler_argument  {
  function query($group_by = FALSE) {
    $this->ensure_my_table();
    $field = $this->get_field();

    $this->query->add_having(0, $field, $this->argument);
  }

  function ui_name() {
    return $this->get_field(parent::ui_name());
  }

  function get_sort_name() {
    return t('Numerical', array(), array('context' => 'Sort order'));
  }
}
