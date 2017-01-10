<?php

/**
 * @file
 * Plugin to provide an relationship handler for node from user.
 */

/**
 * Return a new context based on an existing context.
 */
function user_from_node_context($context, $conf) {
  if (isset($context->data->uid)) {
    // Load the user that is the author of the node.
    $uid = $context->data->uid;
    $account = user_load($uid);

    $context = layout_create_context('user', array());
    $context->data = $account;
      
    return $context;
  }
}
