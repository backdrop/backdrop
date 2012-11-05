<?php

/**
 * @file
 * Provide views data and handlers for tracker.module.
 *
 * @ingroup views_module_handlers
 */
/**
 * Implementation of hook_views_data().
 */
function tracker_views_data() {
  $data = array();

  $data['tracker_node']['table']['group'] = t('Tracker');
  $data['tracker_node']['table']['join'] = array(
    'node' => array(
      'type' => 'INNER',
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );
  $data['tracker_node']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node.'),
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'nid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['tracker_node']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the node was last updated.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $data['tracker_node']['published'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the node is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_tracker_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tracker_user']['table']['group'] = t('Tracker - User');
  $data['tracker_user']['table']['join'] = array(
    'node' => array(
      'type' => 'INNER',
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'user' => array(
      'type' => 'INNER',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  $data['tracker_user']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node a user created or commented on. You must use an argument or filter on UID or you will get misleading results using this field.'),
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'nid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['tracker_user']['uid'] = array(
    'title' => t('Uid'),
    'help' => t('The user ID of a user who touched the node (either created or commented on it).'),
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name',
    ),
    'filter' => array(
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['tracker_user']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the node was last updated or commented on. You must use an argument or filter on UID or you will get misleading results using this field.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $data['tracker_user']['published'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the node is published. You must use an argument or filter on UID or you will get misleading results using this field.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_tracker_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function tracker_views_data_alter(&$data) {
  // Provide additional uid_touch handlers which are handled by tracker
  $data['node']['uid_touch_tracker'] = array(
    'group' => t('Tracker - User'),
    'title' => t('User posted or commented'),
    'help' => t('Display nodes only if a user posted the node or commented on the node.'),
    'argument' => array(
      'field' => 'uid',
      'name table' => 'users',
      'name field' => 'name',
      'handler' => 'views_handler_argument_tracker_comment_user_uid',
      'no group by' => TRUE,
    ),
    'filter' => array(
      'field' => 'uid',
      'name table' => 'users',
      'name field' => 'name',
      'handler' => 'views_handler_filter_tracker_comment_user_uid'
    ),
  );
}
