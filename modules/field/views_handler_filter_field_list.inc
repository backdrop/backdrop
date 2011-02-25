<?php

class views_handler_filter_field_list extends views_handler_filter_in_operator {
  function get_value_options() {
    $field = $this->field_info;
    $this->value_options = list_allowed_values($field);
  }
}
