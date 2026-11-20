<?php
/**
 * @file
 * Hooks for translation module.
 */

/**
 * Respond to changes in a node's translation set.
 *
 * The node object is extended with a 'translation_change' array containing
 * both the old and new tnid.
 *
 * @param Node $node
 *   The node being acted upon.
 */
function hook_node_translation_change(Node $node) {
  $update = db_update('my_table')
    ->fields(array(
      'tnid' => $node->translation_change['new_tnid'],
    ))
    ->condition('tnid', $node->tnid, '=')
    ->execute();
}
