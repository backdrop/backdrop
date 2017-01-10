<?php

/**
 * @file
 * Class that holds information relating to a layouts context.
 *
 * This class does not extend the LayoutHandler class because contexts are not
 * stored directly within configuration. Instead contexts are determined based
 * on the Layout's path
 */
class LayoutRelationshipUserFromNode extends LayoutRelationship {
  /**
   * Get the context from this relationship.
   */
  function getContext($context) {
    if (isset($context->data->uid)) {
      // Load the user that is the author of the node.
      $uid = $context->data->uid;
      $account = user_load($uid);

      $context = layout_create_context('user', array());
      $context->data = $account;
        
      return $context;
    }
  }

}
