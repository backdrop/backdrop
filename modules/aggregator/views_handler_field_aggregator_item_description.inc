<?php
// $Id$
/**
 * @file
 *   Filters htmls tags from item description.
 */

class views_handler_field_aggregator_item_description extends views_handler_field_xss {
  function render($values) {
    $value = $values->{$this->field_alias};
    return aggregator_filter_xss($value);
  }
}
