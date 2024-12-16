<?php
/**
 * @file
 * Provide node nid argument handler.
 */

/**
 * Argument handler to accept a node id.
 */
class views_handler_argument_node_nid extends views_handler_argument_numeric {
  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title_query() {
    $titles = [];

    $result = db_query("SELECT n.title FROM {node} n WHERE n.nid IN (:nids)", [':nids' => $this->value]);
    foreach ($result as $term) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}
