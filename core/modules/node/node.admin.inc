<?php
/**
 * @file
 * Content administration and module settings UI.
 */

/**
 * Menu callback: confirm rebuilding of permissions.
 *
 * @see node_configure_rebuild_confirm_submit()
 * @see node_menu()
 *
 * @ingroup forms
 */
function node_configure_rebuild_confirm() {
  return confirm_form(array(), t('Are you sure you want to rebuild the permissions on site content?'),
                  'admin/reports/status', t('This action rebuilds all permissions on site content, and may be a lengthy process. This action cannot be undone.'), t('Rebuild permissions'), t('Cancel'));
}

/**
 * Handler for wipe confirmation
 *
 * @see node_configure_rebuild_confirm()
 */
function node_configure_rebuild_confirm_submit($form, &$form_state) {
  node_access_rebuild(TRUE);
  $form_state['redirect'] = 'admin/reports/status';
}

/**
 * Updates all nodes in the passed-in array with the passed-in field values.
 *
 * IMPORTANT NOTE: This function is intended to work when called from a form
 * submission handler. Calling it outside of the form submission process may not
 * work correctly.
 *
 * @param array $nodes
 *   Array of node nids to update.
 * @param array $updates
 *   Array of key/value pairs with node field names and the value to update that
 *   field to.
 *
 * @ingroup batch
 */
function node_mass_update($nodes, $updates) {
  // We use batch processing to prevent timeout when updating a large number
  // of nodes.
  if (count($nodes) > 10) {
    $batch = array(
      'operations' => array(
        array('_node_mass_update_batch_process', array($nodes, $updates))
      ),
      'finished' => '_node_mass_update_batch_finished',
      'title' => t('Processing'),
      // We use a single multi-pass operation, so the default
      // 'Remaining x of y operations' message will be confusing here.
      'progress_message' => '',
      'error_message' => t('The update has encountered an error.'),
      // The operations do not live in the .module file, so we need to
      // tell the batch engine which file to load before calling them.
      'file' => backdrop_get_path('module', 'node') . '/node.admin.inc',
    );
    batch_set($batch);
  }
  else {
    foreach ($nodes as $nid) {
      _node_mass_update_helper($nid, $updates);
    }
    backdrop_set_message(t('The update has been performed.'));
  }
}

/**
 * Updates individual nodes when fewer than 10 are queued.
 *
 * @param $nid
 *   ID of node to update.
 * @param $updates
 *   Associative array of updates.
 *
 * @return object
 *   An updated node object.
 *
 * @see node_mass_update()
 */
function _node_mass_update_helper($nid, $updates) {
  $node = node_load($nid, NULL, TRUE);
  // For efficiency manually save the original node before applying any changes.
  $node->original = clone $node;
  foreach ($updates as $name => $value) {
    $node->$name = $value;
  }
  $node->save();
  return $node;
}

/**
 * Executes a batch operation for node_mass_update().
 *
 * @param array $nodes
 *   An array of node IDs.
 * @param array $updates
 *   Associative array of updates.
 * @param array $context
 *   An array of contextual key/values.
 */
function _node_mass_update_batch_process($nodes, $updates, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($nodes);
    $context['sandbox']['nodes'] = $nodes;
  }

  // Process nodes by groups of 5.
  $count = min(5, count($context['sandbox']['nodes']));
  for ($i = 1; $i <= $count; $i++) {
    // For each nid, load the node, reset the values, and save it.
    $nid = array_shift($context['sandbox']['nodes']);
    $node = _node_mass_update_helper($nid, $updates);

    // Store result for post-processing in the finished callback.
    $context['results'][] = l($node->title, 'node/' . $node->nid);

    // Update our progress information.
    $context['sandbox']['progress']++;
  }

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Menu callback: Reports the status of batch operation for node_mass_update().
 *
 * @param bool $success
 *   A boolean indicating whether the batch mass update operation successfully
 *   concluded.
 * @param int $results
 *   The number of nodes updated via the batch mode process.
 * @param array $operations
 *   An array of function calls (not used in this function).
 */
function _node_mass_update_batch_finished($success, $results, $operations) {
  if ($success) {
    backdrop_set_message(t('The update has been performed.'));
  }
  else {
    backdrop_set_message(t('An error occurred and processing did not complete.'), 'error');
    $message = format_plural(count($results), '1 item successfully processed:', '@count items successfully processed:');
    $message .= theme('item_list', array('items' => $results));
    backdrop_set_message($message);
  }
}

/**
 * Multiple node scheduling form.
 *
 * @see node_multiple_schedule_form_submit()
 *
 * @ingroup forms
 */
function node_multiple_schedule_form($form, &$form_state) {
  if (isset($form_state['nids'])) {
    $nids = $form_state['nids'];
  }
  elseif (isset($_SESSION['node_schedule_action']['timestamp']) && (REQUEST_TIME - $_SESSION['node_schedule_action']['timestamp'] < 6000)) {
    $nids = $_SESSION['node_schedule_action']['nids'];
    $form_state['nids'] = $nids;
    $form_state['cache'] = TRUE;
    unset($_SESSION['node_schedule_action']);
  }
  else {
    $nids = array();
  }

  $form['#tree'] = TRUE;

  if (empty($nids)) {
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content';
    $form['empty']['#markup'] = '<p>' . t('Return to the <a href="!url">content administration page</a>.', array('!url' => url($destination))) . '</p>';
    backdrop_set_message(t('No content has been selected for scheduling.'), 'error');
    return $form;
  }

  $form['node_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  $rows = db_query('SELECT nid, title, status FROM {node} WHERE nid IN (:nids)', array(':nids' => $nids))->fetchAllAssoc('nid');
  $already_published_count = 0;
  foreach ($rows as $nid => $node) {
    $form['nodes'][$nid] = array(
      '#type' => 'hidden',
      '#value' => $nid,
    );
    $form['node_list']['#items'][] = check_plain($node->title) . ($node->status ? ' (' . t('already published') . ')' : '');
    if ($node->status) {
      $already_published_count++;
    }
  }

  // Default the scheduled time to the current time + 1 day.
  $form['scheduled'] = array(
    '#type' => module_exists('date') ? 'date_popup' : 'textfield',
    '#title' => t('Publish on'),
    // Note #date_ properties only apply if date module is enabled.
    '#date_format' => 'Y-m-d g:i a',
    '#date_increment' => 10,
    '#default_value' => format_date(REQUEST_TIME + 86400, 'custom', 'Y-m-d g:i a'),
  );

  $form['unpublish'] = array(
    '#title' => t('Unpublish any published posts'),
    '#type' => 'checkbox',
    '#description' => t('If content is already published, it will not be scheduled. Check this option to unpublish this content.'),
    '#access' => $already_published_count > 0,
  );

  // Prepare cancel destination.
  if (isset($_GET['destination'])) {
    $options = backdrop_parse_url($_GET['destination']);
  }
  else {
    $options = array('path' => 'admin/content');
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Schedule publishing'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $options['path'],
    '#options' => $options,
  );

  return $form;
}

/**
 * Form submission handler for node_multiple_schedule_form().
 */
function node_multiple_schedule_form_submit($form, &$form_state) {
  $schedule_date = new BackdropDateTime($form_state['values']['scheduled']);
  $scheduled_timestamp = $schedule_date->format('U');
  $scheduled_count = 0;

  $nodes = node_load_multiple($form_state['values']['nodes']);
  foreach ($nodes as $node) {
    if ($form_state['values']['unpublish']) {
      $node->status = NODE_NOT_PUBLISHED;
    }
    if ($node->status == NODE_NOT_PUBLISHED) {
      $node->scheduled = $scheduled_timestamp;
      $scheduled_count++;
    }
  }
  watchdog('content', 'Scheduled @count posts.', array('@count' => $scheduled_count));

  backdrop_set_message(format_plural($scheduled_count, 'Scheduled 1 post.', 'Scheduled @count posts.'));
  $form_state['redirect'] = 'admin/content';
}

/**
 * Multiple node deletion confirmation form.
 *
 * @see node_multiple_delete_confirm_submit()
 *
 * @ingroup forms
 */
function node_multiple_delete_confirm($form, &$form_state) {
  if (isset($form_state['nids'])) {
    $nids = $form_state['nids'];
  }
  elseif (isset($_SESSION['node_delete_action']['timestamp']) && (REQUEST_TIME - $_SESSION['node_delete_action']['timestamp'] < 6000)) {
    $nids = $_SESSION['node_delete_action']['nids'];
    $form_state['nids'] = $nids;
    $form_state['cache'] = TRUE;
    unset($_SESSION['node_delete_action']);
  }
  else {
    $nids = array();
  }

  $form['#tree'] = TRUE;

  if (empty($nids)) {
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content';
    $form['empty']['#markup'] = '<p>' . t('Return to the <a href="!url">content administration page</a>.', array('!url' => url($destination))) . '</p>';
    backdrop_set_message(t('No content has been selected for deletion.'), 'error');
    return $form;
  }

  $form['node_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  $rows = db_query('SELECT nid, title FROM {node} WHERE nid IN (:nids)', array(':nids' => $nids))->fetchAllKeyed();
  foreach ($rows as $nid => $title) {
    $form['nodes'][$nid] = array(
      '#type' => 'hidden',
      '#value' => $nid,
    );
    $form['node_list']['#items'][] = check_plain($title);
  }

  $confirm_question = format_plural(count($rows), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, 'admin/content', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for node_multiple_delete_confirm().
 */
function node_multiple_delete_confirm_submit($form, &$form_state) {
  node_delete_multiple(array_keys($form_state['values']['nodes']));
  cache_clear_all();
  $count = count($form_state['values']['nodes']);
  watchdog('content', 'Deleted @count posts.', array('@count' => $count));

  backdrop_set_message(format_plural($count, 'Deleted 1 post.', 'Deleted @count posts.'));
  $form_state['redirect'] = 'admin/content';
}
