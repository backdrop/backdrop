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
 * Expires data from the block and page caches.
 */
function cache_clear_all() {
  // @todo: remove before release.
  if (func_get_args()) {
    throw new Exception(t('cache_clear_all() no longer takes arguments, use cache() instead.'));
  }
  // Clear the block cache first, so stale data will
  // not end up in the page cache.
  if (module_exists('block')) {
    cache('block')->expire();
  }
  cache('page')->expire();
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
   * @param $cid
   *   The cache ID of the data to retrieve.
   *
   * @return
   *   The cache or FALSE on failure.
   */
  function get($cid);

  /**
   * Returns data from the persistent cache when given an array of cache IDs.
   *
   * @param $cids
   *   An array of cache IDs for the data to retrieve. This is passed by
   *   reference, and will have the IDs successfully returned from cache
   *   removed.
   *
   * @return
   *   An array of the items successfully returned from cache indexed by cid.
   */
   function getMultiple(&$cids);

  /**
   * Stores data in the persistent cache.
   *
   * @param $cid
   *   The cache ID of the data to store.
   * @param $data
   *   The data to store in the cache. Complex data types will be automatically
   *   serialized before insertion.
   *   Strings will be stored as plain text and not serialized.
   * @param $expire
   *   One of the following values:
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
   * @param $cid
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
   * @param $prefix
   *   A wildcard prefix.
   */
  function deletePrefix($prefix);

  /**
   * Flushes all cache items in a bin.
   */
  function flush();

  /**
   * Expires temporary items from the cache.
   */
  function expire();

  /**
   * Performs garbage collection on a cache bin.
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
  function getMultiple(&$cids) {
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

/**
 * Defines a default cache implementation.
 *
 * This is Backdrop's default cache implementation. It uses the database to store
 * cached data. Each cache bin corresponds to a database table by the same name.
 */
class BackdropDatabaseCache implements BackdropCacheInterface {
  protected $bin;

  /**
   * Constructs a new BackdropDatabaseCache object.
   */
  function __construct($bin) {
    // All cache tables should be prefixed with 'cache_', except for the
    // default 'cache' bin.
    if ($bin != 'cache') {
      $bin = 'cache_' . $bin;
    }
    $this->bin = $bin;
  }

  /**
   * Implements BackdropCacheInterface::get().
   */
  function get($cid) {
    $cids = array($cid);
    $cache = $this->getMultiple($cids);
    return reset($cache);
  }

  /**
   * Implements BackdropCacheInterface::getMultiple().
   */
  function getMultiple(&$cids) {
    try {
      // Garbage collection necessary when enforcing a minimum cache lifetime.
      $this->garbageCollection($this->bin);

      // When serving cached pages, the overhead of using db_select() was found
      // to add around 30% overhead to the request. Since $this->bin is a
      // variable, this means the call to db_query() here uses a concatenated
      // string. This is highly discouraged under any other circumstances, and
      // is used here only due to the performance overhead we would incur
      // otherwise. When serving an uncached page, the overhead of using
      // db_select() is a much smaller proportion of the request.
      $result = db_query('SELECT cid, data, created, expire, serialized FROM {' . db_escape_table($this->bin) . '} WHERE cid IN (:cids)', array(':cids' => $cids));
      $cache = array();
      foreach ($result as $item) {
        $item = $this->prepareItem($item);
        if ($item) {
          $cache[$item->cid] = $item;
        }
      }
      $cids = array_diff($cids, array_keys($cache));
      return $cache;
    }
    catch (Exception $e) {
      // If the database is never going to be available, cache requests should
      // return FALSE in order to allow exception handling to occur.
      return array();
    }
  }

  /**
   * Prepares a cached item.
   *
   * Checks that items are either permanent or did not expire, and unserializes
   * data as appropriate.
   *
   * @param $cache
   *   An item loaded from BackdropCacheInterface::get() or BackdropCacheInterface::getMultiple().
   *
   * @return
   *   The item with data unserialized as appropriate or FALSE if there is no
   *   valid item to load.
   */
  protected function prepareItem($cache) {
    global $user;

    if (!isset($cache->data)) {
      return FALSE;
    }
    // If the cached data is temporary and subject to a per-user minimum
    // lifetime, compare the cache entry timestamp with the user session
    // cache_expiration timestamp. If the cache entry is too old, ignore it.
    if ($cache->expire != CACHE_PERMANENT && config_get('system.performance', 'cache_lifetime') && isset($_SESSION['cache_expiration'][$this->bin]) && $_SESSION['cache_expiration'][$this->bin] > $cache->created) {
      // Ignore cache data that is too old and thus not valid for this user.
      return FALSE;
    }

    // If the data is permanent or not subject to a minimum cache lifetime,
    // unserialize and return the cached data.
    if ($cache->serialized) {
      $cache->data = unserialize($cache->data);
    }

    return $cache;
  }

  /**
   * Implements BackdropCacheInterface::set().
   */
  function set($cid, $data, $expire = CACHE_PERMANENT) {
    $fields = array(
      'serialized' => 0,
      'created' => REQUEST_TIME,
      'expire' => $expire,
    );
    if (!is_string($data)) {
      $fields['data'] = serialize($data);
      $fields['serialized'] = 1;
    }
    else {
      $fields['data'] = $data;
      $fields['serialized'] = 0;
    }

    try {
      db_merge($this->bin)
        ->key(array('cid' => $cid))
        ->fields($fields)
        ->execute();
    }
    catch (Exception $e) {
      // The database may not be available, so we'll ignore these calls.
    }
  }

  /**
   * Implements BackdropCacheInterface::delete().
   */
  function delete($cid) {
    db_delete($this->bin)
      ->condition('cid', $cid)
      ->execute();
  }

  /**
   * Implements BackdropCacheInterface::deleteMultiple().
   */
  function deleteMultiple(Array $cids) {
    // Delete in chunks when a large array is passed.
    do {
      db_delete($this->bin)
        ->condition('cid', array_splice($cids, 0, 1000), 'IN')
        ->execute();
    }
    while (count($cids));
  }

  /**
   * Implements BackdropCacheInterface::deletePrefix().
   */
  function deletePrefix($prefix) {
    db_delete($this->bin)
      ->condition('cid', db_like($prefix) . '%', 'LIKE')
      ->execute();
  }

  /**
   * Implements BackdropCacheInterface::flush().
   */
  function flush() {
    db_truncate($this->bin)->execute();
  }

  /**
   * Implements BackdropCacheInterface::expire().
   */
  function expire() {
    if (config_get('system.performance', 'cache_lifetime')) {
      // We store the time in the current user's session. We then simulate
      // that the cache was flushed for this user by not returning cached
      // data that was cached before the timestamp.
      $_SESSION['cache_expiration'][$this->bin] = REQUEST_TIME;

      $cache_flush = variable_get('cache_flush_' . $this->bin, 0);
      if ($cache_flush == 0) {
        // This is the first request to clear the cache; start a timer.
        variable_set('cache_flush_' . $this->bin, REQUEST_TIME);
      }
      elseif (REQUEST_TIME > ($cache_flush + config_get('system.performance', 'cache_lifetime'))) {
        // Clear the cache for everyone; cache_lifetime seconds have passed
        // since the first request to clear the cache.
        db_delete($this->bin)
          ->condition('expire', CACHE_PERMANENT, '<>')
          ->condition('expire', REQUEST_TIME, '<')
          ->execute();
        variable_set('cache_flush_' . $this->bin, 0);
      }
    }
    else {
      // No minimum cache lifetime; flush all temporary cache entries now.
      db_delete($this->bin)
        ->condition('expire', CACHE_PERMANENT, '<>')
        ->condition('expire', REQUEST_TIME, '<')
        ->execute();
    }
  }

  /**
   * Implements BackdropCacheInterface::garbageCollection().
   */
  function garbageCollection() {
    $cache_lifetime = config_get('system.performance', 'cache_lifetime');

    // Clean-up the per-user cache expiration session data, so that the session
    // handler can properly clean-up the session data for anonymous users.
    if (isset($_SESSION['cache_expiration'])) {
      $expire = REQUEST_TIME - $cache_lifetime;
      foreach ($_SESSION['cache_expiration'] as $bin => $timestamp) {
        if ($timestamp < $expire) {
          unset($_SESSION['cache_expiration'][$bin]);
        }
      }
      if (!$_SESSION['cache_expiration']) {
        unset($_SESSION['cache_expiration']);
      }
    }

    // Garbage collection of temporary items is only necessary when enforcing
    // a minimum cache lifetime.
    if (!$cache_lifetime) {
      return;
    }
    // When cache lifetime is in force, avoid running garbage collection too
    // often since this will remove temporary cache items indiscriminately.
    $cache_flush = variable_get('cache_flush_' . $this->bin, 0);
    if ($cache_flush && ($cache_flush + $cache_lifetime <= REQUEST_TIME)) {
      // Reset the variable immediately to prevent a meltdown in heavy load
      // situations.
      variable_set('cache_flush_' . $this->bin, 0);
      // Time to flush old cache data
      db_delete($this->bin)
        ->condition('expire', CACHE_PERMANENT, '<>')
        ->condition('expire', $cache_flush, '<=')
        ->execute();
    }
  }

  /**
   * Implements BackdropCacheInterface::isEmpty().
   */
  function isEmpty() {
    $this->garbageCollection();
    $query = db_select($this->bin);
    $query->addExpression('1');
    $result = $query->range(0, 1)
      ->execute()
      ->fetchField();
    return empty($result);
  }
}
