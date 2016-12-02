<?php
/**
 * @file
 * Supports file operations including View, Edit, and Delete.
 */

/**
 * Page callback: Form constructor for the file deletion confirmation form.
 *
 * Path: file/%file/delete
 *
 * @param object $file
 *   A file object from file_load().
 *
 * @see file_menu()
 */
function file_delete_form($form, &$form_state, $file) {
  $form_state['file'] = $file;

  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $file->fid,
  );

  $description = t('This action cannot be undone.');
  if ($references = file_usage_list($file)) {
    $description .= ' ' . t('This file is currently in use and may cause problems if deleted.');
  }

  return confirm_form($form,
    t('Are you sure you want to delete the file %title?', array(
      '%title' => entity_label('file', $file),
    )),
    'file/' . $file->fid,
    $description,
    t('Delete')
  );
}

/**
 * Form submission handler for file_delete_form().
 */
function file_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm'] && $file = file_load($form_state['values']['fid'])) {
    // Use file_delete_multiple() rather than file_delete() since we want to
    // avoid unwanted validation and usage checking.
    file_delete_multiple(array($file->fid));

    $args = array(
      '@type' => file_type_get_name($file),
      '%title' => entity_label('file', $file),
    );
    watchdog('file', '@type: deleted %title.', $args);
    drupal_set_message(t('@type %title has been deleted.', $args));
  }

  $form_state['redirect'] = '<front>';

  // Clear the page and block caches.
  cache_clear_all();
}
