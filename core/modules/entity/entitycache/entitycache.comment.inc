<?php

/**
 * @file
 */

/**
 * Comment entity controller with persistent cache.
 */
class EntityCacheCommentController extends CommentController {
  public function resetCache(array $ids = NULL) {
    EntityCacheControllerHelper::resetEntityCache($this, $ids);
    parent::resetCache($ids);
  }
  public function load($ids = array(), $conditions = array()) {
    return EntityCacheControllerHelper::entityCacheLoad($this, $ids, $conditions);
  }
}
