<?php
// $Id$

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