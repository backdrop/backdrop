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
    $account = NULL;
    if (isset($context->data->uid)) {
      // Load the user that is the author of the node.
      $uid = $context->data->uid;
      $account = user_load($uid);
    }
    $user_context = layout_create_context('user', array(
      'position' => $context->position,
    ));
    $user_context->setData($account);

    return $user_context;
  }

  /**
   * Return a summary for this relationship.
   */
  function getAdminSummary() {
    if (!empty($this->settings['context_parent'])) {
      $parts = explode(':', $this->settings['context_parent']);
      if ($parts[0] == 'position') {
        return t('User from node in position ') . $parts[1] + 1;
      }
      else {
        return t('User from node ') . $parts[1];
      }
    }
    return t('User from node');
  }

}
