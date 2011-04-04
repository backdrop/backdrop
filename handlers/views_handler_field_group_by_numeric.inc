<?php

/**
 * Handler for GROUP BY on simple numeric fields.
 */
class views_handler_field_group_by_numeric extends views_handler_field_numeric {
  function add_additional_fields($fields = NULL) {
    if (!isset($fields)) {
      // notice check
      if (empty($this->additional_fields)) {
        return;
      }
      $fields = $this->additional_fields;
    }
    $new_fields = array();

    foreach ($fields as $identifier => $info) {
      if (!is_array($info)) {
        $identifier = $info;
        $info = array(
          'field' => $identifier,
        );
      }

      $info['params'] = array(
        'function' => $this->options['group_type'],
      );

      $new_fields[$identifier] = $info;
    }

    if (!empty($new_fields)) {
      return parent::add_additional_fields($new_fields);
    }
  }

  function init(&$view, &$options) {
    // group by fields are always floats.
    $this->definition['float'] = TRUE;
    parent::init($view, $options);

    // Initialize the original handler.
    $this->handler = views_get_handler($options['table'], $options['field'], 'field');
    $this->handler->init($view, $options);
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field, taking care of any aggregation that may affect it.
    $params = array(
      'function' => $this->options['group_type'],
    );

    $this->field_alias = $this->query->add_field($this->table_alias, $this->real_field, NULL, $params);
    $this->add_additional_fields();
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $params = array(
      'function' => $this->options['group_type'],
    );

    $this->query->add_orderby($this->table, $this->field, $order, $this->field_alias, $params);
  }

  function ui_name($short = FALSE) {
    return $this->get_field(parent::ui_name($short));
  }
}
