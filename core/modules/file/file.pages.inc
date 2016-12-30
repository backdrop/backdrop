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

  $description = '';
  $known_count = 0;
  $unknown_count = 0;
  $entity_list = _file_usage_list_links($file, $known_count, $unknown_count);

  if ($known_count || $unknown_count) {
    $description .= format_plural($known_count + $unknown_count, 'This file is referenced by one piece of content.', 'This file is referenced by @count pieces of content.');
  }

  if ($entity_list) {
    if ($unknown_count) {
      $entity_list[] = format_plural($unknown_count, 'And one additional unknown piece of content.', 'And @count additional unknown pieces of content.');
    }
    $description .= ' ' . t('Content referencing this file includes:');
    $description .= theme('item_list', array('items' => $entity_list));
  }
  elseif ($unknown_count) {
    $description .= ' ' . t('However, this content is either missing or you do not have access to it.');
  }
  else {
    $description .= t('This file has no known content referencing it, although it may still be in use.');
  }
  $description .= '<p>' . t('Deleting this file may cause content to display improperly.') . '</p>';

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
  if ($form_state['values']['confirm'] && ($file = file_load($form_state['values']['fid']))) {
    file_delete($file->fid);
    watchdog('file', 'Administrator deleted file %title.', array('%title' => $file->label()));
    backdrop_set_message(t('%title has been deleted.', array('%title' => $file->label())));
  }

  $form_state['redirect'] = '<front>';

  // Clear the page and block caches.
  cache_clear_all();
}

function _file_usage_list_links($file, &$known_count, &$unknown_count) {

  // Make a list of links showing where this file is in use. Files in use by
  // known entities are linked. Orphaned usages or ones not associated with an
  // entities are considered an "Unknown" usage. If the user does not have
  // access to the associated entity, it is considered unknown and no link is
  // shown.
  $known_count = 0;
  $unknown_count = 0;
  $entity_list = array();
  if ($file_usage_list = file_usage_list($file)) {
    foreach ($file_usage_list as $module_name => $module_usages) {
      foreach ($module_usages as $entity_type => $entity_usages) {
        $entity_type_info = entity_get_info($entity_type);
        $entity_controller = entity_get_controller($entity_type);
        $entity_ids = array();
        foreach ($entity_usages as $entity_id => $usage_count) {
          if ($entity_type_info) {
            $entity_ids[] = $entity_id;
          }
          else {
            $unknown_count += $usage_count;
          }
        }
        if ($entity_ids) {
          $entities = entity_load_multiple($entity_type, $entity_ids);

          // Perform an access check on all given entities.
          $query = new EntityFieldQuery();
          $query->entityCondition('entity_type', $entity_type);
          $query->entityCondition('entity_id', $entity_ids);
          $query->addTag($entity_type . '_access');
          $result = $query->execute();

          foreach ($entities as $entity) {
            // If the entity is in the EFQ result, access is allowed.
            if (isset($result[$entity_type][$entity->id()])) {
              $uri = $entity->uri();
              $entity_list[] = l($entity->label(), $uri['path'], $uri['options']);
              $known_count++;
            }
            // If not, do not show a link and consider it an unknown location.
            else {
              $unknown_count++;
            }
          }
        }
      }
    }
  }
  return $entity_list;
}