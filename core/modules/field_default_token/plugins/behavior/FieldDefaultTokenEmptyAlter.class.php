<?php

/**
 * @file
 * Additional behavior for Entity reference fields.
 */

/**
 * Prevents field value containing tokens from being treated as empty.
 */
class FieldDefaultTokenEmptyAlter extends EntityReference_BehaviorHandler_Abstract {

  /**
   * Alter the empty status of a field item.
   */
  public function is_empty_alter(&$empty, $item, $field) {
    // If field value contains tokens, entityreference.module treats it as empty.
    if (($empty) && (isset($item['target_id'])) && (strpos($item['target_id'], '[') !== FALSE)) {
      $empty = FALSE;
    }
  }

}
