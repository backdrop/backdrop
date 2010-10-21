<?php
// $Id $

// @todo Implement a real CRUD API, and store in a real db table...

function field_group_get_groups($entity_type = NULL, $bundle = NULL, $mode = NULL) {
  $groups = variable_get('field_group_groups', _field_groups_default_groups());

  if (!isset($entity_type)) {
    return $groups;
  }
  else {
    if (isset($groups[$entity_type][$bundle][$mode])) {
      return $groups[$entity_type][$bundle][$mode];
    }
    else {
      return array();
    }
  }
}

function field_group_save_group($entity_type, $bundle, $mode, $group) {
  $groups = variable_get('field_group_groups', _field_groups_default_groups());

  $groups[$entity_type][$bundle][$mode][$group['name']] = $group;

  variable_set('field_group_groups', $groups);
}

// @todo temporary, dev only.
function _field_groups_default_groups() {
  return array(
    'node' => array(
      'article' => array(
        'default' => array(
          'group_1' => array(
            'name' => 'group_1',
            'label' => 'Group 1',
            'weight' => 0,
            'visible' => TRUE,
  // Having the parent list its children is unusual.
  // It's because potential children don't know they have parents
  // (no 'parent' column incore field_config_instances table).
            'children' => array('group_2'),
  ),
          'group_2' => array(
            'name' => 'group_2',
            'label' => 'Group 2',
            'weight' => 1,
            'visible' => TRUE,
            'children' => array('body'),
  ),
  ),
  ),
      'page' => array(
        'default' => array(
          'group_1' => array(
            'name' => 'group_1',
            'label' => 'Group 1',
            'weight' => 0,
            'visible' => TRUE,
             'children' => array('group_2'),
  ),
          'group_2' => array(
            'name' => 'group_2',
            'label' => 'Group 2',
            'weight' => 1,
            'visible' => TRUE,
            'children' => array('body'),
  ),
  ),
  ),
  ),
  );
}