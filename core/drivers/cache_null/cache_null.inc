<?php

/**
 * Defines a stub cache implementation.
 *
 * The stub implementation is needed when database access is not yet available.
 * Because Backdrop's caching system never requires that cached data be present,
 * these stub functions can short-circuit the process and sidestep the need for
 * any persistent storage. Using this cache implementation during normal
 * operations would have a negative impact on performance.
 *
 * This also can be used for testing purposes.
 */
class BackdropNullCache implements BackdropCacheInterface {

  /**
   * Constructs a BackdropNullCache object.
   *
   * @param $bin
   *   The cache bin for which the object is created.
   */
  function __construct($bin) {}

  /**
   * Implements BackdropCacheInterface::get().
   */
  function get($cid) {
    return FALSE;
  }

  /**
   * Implements BackdropCacheInterface::getMultiple().
   */
  function getMultiple(array &$cids) {
    return array();
  }

  /**
   * Implements BackdropCacheInterface::set().
   */
  function set($cid, $data, $expire = CACHE_PERMANENT) {}

  /**
   * Implements BackdropCacheInterface::delete().
   */
  function delete($cid) {}

  /**
   * Implements BackdropCacheInterface::deleteMultiple().
   */
  function deleteMultiple(array $cids) {}

  /**
   * Implements BackdropCacheInterface::deletePrefix().
   */
  function deletePrefix($prefix) {}

  /**
   * Implements BackdropCacheInterface::flush().
   */
  function flush() {}

  /**
   * Implements BackdropCacheInterface::expire().
   */
  function expire() {}

  /**
   * Implements BackdropCacheInterface::garbageCollection().
   */
  function garbageCollection() {}

  /**
   * Implements BackdropCacheInterface::isEmpty().
   */
  function isEmpty() {
    return TRUE;
  }
}
