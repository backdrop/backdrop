<?php

/**
 * @file
 * Class that holds information relating to the user from node relationship.
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
      'label' => $this->getAdminSummary(),
    ));
    $user_context->setData($account);

    return $user_context;
  }

  /**
   * Return a summary for this relationship.
   */
  function getAdminSummary() {
    if (!empty($this->settings['context_parent'])) {
      list($label, $value) = explode(':', $this->settings['context_parent']);
      if ($label == 'position') {
        return t('User from node in position @position', array('@position' => $value+1));
      }
      else {
        return t('User from node @id', array('@id' => $value));
      }
    }
    return t('User from node');
  }

}
