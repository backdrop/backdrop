<?php
/**
 * @file
 * Action callbacks for Node module.
 */

/**
 * Sets the status of a node to 1 (published).
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_publish_action(Node $node, &$context) {
    $node->status = NODE_PUBLISHED;
    $node->save();
  }

/**
 * Sets the status of a node to 0 (unpublished).
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action..
 *
 * @ingroup actions
 */
function node_unpublish_action(Node $node, &$context) {
    $node->status = NODE_NOT_PUBLISHED;
    $node->save();
  }

/**
 * Sets the sticky-at-top-of-list property of a node to 1.
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_make_sticky_action(Node $node, &$context) {
    $node->sticky = NODE_STICKY;
    $node->save();
  }

  /**
 * Sets the sticky-at-top-of-list property of a node to 0.
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_make_unsticky_action(Node $node, &$context) {
    $node->sticky = NODE_NOT_STICKY;
    $node->save();
  }

  /**
 * Sets the promote property of a node to 1.
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_promote_action(Node $node, &$context) {
    $node->promote = NODE_PROMOTED;
    $node->save();
  }

  /**
 * Sets the promote property of a node to 0.
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_unpromote_action(Node $node, &$context) {
    $node->promote = NODE_NOT_PROMOTED;
    $node->save();
  }

/**
 * Queues a node for deletion.
 *
 * @param Node $node
 *   A node entity.
 * @param $context
 *   Contextual information about the triggered action.
 *
 * @ingroup actions
 */
function node_delete_action(Node $node, &$context) {
    // Save the list of nodes to be deleted in the session. Append to the existing
    // list if within the last minute, otherwise start a new list of nodes.
    $last_action_time = 0;
    if (isset($_SESSION['node_delete_action'])) {
      $last_action_time = $_SESSION['node_delete_action']['timestamp'];
    }
    if (REQUEST_TIME - $last_action_time > 60) {
      $_SESSION['node_delete_action'] = array(
        'timestamp' => REQUEST_TIME,
        'nids' => array(),
      );
    }
  
    $_SESSION['node_delete_action']['nids'][] = $node->nid;
    $context['redirect'] = 'admin/content/node/delete';
  }
  

