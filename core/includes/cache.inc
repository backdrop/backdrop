<?php

/**
 * @file
 * Functions and interfaces for cache handling.
 */

/**
 * Instantiates and statically caches the correct class for a cache bin.
 *
 * By default, this returns an instance of the BackdropDatabaseCache class.
 * Classes implementing BackdropCacheInterface can register themselves both as a
 * default implementation and for specific bins.
 *
 * @param $bin
 *   The cache bin for which the cache object should be returned, defaults to
 *   'cache'.
 *
 * @return BackdropCacheInterface
 *   The cache object associated with the specified bin.
 *
 * @see BackdropCacheInterface
 */
function cache($bin = 'cache') {
  // Temporary backwards compatibiltiy layer, allow old style prefixed cache
  // bin names to be passed as arguments.
  $bin = str_replace('cache_', '', $bin);

  // We do not use backdrop_static() here because we do not want to change the
  // storage of a cache bin mid-request.
  static $cache_objects;
  if (!isset($cache_objects[$bin])) {
    $class = settings_get('cache_class_' . $bin);
    if (!isset($class)) {
      $class = settings_get('cache_default_class', 'BackdropDatabaseCache');
    }
    $cache_objects[$bin] = new $class($bin);
  }
  return $cache_objects[$bin];
}

/**
 * Returns data from the persistent cache.
 *
 * Data may be stored as either plain text or as serialized data. cache_get()
 * will automatically return unserialized objects and arrays.
 *
 * @param string $cid
 *   The cache ID of the data to retrieve.
 * @param string $bin
 *   The cache bin from which data should be retrieved. Valid core values are
 *   "cache", "bootstrap", "field", "filter", "form", "menu", "page", "path",
 *   "update", "views", and "views_data". Bin names may include the prefix of
 *   "cache_", but it is stripped out before execution.
 *
 * @return
 *   The cache or FALSE on failure.
 */
function cache_get($cid, $bin = 'cache') {
  return cache($bin)->get($cid);
}

/**
 * Returns data from the persistent cache when given an array of cache IDs.
 *
 * @param $cids
 *   An array of cache IDs for the data to retrieve. This is passed by
 *   reference, and will have the IDs successfully returned from cache
 *   removed.
 * @param $bin
 *   The cache bin from which data should be retrieved. Valid core values are
 *   "cache", "bootstrap", "field", "filter", "form", "menu", "page", "path",
 *   "update", "views", and "views_data". Bin names may include the prefix of
 *   "cache_", but it is stripped out before execution
 *
 * @return
 *   An array of the items successfully returned from cache indexed by cid.
 */
function cache_get_multiple(array &$cids, $bin = 'cache') {
  return cache($bin)->getMultiple($cids);
}

/**
 * Stores data in the persistent cache.
 *
 * The persistent cache is split up into several cache bins. In the default
 * cache implementation, each cache bin corresponds to a database table by the
 * same name. Other implementations might want to store several bins in data
 * structures that get flushed together. While it is not a problem for most
 * cache bins if the entries in them are flushed before their expire time, some
 * might break functionality or are extremely expensive to recalculate. The
 * other bins are expired automatically by core. Contributed modules can add
 * additional bins and get them expired automatically by implementing
 * hook_flush_caches().
 *
 * The reasons for having several bins are as follows:
 * - Smaller bins mean smaller database tables and allow for faster selects and
 *   inserts.
 * - We try to put fast changing cache items and rather static ones into
 *   different bins. The effect is that only the fast changing bins will need a
 *   lot of writes to disk. The more static bins will also be better cacheable
 *   with MySQL's query cache.
 *
 * @param string $cid
 *   The cache ID of the data to store.
 * @param mixed $data
 *   The data to store in the cache. Complex data types will be automatically
 *   serialized before insertion. Strings will be stored as plain text and are
 *   not serialized.
 * @param string $bin
 *   The cache bin in which data should be stored. Valid core values are
 *   "cache", "bootstrap", "field", "filter", "form", "menu", "page", "path",
 *   "update", "views", and "views_data". Bin names may include the prefix of
 *   "cache_", but it is stripped out before execution
 * @param int $expire
 *   One of the following values:
 *   - CACHE_PERMANENT: Indicates that the item should never be removed unless
 *     explicitly told to using cache_clear_all() with a cache ID.
 *   - CACHE_TEMPORARY: Indicates that the item should be removed at the next
 *     general cache wipe.
 *   - A Unix timestamp: Indicates that the item should be kept at least until
 *     the given time, after which it behaves like CACHE_TEMPORARY.
 *
 * @see cache_get()
 */
function cache_set($cid, $data, $bin = 'cache', $expire = CACHE_PERMANENT) {
  return cache($bin)->set($cid, $data, $expire);
}

/**
 * Flushes all cache items in a bin.
 *
 * @param string $bin
 *   The bin whose data should be emptied.
 */
function cache_flush($bin) {
  return cache($bin)->flush();
}

/**
 * Clears data from the cache.
 *
 * If called with the arguments $cid and $bin set to NULL or omitted, then
 * expirable entries will be cleared from the page and layout bins, and the
 * $wildcard argument is ignored.
 *
 * @param string $cid
 *   The cache ID or an array of cache IDs. Otherwise, all cache entries that
 *   can expire are deleted.
 * @param string $bin
 *   The cache bin whose data should be cleared. Mandatory argument if $cid is
 *   set.
 * @param bool $wildcard
 *   If TRUE, the $cid argument must contain a string value and cache IDs
 *   starting with $cid are deleted in addition to the exact cache ID specified
 *   by $cid. If $wildcard is TRUE and $cid is '*', the entire cache is emptied.
 *
 *   The wildcard parameter is a legacy argument. If needing to do a full bin
 *   flush, use cache_flush() instead. Prefix-based flushes are also
 *   discouraged, as not all cache backends natively support wildcard
 *   functionality.
 *
 * @see cache_flush()
 */
function cache_clear_all($cid = NULL, $bin = NULL, $wildcard = FALSE) {
  // Default value cache flush.
  if (!isset($cid) && !isset($bin)) {
    if (module_exists('layout')) {
      cache('layout_path')->flush();
    }
    cache('page')->flush();
    return;
  }

  if (!$wildcard) {
    if (is_null($cid)) {
      cache($bin)->garbageCollection();
    }
    else {
      cache($bin)->delete($cid);
    }
  }
  else {
    if ($cid === '*') {
      cache($bin)->flush();
    }
    else {
      cache($bin)->deletePrefix($cid);
    }
  }
}

/**
 * Checks if a cache bin is empty.
 *
 * A cache bin is considered empty if it does not contain any valid data for any
 * cache ID.
 *
 * @param string $bin
 *   The cache bin to check.
 *
 * @return
 *   TRUE if the cache bin specified is empty.
 */
function cache_is_empty($bin) {
  return cache($bin)->isEmpty();
}

/**
 * Defines an interface for cache implementations.
 *
 * All cache implementations have to implement this interface.
 * BackdropDatabaseCache provides the default implementation, which can be
 * consulted as an example.
 *
 * To make Backdrop use your implementation for a certain cache bin, you have to
 * set a value in settings.php with the name of the cache bin as its key and the
 * name of your class as its value. For example, if your implementation of
 * BackdropCacheInterface was called MyCustomCache, the following line in
 * settings.php would make Backdrop use it for the 'cache_page' bin:
 * @code
 *  $settings['cache_class_cache_page'] = 'MyCustomCache';
 * @endcode
 *
 * Additionally, you can register your cache implementation to be used by
 * default for all cache bins by setting the $settings['cache_default_class'] to
 * the name of your implementation of the BackdropCacheInterface, e.g.
 * @code
 *  $settings['cache_default_class'] = 'MyCustomCache';
 * @endcode
 *
 * To implement a completely custom cache bin, use the same format:
 * @code
 *  $settings['cache_class_custom_bin'] = 'MyCustomCache';
 * @endcode
 * To access your custom cache bin, specify the name of the bin when storing
 * or retrieving cached data:
 * @code
 *  cache('custom_bin')->set($cid, $data, $expire);
 *  cache('custom_bin')->get($cid, 'custom_bin');
 * @endcode
 *
 * @see cache()
 * @see BackdropDatabaseCache
 */
interface BackdropCacheInterface {

  /**
   * Returns data from the persistent cache.
   *
   * Data may be stored as either plain text or as serialized data.
   * cache()->get() will automatically return unserialized objects and arrays.
   *
   * @param string $cid
   *   The cache ID of the data to retrieve.
   *
   * @return
   *   The cache or FALSE on failure.
   */
  function get($cid);

  /**
   * Returns data from the persistent cache when given an array of cache IDs.
   *
   * @param array $cids
   *   An array of cache IDs for the data to retrieve. This is passed by
   *   reference, and will have the IDs successfully returned from cache
   *   removed.
   *
   * @return
   *   An array of the items successfully returned from cache indexed by cid.
   */
   function getMultiple(array &$cids);

  /**
   * Stores data in the persistent cache.
   *
   * @param string $cid
   *   The cache ID of the data to store.
   * @param mixed $data
   *   The data to store in the cache. Complex data types will be automatically
   *   serialized before insertion. Strings will be stored as plain text and not
   *   serialized. Some storage engines only allow objects up to a maximum of
   *   1MB in size to be stored by default. When caching large arrays or
   *   similar, take care to ensure $data does not exceed this size.
   * @param int $expire
   *   (optional) One of the following values:
   *   - CACHE_PERMANENT: Indicates that the item should never be removed unless
   *     explicitly told to using cache_clear_all() with a cache ID.
   *   - CACHE_TEMPORARY: Indicates that the item should be removed at the next
   *     general cache wipe.
   *   - A Unix timestamp: Indicates that the item should be kept at least until
   *     the given time, after which it behaves like CACHE_TEMPORARY.
   */
  function set($cid, $data, $expire = CACHE_PERMANENT);

  /**
   * Deletes an item from the cache.
   *
   * @param string $cid
   *    The cache ID to delete.
   */
  function delete($cid);

  /**
   * Deletes multiple items from the cache.
   *
   * @param $cids
   *   An array of $cids to delete.
   */
  function deleteMultiple(Array $cids);

  /**
   * Deletes items from the cache using a wildcard prefix.
   *
   * @param string $prefix
   *   A wildcard prefix.
   */
  function deletePrefix($prefix);

  /**
   * Flushes all cache items in a bin.
   */
  function flush();

  /**
   * Performs garbage collection on a cache bin. Removing expired items.
   */
  function garbageCollection();

  /**
   * Checks if a cache bin is empty.
   *
   * A cache bin is considered empty if it does not contain any valid data for
   * any cache ID.
   *
   * @return
   *   TRUE if the cache bin specified is empty.
   */
  function isEmpty();
}
