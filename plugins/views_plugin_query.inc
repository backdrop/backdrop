<?php
// $Id$
/**
 * @file views_plugin_query_default.inc
 * Defines the base query class, which is the underlying layer in a View.
 */

/**
 * Object used to create a SELECT query.
 */
class views_plugin_query extends views_plugin {
  /**
   * A pager plugin that should be provided by the display.
   */
  var $pager = NULL;

  /**
   * Constructor; Create the basic query object and fill with default values.
   */
  function init($base_table, $base_field) {  }

  /**
   * Generate a query and a countquery from all of the information supplied
   * to the object.
   *
   * @param $get_count
   *   Provide a countquery if this is true, otherwise provide a normal query.
   */
  function query($get_count = FALSE) { }

  /**
   * Let modules modify the query just prior to finalizing it.
   */
  function alter(&$view) {  }

  /**
   * Builds the necessary info to execute the query.
   */
  function build(&$view) { }

  /**
   * Executes the query and fills the associated view object with according
   * values.
   *
   * Values to set: $view->result, $view->total_rows, $view->execute_time,
   * $view->pager['current_page'].
   */
  function execute(&$view) {  }

  /**
   * Add a signature to the query, if such a thing is feasible.
   *
   * This signature is something that can be used when perusing query logs to
   * discern where particular queries might be coming from.
   */
  function add_signature(&$view) { }

  /**
   * Get aggregation info for group by queries.
   *
   * If NULL, aggregation is not allowed.
   */
  function get_aggregation_info() { }

  /**
   * Set a LIMIT on the query, specifying a maximum number of results.
   */
  function set_limit($limit) {
    $this->limit = $limit;
  }

  /**
   * Set an OFFSET on the query, specifying a number of results to skip
   */
  function set_offset($offset) {
    $this->offset = $offset;
  }

  function init_pager(&$view) {
    // @todo -- probably this should be a method on the display rather than directly calling
    // get plugin?
    $this->pager = $view->display_handler->get_plugin('pager');
  }

  /**
   * Render the pager, if necessary.
   */
  function render_pager() {
    if (!empty($this->pager) && $this->pager->use_pager()) {
      $exposed_input = isset($this->view->exposed_data_raw) ? $this->view->exposed_data_raw : NULL;
      return $this->pager->render($exposed_input);
    }
    return '';
  }


}
