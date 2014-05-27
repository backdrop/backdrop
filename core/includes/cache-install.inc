<?php

/**
 * @file
 * Provides a stub cache implementation to be used during installation.
 */

/**
 * Defines a stub cache implementation to be used during installation.
 *
 * The stub implementation is needed when database access is not yet available.
 * Because Backdrop's caching system never requires that cached data be present,
 * these stub functions can short-circuit the process and sidestep the need for
 * any persistent storage. Obviously, using this cache implementation during
 * normal operations would have a negative impact on performance.
 */
class BackdropFakeCache extends BackdropDatabaseCache implements BackdropCacheInterface {

  /**
   * Overrides BackdropDatabaseCache::get().
   */
  function get($cid) {
    return FALSE;
  }

  /**
   * Overrides BackdropDatabaseCache::getMultiple().
   */
  function getMultiple(&$cids) {
    return array();
  }

  /**
   * Overrides BackdropDatabaseCache::set().
   */
  function set($cid, $data, $expire = CACHE_PERMANENT) {
  }

  /**
   * Implements BackdropCacheInterface::deletePrefix().
   */
  function deletePrefix($cid) {
    try {
      if (class_exists('Database')) {
        parent::deletePrefix($cid);
      }
    }
    catch (Exception $e) {
    }
  }

  /**
   * Overrides BackdropDatabaseCache::clear().
   */
  function clear($cid = NULL, $wildcard = FALSE) {
    // If there is a database cache, attempt to clear it whenever possible. The
    // reason for doing this is that the database cache can accumulate data
    // during installation due to any full bootstraps that may occur at the
    // same time (for example, Ajax requests triggered by the installer). If we
    // didn't try to clear it whenever this function is called, the data in the
    // cache would become stale; for example, the installer sometimes calls
    // variable_set(), which updates the {variable} table and then clears the
    // cache to make sure that the next page request picks up the new value.
    // Not actually clearing the cache here therefore leads old variables to be
    // loaded on the first page requests after installation, which can cause
    // subtle bugs, some of which would not be fixed unless the site
    // administrator cleared the cache manually.
    try {
      if (class_exists('Database')) {
        parent::clear($cid, $wildcard);
      }
    }
    // If the attempt at clearing the cache causes an error, that means that
    // either the database connection is not set up yet or the relevant cache
    // table in the database has not yet been created, so we can safely do
    // nothing here.
    catch (Exception $e) {
    }
  }

  /**
   * Overrides BackdropDatabaseCache::isEmpty().
   */
  function isEmpty() {
    return TRUE;
  }
}
