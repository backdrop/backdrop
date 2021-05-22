<?php
/**
 * @file
 * Entity controller and class for comments.
 */

/**
 * Defines the comment entity class.
 */
class Comment extends Entity {

  /**
   * The comment ID.
   *
   * @var integer
   */
  public $cid;

  /**
   * The parent comment ID if this is a reply to a comment.
   *
   * @var integer
   */
  public $pid;

  /**
   * The comment type (bundle).
   *
   * @var string
   */
  public $node_type;

  /**
   * The comment language code.
   *
   * @var string
   */
  public $langcode = LANGUAGE_NONE;

  /**
   * The comment title.
   *
   * @var string
   */
  public $subject;

  /**
   * The comment author ID.
   *
   * @var integer
   */
  public $uid = 0;

  /**
   * The comment author's name.
   *
   * For anonymous authors, this is the value as typed in the comment form.
   *
   * @var string
   */
  public $name = '';

  /**
   * The comment author's e-mail address.
   *
   * For anonymous authors, this is the value as typed in the comment form.
   *
   * @var string
   */
  public $mail;

  /**
   * The comment author's home page address.
   *
   * For anonymous authors, this is the value as typed in the comment form.
   *
   * @var string
   */
  public $homepage;

  /**
   * A list of comment IDs that make up the parents of this comment.
   *
   * For example a thread of 10/20/30 would indicate a comment that has 3 parent
   * comments, with CID 30 being the immediate parent comment.
   *
   * @var string
   */
  public $thread;

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return $this->cid;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'comment';
  }

  /**
   * Implements EntityInterface::bundle().
   */
  public function bundle() {
    return $this->node_type;
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return $this->subject;
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => 'comment/' . $this->cid,
      'options' => array('fragment' => 'comment-' . $this->cid),
    );
  }

  /**
   * Overrides Entity::createAccess().
   */
  public static function createAccess($bundle = NULL, $account = NULL) {
    // If no user object is supplied, the access check is for the current user.
    if (empty($account)) {
      $account = $GLOBALS['user'];
    }

    return user_access('post comments', $account);
  }

  /**
   * Overrides Entity::access().
   *
   * @param string $op
   *   The operation to be performed on the node. Possible values are:
   *   - view
   *   - update
   *   - approve
   *   - delete
   * @param User|AnonymousUser|object $account
   *   (optional) The user to check for. Leave it to NULL to check for the
   *   global user.
   *
   * @return bool
   *   TRUE if access is granted, FALSE otherwise.
   */
  public function access($op, $account = NULL) {
    // Casting class with private property causes errors due to added prefix.
    // e.g. "\0" . 'Comment' . "\0". So use static array instead.
    $rights = &backdrop_static(__METHOD__, array());

    if ($op == 'create') {
      return self::createAccess(NULL, $account);
    }
    elseif (!in_array($op, array('view', 'update', 'approve', 'delete'), TRUE)) {
      // If the $op was not one of the supported ones, we return access denied.
      return FALSE;
    }
    // If no user object is supplied, the access check is for the current user.
    if (empty($account)) {
      $account = $GLOBALS['user'];
    }

    $cid = $this->id();

    // If we've already checked access for this node, user and op, return from
    // cache.
    if (isset($rights[$account->uid][$cid][$op])) {
      return $rights[$account->uid][$cid][$op];
    }

    if ($op == 'view') {
      $rights[$account->uid][$cid][$op] = user_access('access comments', $account);
      return $rights[$account->uid][$cid][$op];
    }
    elseif ($op == 'update') {
      $rights[$account->uid][$cid][$op] = ($account->uid && $account->uid == $this->uid && $this->status == COMMENT_PUBLISHED && user_access('edit own comments', $account)) || user_access('administer comments', $account);
      return $rights[$account->uid][$cid][$op];
    }
    elseif ($op == 'approve') {
      $rights[$account->uid][$cid][$op] = user_access('administer comments', $account);
      return $rights[$account->uid][$cid][$op];
    }
    elseif ($op == 'delete') {
      $rights[$account->uid][$cid][$op] = user_access('administer comments', $account);
      return $rights[$account->uid][$cid][$op];
    }

    $rights[$account->uid][$cid][$op] = FALSE;
    return $rights[$account->uid][$cid][$op];
  }

}

/**
 * Defines the controller class for comments.
 *
 * This extends the EntityDatabaseStorageController class, adding required
 * special handling for comment entities.
 */
class CommentStorageController extends EntityDatabaseStorageController {

  /**
   * Overrides DefaultEntityController::load().
   */
  public function load($ids = array(), $conditions = array()) {
    $comments = parent::load($ids, $conditions);

    foreach ($comments as $key => $comment) {
      $comment->new = node_mark($comment->nid, $comment->changed);
    }
    return $comments;
  }

  /**
   * Overrides EntityDatabaseStorageController::buildQuery().
   */
  protected function buildQuery($ids, $conditions = array(), $revision_id = FALSE) {
    $query = parent::buildQuery($ids, $conditions, $revision_id);
    // Specify additional fields from the user and node tables.
    $query->innerJoin('node', 'n', 'base.nid = n.nid');
    $query->addField('n', 'type', 'node_type');
    $query->innerJoin('users', 'u', 'base.uid = u.uid');
    $query->addField('u', 'name', 'registered_name');
    $query->fields('u', array('uid', 'signature', 'signature_format', 'picture'));
    return $query;
  }

  /**
   * Overrides EntityDatabaseStorageController::attachLoad().
   */
  protected function attachLoad(&$comments, $revision_id = FALSE) {
    // Set up standard comment properties.
    foreach ($comments as $key => $comment) {
      $comment->name = $comment->uid ? $comment->registered_name : $comment->name;
      $comment->node_type = 'comment_node_' . $comment->node_type;
      $comments[$key] = $comment;
    }
    parent::attachLoad($comments, $revision_id);
  }

  /**
   * Overrides EntityDatabaseStorageController::preSave().
   *
   * @see comment_int_to_alphadecimal()
   * @see comment_increment_alphadecimal()
   */
  protected function preSave(EntityInterface $comment) {
    global $user;

    if (!isset($comment->status)) {
      $comment->status = user_access('skip comment approval') ? COMMENT_PUBLISHED : COMMENT_NOT_PUBLISHED;
    }
    // Make sure we have a proper bundle name.
    if (!isset($comment->node_type)) {
      $node = node_load($comment->nid);
      $comment->node_type = 'comment_node_' . $node->type;
    }
    if (!$comment->cid) {
      // Add the comment to database. This next section builds the thread field.
      // Also see the documentation for comment_view().
      if (!empty($comment->thread)) {
        // Allow calling code to set thread itself.
        $thread = $comment->thread;
      }
      elseif ($comment->pid == 0) {
        // This is a comment with no parent comment (depth 0): we start
        // by retrieving the maximum thread level.
        $max = db_query('SELECT MAX(thread) FROM {comment} WHERE nid = :nid', array(':nid' => $comment->nid))->fetchField();
        // Strip the "/" from the end of the thread.
        $max = rtrim($max, '/');
        // We need to get the value at the correct depth.
        $parts = explode('.', $max);
        $firstsegment = $parts[0];
        // Finally, build the thread field for this new comment.
        $thread = comment_increment_alphadecimal($firstsegment) .'/';
      }
      else {
        // This is a comment with a parent comment, so increase the part of
        // the thread value at the proper depth.

        // Get the parent comment:
        $parent = comment_load($comment->pid);
        // Strip the "/" from the end of the parent thread.
        $parent->thread = (string) rtrim((string) $parent->thread, '/');
        // Get the max value in *this* thread.
        $max = db_query("SELECT MAX(thread) FROM {comment} WHERE thread LIKE :thread AND nid = :nid", array(
          ':thread' => $parent->thread . '.%',
          ':nid' => $comment->nid,
        ))->fetchField();

        if ($max == '') {
          // First child of this parent.
          $thread = $parent->thread . '.' . comment_int_to_alphadecimal(0) . '/';
        }
        else {
          // Strip the "/" at the end of the thread.
          $max = rtrim($max, '/');
          // Get the value at the correct depth.
          $parts = explode('.', $max);
          $parent_depth = count(explode('.', $parent->thread));
          $last = $parts[$parent_depth];
          // Finally, build the thread field for this new comment.
          $thread = $parent->thread . '.' . comment_increment_alphadecimal($last) . '/';
        }
      }
      if (empty($comment->created)) {
        $comment->created = REQUEST_TIME;
      }
      if (empty($comment->changed)) {
        $comment->changed = $comment->created;
      }
      // We test the value with '===' because we need to modify anonymous
      // users as well.
      if ($comment->uid === $user->uid && isset($user->name)) {
        $comment->name = $user->name;
      }
      // Add the values which aren't passed into the function.
      $comment->thread = $thread;
      $comment->hostname = ip_address();
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::postSave().
   */
  protected function postSave(EntityInterface $comment, $update) {
    // Update the {node_comment_statistics} table prior to executing the hook.
    $this->updateNodeStatistics($comment->nid);
    if ($comment->status == COMMENT_PUBLISHED) {
      module_invoke_all('comment_publish', $comment);
    }

    // If entity caching is enabled on nodes, clear the cache for the node.
    $node_info = entity_get_info('node');
    if (isset($node_info['entity cache']) && $node_info['entity cache']) {
      cache('entity_node')->delete($comment->nid);
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::postDelete().
   */
  protected function postDelete($comments) {
    // Delete the comments' replies.
    $query = db_select('comment', 'c')
      ->fields('c', array('cid'))
      ->condition('pid', array(array_keys($comments)), 'IN');
    $child_cids = $query->execute()->fetchCol();
    comment_delete_multiple($child_cids);

    foreach ($comments as $comment) {
      $this->updateNodeStatistics($comment->nid);
      $nodes[] = $comment->nid;
    }

    // If entity caching is enabled on nodes, clear the cache for the nodes that
    // the deleted comments were on.
    $node_info = entity_get_info('node');
    if (isset($node_info['entity cache']) && $node_info['entity cache']) {
      foreach (array_unique($nodes) as $nid) {
        cache('entity_node')->delete($nid);
      }
    }

  }

  /**
   * Updates the comment statistics for a given node.
   *
   * The {node_comment_statistics} table has the following fields:
   * - last_comment_timestamp: The timestamp of the last comment for this node,
   *   or the node created timestamp if no comments exist for the node.
   * - last_comment_name: The name of the anonymous poster for the last comment.
   * - last_comment_uid: The user ID of the poster for the last comment for
   *   this node, or the node author's user ID if no comments exist for the
   *   node.
   * - comment_count: The total number of approved/published comments on this
   *   node.
   *
   * @param $nid
   *   The node ID.
   */
  protected function updateNodeStatistics($nid) {
    // Allow bulk updates and inserts to temporarily disable the
    // maintenance of the {node_comment_statistics} table.
    if (!state_get('comment_maintain_node_statistics', TRUE)) {
      return;
    }

    $count = db_query('SELECT COUNT(cid) FROM {comment} WHERE nid = :nid AND status = :status', array(
      ':nid' => $nid,
      ':status' => COMMENT_PUBLISHED,
    ))->fetchField();

    if ($count > 0) {
      // Comments exist.
      $last_reply = db_query_range('SELECT cid, name, changed, uid FROM {comment} WHERE nid = :nid AND status = :status ORDER BY cid DESC', 0, 1, array(
        ':nid' => $nid,
        ':status' => COMMENT_PUBLISHED,
      ))->fetchObject();
      db_update('node_comment_statistics')
        ->fields(array(
          'cid' => $last_reply->cid,
          'comment_count' => $count,
          'last_comment_timestamp' => $last_reply->changed,
          'last_comment_name' => $last_reply->uid ? '' : $last_reply->name,
          'last_comment_uid' => $last_reply->uid,
        ))
        ->condition('nid', $nid)
        ->execute();
    }
    else {
      // Comments do not exist.
      $node = db_query('SELECT uid, created FROM {node} WHERE nid = :nid', array(':nid' => $nid))->fetchObject();
      db_update('node_comment_statistics')
        ->fields(array(
          'cid' => 0,
          'comment_count' => 0,
          'last_comment_timestamp' => $node->created,
          'last_comment_name' => '',
          'last_comment_uid' => $node->uid,
        ))
        ->condition('nid', $nid)
        ->execute();
    }
  }
}
