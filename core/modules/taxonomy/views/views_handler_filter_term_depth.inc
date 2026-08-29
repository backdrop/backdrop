<?php

/**
 * @file
 * Definition of views_handler_filter_term_depth.
 */

/**
 * Filter handler for taxonomy terms with depth.
 *
 * This handler is actually part of the node table and has some restrictions,
 * because it uses a subquery to find nodes with.
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_term_depth extends views_handler_filter_many_to_one {
  function operator_options($which = 'title') {
    return array(
    );
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['value'] = array('default' => 0);

    return $options;
  }

  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    $form['value'] = array(
      '#type' => 'weight',
      '#title' => t('Depth'),
      '#default_value' => $this->options['depth'],
      '#description' => t('The depth will match nodes tagged with terms in the hierarchy. For example, if you have the term "fruit" and a child term "apple", with a depth of 1 (or higher) then filtering for the term "fruit" will get nodes that are tagged with "apple" as well as "fruit". If negative, the reverse is true; searching for "apple" will also pick up nodes tagged with "fruit" if depth is -1 (or lower).'),
    );

    if (empty($form_state['exposed'])) {
      $this->helper->options_form($form, $form_state);
    }
  }

  function query() {
    // If no filter values are present, then do nothing.
    if (count($this->value) == 0) {
      return;
    }
    elseif (count($this->value) == 1) {
      // Somethis $this->value is an array with a single element so convert it.
      if (is_array($this->value)) {
        $this->value = current($this->value);
      }
      $operator = '=';
    }
    else {
      $operator = 'IN';# " IN (" . implode(', ', array_fill(0, sizeof($this->value), '%d')) . ")";
    }

    // The normal use of ensure_my_table() here breaks Views.
    // So instead we trick the filter into using the alias of the base table.
    // See http://drupal.org/node/271833
    // If a relationship is set, we must use the alias it provides.
    if (!empty($this->relationship)) {
      $this->table_alias = $this->relationship;
    }
    // If no relationship, then use the alias of the base table.
    elseif (isset($this->query->table_queue[$this->query->base_table]['alias'])) {
      $this->table_alias = $this->query->table_queue[$this->query->base_table]['alias'];
    }
    // This should never happen, but if it does, we fail quietly.
    else {
      return;
    }

    // If depth = 0, Assure this term is not in the taxonomy_term_hierarchy table.
    if () {

    }
    else {

    }

    // Now build the subqueries.
    $subquery = db_select('taxonomy_term_hierarchy', 'th');
    $subquery->addField('th', 'tid');
    $last = "th";

    if ($this->value > 0) {
      $where = db_or();
      foreach (range(1, abs($this->value)) as $count) {
        $subquery->leftJoin('taxonomy_term_hierarchy', "th$count", "$last.parent = th$count.tid");
        $where->condition("th$count.tid", "$last.tid");
        $last = "th$count";
      }
    }
    elseif ($this->value < 0) {
      $where = db_or();
      foreach (range(1, abs($this->value)) as $count) {
        $subquery->leftJoin('taxonomy_term_hierarchy', "th$count", "$last.tid = th$count.parent");
        $where->condition("th$count.tid", "$last.tid");
        $last = "th$count";
      }
    }

    $subquery->condition($where);
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field", $subquery, 'IN');
  }
}
