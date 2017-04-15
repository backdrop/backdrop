<?php

/**
 * @file
 */

/**
 * Taxonomy term entity controller with persistent cache.
 */
class EntityCacheTaxonomyTermController extends TaxonomyTermController {
  public function resetCache(array $ids = NULL) {
    EntityCacheControllerHelper::resetEntityCache($this, $ids);
    parent::resetCache($ids);
  }
  public function load($ids = array(), $conditions = array()) {
    return EntityCacheControllerHelper::entityCacheLoad($this, $ids, $conditions);
  }
}

/**
 * Taxonomy vocabulary entity controller with persistent cache.
 */
class EntityCacheTaxonomyVocabularyController extends TaxonomyVocabularyController {
  public function resetCache(array $ids = NULL) {
    EntityCacheControllerHelper::resetEntityCache($this, $ids);
    parent::resetCache($ids);
  }
  public function load($ids = array(), $conditions = array()) {
    return EntityCacheControllerHelper::entityCacheLoad($this, $ids, $conditions);
  }
}
