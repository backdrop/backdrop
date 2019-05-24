<?php

/**
 * @file
 * Admin page callbacks for the Comment module.
 */

/**
 * Form constructor for the confirmation form for bulk comment deletion.
 *
 * @ingroup forms
 * @see comment_admin()
 * @see comment_multiple_delete_confirm_submit()
 */
function comment_multiple_delete_confirm($form, &$form_state) {
  if (isset($form_state['cids'])) {
    $cids = $form_state['cids'];
  }
  elseif (isset($_SESSION['comment_delete_action']['timestamp']) && (REQUEST_TIME - $_SESSION['comment_delete_action']['timestamp'] < 6000)) {
    $cids = $_SESSION['comment_delete_action']['cids'];
    $form_state['cids'] = $cids;
    $form_state['cache'] = TRUE;
    unset($_SESSION['comment_delete_action']);
  }
  else {
    $cids = array();
  }

  $form['#tree'] = TRUE;

  if (empty($cids)) {
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content/comment';
    $form['empty']['#markup'] = '<p>' . t('Return to the <a href="!url">comment administration page</a>.', array('!url' => url($destination))) . '</p>';
    backdrop_set_message(t('No comments have been selected for deletion.'), 'error');
    return $form;
  }

  $form['comment_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );

  $edit = $form_state['input'];

  // array_filter() returns only elements with actual values.
  $comment_counter = 0;
  foreach (array_filter($cids) as $cid) {
    $comment = comment_load($cid);
    if (is_object($comment) && is_numeric($comment->cid)) {
      $subject = db_query('SELECT subject FROM {comment} WHERE cid = :cid', array(':cid' => $cid))->fetchField();
      $form['comments'][$cid] = array('#type' => 'hidden', '#value' => $cid, '#prefix' => '<li>', '#suffix' => check_plain($subject) . '</li>');
      $comment_counter++;
    }
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');

  if (!$comment_counter) {
    backdrop_set_message(t('There do not appear to be any comments to delete, or your selected comment was deleted by another administrator.'));
    backdrop_goto('admin/content/comment');
  }
  else {
    return confirm_form($form,
                        t('Are you sure you want to delete these comments and all their children?'),
                        'admin/content/comment', t('This action cannot be undone.'),
                        t('Delete comments'), t('Cancel'));
  }
}

/**
 * Form submission handler for comment_multiple_delete_confirm().
 */
function comment_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    comment_delete_multiple(array_keys($form_state['values']['comments']));
    cache_clear_all();
    $count = count($form_state['values']['comments']);
    watchdog('content', 'Deleted @count comments.', array('@count' => $count));
    backdrop_set_message(format_plural($count, 'Deleted 1 comment.', 'Deleted @count comments.'));
  }
  $form_state['redirect'] = 'admin/content/comment';
}

/**
 * Page callback: Shows a confirmation page for comment deletions.
 *
 * @param $cid
 *   The ID of the comment that is about to be deleted.
 *
 * @see comment_menu()
 * @see comment_confirm_delete()
 */
function comment_confirm_delete_page($cid) {
  if ($comment = comment_load($cid)) {
    return backdrop_get_form('comment_confirm_delete', $comment);
  }
  return MENU_NOT_FOUND;
}

/**
 * Form constructor for the confirmation form for comment deletion.
 *
 * @param $comment
 *   The comment that is about to be deleted.
 *
 * @ingroup forms
 * @see comment_confirm_delete_page()
 * @see comment_confirm_delete_submit()
 * @see confirm_form()
 */
function comment_confirm_delete($form, &$form_state, $comment) {
  $form['#comment'] = $comment;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['cid'] = array('#type' => 'value', '#value' => $comment->cid);
  return confirm_form(
    $form,
    t('Are you sure you want to delete the comment %title?', array('%title' => $comment->subject)),
    'node/' . $comment->nid,
    t('Any replies to this comment will be lost. This action cannot be undone.'),
    t('Delete'),
    t('Cancel'),
    'comment_confirm_delete');
}

/**
 * Form submission handler for comment_confirm_delete().
 */
function comment_confirm_delete_submit($form, &$form_state) {
  $comment = $form['#comment'];
  // Delete the comment and its replies.
  comment_delete($comment->cid);
  backdrop_set_message(t('The comment and all its replies have been deleted.'));
  watchdog('content', 'Deleted comment @cid and its replies.', array('@cid' => $comment->cid));

  $form_state['redirect'] = "node/$comment->nid";
}
