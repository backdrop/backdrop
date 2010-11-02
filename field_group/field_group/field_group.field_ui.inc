<?php
// $Id$

/**
 * Implements hook_menu().
 */
function field_group_menu() {
  $items = array();

  // Ensure the following is not executed until field_bundles is working and
  // tables are updated. Needed to avoid errors on initial installation.
  if (defined('MAINTENANCE_MODE')) {
    return $items;
  }

  // Create tabs for all possible bundles.
  foreach (entity_get_info() as $entity_type => $entity_info) {
    if ($entity_info['fieldable']) {
      foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
        if (isset($bundle_info['admin'])) {
          // Extract path information from the bundle.
          $path = $bundle_info['admin']['path'];
          // Different bundles can appear on the same path (e.g. %node_type and
          // %comment_node_type). To allow field_ui_menu_load() to extract the
          // actual bundle object from the translated menu router path
          // arguments, we need to identify the argument position of the bundle
          // name string ('bundle argument') and pass that position to the menu
          // loader. The position needs to be casted into a string; otherwise it
          // would be replaced with the bundle name string.
          if (isset($bundle_info['admin']['bundle argument'])) {
            $bundle_arg = $bundle_info['admin']['bundle argument'];
            $bundle_pos = (string) $bundle_arg;
          }
          else {
            $bundle_arg = $bundle_name;
            $bundle_pos = '0';
          }

          // This is the position of the %group_ui_menu placeholder in the
          // items below.
          $group_position = count(explode('/', $path)) + 1;

          // Extract access information, providing defaults.
          $access = array_intersect_key($bundle_info['admin'], drupal_map_assoc(array('access callback', 'access arguments')));
          $access += array(
            'access callback' => 'user_access',
            'access arguments' => array('administer site configuration'),
          );

          $items["$path/groups/%field_group_menu/delete"] = array(
            'load arguments' => array($entity_type, $bundle_arg, $bundle_pos, '%map'),
            'title' => 'Delete',
            'page callback' => 'drupal_get_form',
            'page arguments' => array('field_group_delete_form', $group_position),
            'type' => MENU_CALLBACK,
            'file' => 'field_group.admin.inc',
          ) + $access;
        }
      }
    }
  }

  return $items;
}

/**
 * Menu callback; present a form for removing a group.
 */
function field_group_delete_form($form, &$form_state, $group, $view_mode = 'form') {

  $form['#group'] = $group;
  $admin_path = _field_ui_bundle_admin_path($group->entity_type, $group->bundle);
  if ($view_mode == 'form') {
    $admin_path .= '/fields';
  }
  else {
    $admin_path .= '/display/'. $view_mode;
  }
  $form['#redirect'] = array($admin_path);
  $output = confirm_form($form,
    t('Are you sure you want to delete the group %group?', array('%group' => $group->label)),
    $admin_path,
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel'),
    'confirm'
  );
  return $output;
}

/**
 * Remove group from bundle.
 *
 * @todo we'll have to reset all view mode settings - that will be fun :)
 */
function field_group_delete_form_submit($form, &$form_state) {
  $group = $form['#group'];
  $bundle = $group->bundle;
  $entity_type = $group->entity_type;
  $mode = $form_state['build_info']['args'][1];

  $bundles = field_info_bundles();
  $bundle_label = $bundles[$entity_type][$bundle]['label'];

  db_delete('field_group')->condition('id', $group->id)->condition('mode', $mode)->execute();
  drupal_set_message(t('The group %group has been deleted from the %type content type.', array('%group' => $group->label, '%type' => $bundle_label)));
  cache_clear_all('field_groups', 'cache_field');

  // Redirect.
  $form_state['redirect'] = $form['#redirect'];
}