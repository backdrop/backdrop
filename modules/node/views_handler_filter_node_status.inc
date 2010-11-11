<?php
// $Id: views_handler_filter_node_status.inc,v 1.2.4.5 2010/09/22 21:31:29 dereine Exp $
/**
 * Filter by published status
 */
class views_handler_filter_node_status extends views_handler_filter {
  function admin_summary() { }
  function operator_form(&$form, &$form_state) { }

  function query() {
    $table = $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$table.status <> 0 OR ($table.uid = ***CURRENT_USER*** AND ***CURRENT_USER*** <> 0 AND ***VIEW_OWN_UNPUBLISHED_NODES*** = 1) OR ***ADMINISTER_NODES*** = 1", array(), 'formula');
  }
}
