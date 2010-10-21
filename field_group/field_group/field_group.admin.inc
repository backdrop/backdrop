<?php
// $Id $

/**
 * @file
 * Field group administrative functions.
 */

function field_group_edit_form($form, &$form_state, $group) {
  $output = array();



  return $output;
}

/**
 * Menu callback; present a form for removing a group.
 */
function field_group_delete_form($form, &$form_state, $group) {
  // @todo broken - see field_group_menu_load().
  $form['#group'] = $group;

  // @todo this shouldn't redirect to fields - this can be display too.
  $admin_path = _field_ui_bundle_admin_path($group->entity_type, $group->bundle);

  $output = confirm_form($form,
  t('Are you sure you want to delete the group %group?', array('%group' => $group->label)),
  $admin_path . '/fields',
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

  $bundles = field_info_bundles();
  $bundle_label = $bundles[$entity_type][$bundle]['label'];

  db_delete('field_group')->condition('id', $group->id)->execute();
  drupal_set_message(t('The group %group has been deleted from the %type content type.', array('%group' => $group->label, '%type' => $bundle_label)));
  cache_clear_all('field_groups', 'cache_field');

  // @todo this shouldn't redirect to fields - this can be display too.
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
  $form_state['redirect'] = array($admin_path . '/fields');
}
