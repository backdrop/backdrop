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
      $class = settings_get('cache_default_class', 'BackdropTaggableCache');
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
 *   (optional) Controls the maximum lifetime of this cache entry. Note that
 *   caches might be subject to clearing at any time, so this setting does not
 *   guarantee a minimum lifetime. With this in mind, the cache should not be
 *   used for data that must be kept during a cache clear, like sessions.
 *
 *   Use one of the following values:
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
   *   (optional) Controls the maximum lifetime of this cache entry. Note that
   *   caches might be subject to clearing at any time, so this setting does not
   *   guarantee a minimum lifetime. With this in mind, the cache should not be
   *   used for data that must be kept during a cache clear, like sessions.
   *
   *   Use one of the following values:
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

/**
 * Defines a metadata aware cache backend.
 *
 * @ingroup cache
 */
interface BackdropTaggableCacheInterface  extends BackdropCacheInterface {

  /**
   * Stores data in the persistent cache.
   *
   * @param string $cid
   *   The cache ID of the data to store.
   * @param mixed $data
   *   The data to store in the cache. Complex data types will be
   *   automatically serialized before insertion. Strings will be stored as
   *   plain text and not serialized. Some storage engines only allow
   *   objects up to a maximum of 1MB in size to be stored by default. When
   *   caching large arrays or similar, take care to ensure $data does not
   *   exceed this size.
   * @param int $expire
   *   (optional) One of the following values:
   *     - CACHE_PERMANENT: Indicates that the item should never be removed
   *     unless explicitly told to using cache_clear_all() with a cache ID.
   *     - CACHE_TEMPORARY: Indicates that the item should be removed at the
   *     next general cache wipe.
   *     - A Unix timestamp: Indicates that the item should be kept at least
   *     until the given time, after which it behaves like CACHE_TEMPORARY.
   * @param array $tags
   *   (optional) An array of cache tags, defaults to no tags.
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT, $tags = array());

  /**
   * Returns data from the persistent cache.
   *
   * Data may be stored as either plain text or as serialized data.
   * cache_get() will automatically return unserialized objects and arrays.
   *
   * @param string $cid
   *   The cache ID of the data to retrieve.
   * @param bool $allow_invalid
   *   (optional) If TRUE, a cache item may be returned even if it is
   *   expired or has been invalidated. Such items may sometimes be
   *   preferred, if the alternative is recalculating the value stored in the
   *   cache, especially if another concurrent request is already
   *   recalculating the same value. The "valid" property of the returned
   *   object indicates whether the item is valid or not. Defaults to FALSE.
   *
   * @return object|bool
   *   The cache or FALSE on failure.
   */
  public function get($cid, $allow_invalid = FALSE);

  /**
   * Returns data from the persistent cache when given an array of cache IDs.
   *
   * @param array $cids
   *   An array of cache IDs for the data to retrieve. This is passed by
   *   reference, and will have the IDs successfully returned from cache
   *   removed.
   * @param bool $allow_invalid
   *   (optional) If TRUE, cache items may be returned even if they have
   *   expired or been invalidated. Such items may sometimes be preferred,
   *   if the alternative is recalculating the value stored in the cache,
   *   especially if another concurrent thread is already recalculating the
   *   same value. The "valid" property of the returned objects indicates
   *   whether the items are valid or not. Defaults to FALSE.
   *
   * @return array
   *   An array of the items successfully returned from cache indexed by cid.
   */
  public function getMultiple(&$cids, $allow_invalid = FALSE);
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

    // Bootstrap the database if it is not yet available.
    if (!function_exists('db_query') || backdrop_get_bootstrap_phase() < BACKDROP_BOOTSTRAP_DATABASE) {
      backdrop_bootstrap(BACKDROP_BOOTSTRAP_DATABASE, FALSE);
    }
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
  function getMultiple(array &$cids) {
    try {
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
   * @return stdClass|FALSE
   *   The item with data unserialized as appropriate or FALSE if there is no
   *   valid item to load.
   */
  protected function prepareItem($cache) {
    if (!isset($cache->data)) {
      return FALSE;
    }

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
  function deleteMultiple(array $cids) {
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
   * Implements BackdropCacheInterface::garbageCollection().
   */
  function garbageCollection() {
    db_delete($this->bin)
      ->condition('expire', CACHE_PERMANENT, '<>')
      ->condition('expire', REQUEST_TIME, '<')
      ->execute();
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

class BackdropTaggableCache implements BackdropTaggableCacheInterface {

  /**
   * The cache bin.
   *
   * @var string
   */
  protected $bin;

  /**
   * The cache backend.
   *
   * @var \BackdropCacheInterface
   */
  protected $backend;

  /**
   * The cache bin specific configuration.
   *
   * @var array
   */
  protected $configuration;

  /**
   * Constructs a Backdrop8CacheBackend object.
   *
   * @param string $bin
   *   The cache bin for which the object is created.
   */
  public function __construct($bin) {
    $config = config('system.core.settings');

    $this->bin = $bin;

    $class = settings_get('taggable_cache_default_class');
    if (empty($class)) {
      $class = settings_get('cache_default_class', 'BackdropDatabaseCache');
    }

    $this->backend = new $class($bin);

    $configuration = $config->get('cache_options');
    if (isset($configuration[$bin])) {
      $this->configuration = $configuration[$bin];
    }
    else {
      $this->configuration = array(
        'ttl' => CACHE_PERMANENT,
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function get($cid, $allow_invalid = FALSE) {
    $cids = array($cid);
    $cache = $this->getMultiple($cids, $allow_invalid);
    return reset($cache);
  }

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids, $allow_invalid = FALSE) {
    $cache = array();
    $cids_map = array_flip($cids);

    foreach ($this->backend->getMultiple($cids) as $cid => $item) {
      // This should never happen.
      if (!isset($cids_map[$cid])) {
        continue;
      }

      $data = $item->data;

      $expire = isset($data->cache_expire) ? (int) $data->cache_expire : (int) $item->expire;

      // Check expire time.
      $item->valid = $expire === CACHE_PERMANENT || $expire === CACHE_TEMPORARY || $expire >= REQUEST_TIME;

      // Is this packed data?
      if ($data instanceof stdClass && isset($data->cache_tags)) {
        // Check if the cache tags are valid.
        if (!$this->checksumValid($data->cache_checksum, $data->cache_tags)) {
          $item->valid = FALSE;
        }

        $item->data = $data->cache_data;
      }

      if (!$allow_invalid && !$item->valid) {
        continue;
      }

      $cache[$cid] = $item;
      unset($cids_map[$cid]);
    }

    // Re-calculate the cids property.
    $cids = array_keys($cids_map);

    return $cache;
  }

  /**
   * {@inheritdoc}
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT, $tags = array()) {
    // Allow to override the TTL for a whole bin.
    if (isset($this->configuration['ttl'])) {
      if ($this->configuration['ttl'] == CACHE_MAX_AGE_PERMANENT) {
        // Convert the TTL magic value to the equivilent expire magic value.
        $expire = CACHE_PERMANENT;
      }
      elseif ($this->configuration['ttl'] == CACHE_PERMANENT || $this->configuration['ttl'] == CACHE_TEMPORARY) {
        // Also accept passing the expire magic values directly through.
        $expire = $this->configuration['ttl'];
      }
      else {
        // Convert the TTL to an expiration timestamp.
        $expire = REQUEST_TIME + $this->configuration['ttl'];
      }
    }

    // Allow to set global tags per bin.
    if (isset($this->configuration['tags'])) {
      $tags = backdrop_merge_cache_tags($tags, $this->configuration['tags']);
    }

    // Special case cache_page.
    if ($this->bin == 'cache_page') {
      $page_cache_tags = &backdrop_static('system_emit_cache_tags', array());
      $page_cache_max_age = &backdrop_static('system_emit_cache_max_age', CACHE_MAX_AGE_PERMANENT);

      if (!empty($page_cache_tags)) {
        $tags = backdrop_merge_cache_tags($tags, $page_cache_tags);
      }

      $expire = $this->mergeExpireWithMaxAge($expire, $page_cache_max_age);
    }

    // Render arrays.
    if (is_array($data)
         && isset($data['#attached'])
         && (isset($data['#attached']['backdrop_add_cache_tags']) || isset($data['#attached']['backdrop_set_cache_max_age']))) {
      $cacheable_metadata = backdrop_get_cacheable_metadata_from_render_array($data);

      if (!empty($cacheable_metadata['tags'])) {
        $tags = backdrop_merge_cache_tags($tags, $cacheable_metadata['tags']);
        // Collapse the attached tags into a single attachment before saving.
        $data['#attached']['backdrop_add_cache_tags'] = array(array(0 => $cacheable_metadata['tags']));
      }

      $expire = $this->mergeExpireWithMaxAge($expire, $cacheable_metadata['max-age']);
      if ($cacheable_metadata['max-age'] != CACHE_MAX_AGE_PERMANENT) {
        // Collapse the attached max-age into a single attachment before saving.
        $data['#attached']['backdrop_set_cache_max_age'] = array(array(0 => $cacheable_metadata['max-age']));
      }
    }

    // No tags, present, just continue normally.
    if (empty($tags)) {
      $this->backend->set($cid, $data, $expire);
      return;
    }

    // Does the backend support tags natively?
    if ($this->backend instanceof TaggableBackdropCacheInterface) {
      $this->backend->set($cid, $data, $expire, $tags);
    }
    else {
      $checksum = $this->getCurrentChecksum($tags);

      $data = (object) array(
        'cache_tags' => $tags,
        'cache_checksum' => $checksum,
        'cache_expire' => $expire,
        'cache_data' => $data,
      );

      $this->backend->set($cid, $data, $expire);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function clear($cid = NULL, $wildcard = FALSE) {
    $this->backend->clear($cid, $wildcard);
  }

  /**
   * {@inheritdoc}
   */
  public function isEmpty() {
    return $this->backend->isEmpty();
  }

  /**
   * Merges an max-age value with an expire timestamp.
   *
   * @param int $expire
   *   A unix timestamp when this item will expire or one of the CACHE_*
   *   constants.
   * @param int $max_age
   *   A max-age ttl value like an integer or CACHE_MAX_AGE_PERMANENT.
   *
   * @return int
   *   A unix timestamp when this item will expire or one of the CACHE_*
   *   constants.
   */
  protected function mergeExpireWithMaxAge($expire, $max_age) {
    // The difference between $expire and $max-age is that while $expire is
    // a unix timestamp, $max_age is a relative TTL. So, when they interact,
    // we have to be careful to not compare apples and oranges.

    // Do not mess with temporary items.
    if ($expire == CACHE_TEMPORARY) {
      return $expire;
    }

    // In case $max_age is PERMANENT return $expire as is.
    if ($max_age === CACHE_MAX_AGE_PERMANENT) {
      return $expire;
    }

    // If $expire is permanent return the numeric ttl.
    if ($expire == CACHE_PERMANENT) {
      return REQUEST_TIME + $max_age;
    }

    // In all other cases return the minimum of ttl($expire) and $max_age.
    return REQUEST_TIME + min(REQUEST_TIME - $expire, $max_age);
  }

  /**
   * Returns the sum total of validations for a given set of tags.
   *
   * Called by a backend when storing a cache item.
   *
   * @param string[] $tags
   *   Array of cache tags.
   *
   * @return string
   *   Cache tag invalidations checksum.
   */
  protected function getCurrentChecksum(array $tags) {
    if (!$this->ensureInstalled()) {
      // When the module has not been installed yet, don't calculate a checksum.
      return 0;
    }
    return system_cache_tags_get_current_checksum($tags);
  }

  /**
   * Returns whether the checksum is valid for the given cache tags.
   *
   * Used when retrieving a cache item in a cache backend, to verify that no
   * cache tag based invalidation happened.
   *
   * @param int $checksum
   *   The checksum that was stored together with the cache item.
   * @param string[] $tags
   *   The cache tags that were stored together with the cache item.
   *
   * @return bool
   *   FALSE if cache tag invalidations happened for the passed in tags since
   *   the cache item was stored, TRUE otherwise.
   */
  protected function checksumValid($checksum, array $tags) {
    if (!$this->ensureInstalled()) {
      // When the module has not been installed yet, assume all checksums are
      // valid.
      return TRUE;
    }

    return system_cache_tags_is_valid($checksum, $tags);
  }

  /**
   * Ensure that the database table for cache tags has been created.
   *
   * @return bool TRUE if the table {invalidated_cache_tags} exists , otherwise FALSE.
   */
  public function ensureInstalled() {
    static $is_installed = NULL;

    if (is_null($is_installed)) {
      if (db_table_exists('invalidated_cache_tags')) {
        $is_installed = TRUE;
      }
      else {
        $is_installed = FALSE;
      }
    }

    return $is_installed;
  }

  /**
   * {@inheritdoc}
   */
  public function delete($cid) {
    $this->backend->delete($cid);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteMultiple($cids) {
    $this->backend->deleteMultiple($cids);
  }

  /**
   * {@inheritdoc}
   */
  public function deletePrefix($prefix) {
    $this->backend->deletePrefix($prefix);
  }

  /**
   * {@inheritdoc}
   */
  public function flush() {
    $this->backend->flush();
  }

  /**
   * {@inheritdoc}
   */
  public function garbageCollection() {
    $this->backend->garbageCollection();
  }
}

/**
 * Provices a way to collect attachments added during the rendering process.
 *
 * Attachments added via backdrop_add_js() / backdrop_add_css() or that are early
 * rendered as part of a template have been lost in the past and that made
 * caching of anything that is not the whole page difficult.
 * The AttachmentsCollector solves this problem by routing all adding of
 * attachments through backdrop_process_attached(), which is then storing the
 * attachments in the registered storage of listeners - if any.
 * To create an attachments collector the following code shows an example:
 *
 * @code
 * // Register a listener.
 * $attachments_collector = new BackdropAttachmentsCollector();
 * // Render the render array.
 * $rendered = backdrop_render($build);
 * // Store the attachments.
 * $attachments = $attachments_collector->getAttachments();
 * // Unregister the listener.
 * unset($attachments_collector);
 * // Now store the attachments in the render array to cache.
 * $build_to_cache['#markup'] = $rendered;
 * $build_to_cache['#attached'] = $attachments;
 * @endcode
 */
class BackdropAttachmentsCollector {

  /**
   * The collected attachments.
   *
   * @var array
   */
  protected $attachments = array();

  /**
   * A count for how many cache IDs this collector should be used.
   *
   * @var int
   */
  public $count = 0;

  /**
   * The stored previous collector.
   *
   * @var \BackdropAttachmentsCollector
   */
  public $previousCollector = NULL;

  /**
   * Constructs a BackdropAttachmentsCollector object.
   *
   * This will register a listener that listens for attachments created via:
   * - backdrop_add_js()
   * - backdrop_add_css()
   * - backdrop_add_library()
   * - backdrop_add_html_head()
   * - backdrop_add_http_header()
   * and everything that is processed with backdrop_process_attached().
   */
  public function __construct() {
    $key = spl_object_hash($this);

    $render_listeners = &backdrop_static('backdrop_render:render_listeners', array());
    $render_listeners[$key] = array();

    $this->attachments = &$render_listeners[$key];
  }

  /**
   * Returns the collected attachments.
   *
   * The returned values can be put into the '#attached' property of an
   * render array.
   *
   * @code
   * $build['#attached'] = $attachments_collector->getAttachments();
   *
   * // The structure of the render array looks like this:
   * $build['#attached'] = array(
   *   'js' => array(
   *      array('data' => 'somefile.js'),
   *      array('data' => 'anotherfile.js'),
   *   ),
   * );
   * @endcode
   *
   * @return array
   *   Returns the collected attachments as an #attached array in the form of
   *   key:value.
   */
  public function getAttachments() {
    return $this->attachments;
  }

  /**
   * Reset the attachments in the collector.
   */
  public function reset() {
    $this->attachments = array();
  }

  /**
   * Destructs the BackdropAttachmentsCollector object.
   *
   * This will unregister the listener that listens for attachments.
   */
  public function __destruct() {
    $key = spl_object_hash($this);

    $render_listeners = &backdrop_static('backdrop_render:render_listeners', array());
    unset($render_listeners[$key]);
  }

  /**
   * Implements __sleep().
   *
   * This disallows serializing the attachments collector as it relies on
   * state.
   */
  public function __sleep() {
    throw new Exception('An attachments collector cannot be serialized.');
  }
}

/**
 * Special cache backend that tracks attachments.
 *
 * @ingroup cache
 */
class BackdropAttachmentsCollectorCache extends BackdropTaggableCache {

  /**
   * An array of attachment collectors, keyed by cache id.
   *
   * @var array
   */
  protected $attachmentsCollectors;

  /**
   * The currently active attachments collector for this bin.
   *
   * @var array
   */
  protected $currentAttachmentsCollector;

  /**
   * Whether this CacheAttachmentsCollector is in a valid state or not.
   * If not valid, behavior falls back to BackdropTaggableCache behavior.
   *
   * The most common cause of an invalid state is attempting to use this
   * class to handle early page cache.
   *
   * @var boolean
   */
  protected $isValid;

  /**
   * {@inheritdoc}
   */
  public function __construct($bin) {
    $this->isValid = TRUE;
    if (!function_exists('backdrop_process_attached')) {
      // Something is trying to use us during early bootstrap.
      // Fall back to baseline BackdropTaggableCache behavior for safety.
      $this->isValid = FALSE;

      // Additionally, prevent the request from saving to cache, and warn
      // loudly that we are in an unsupported configuration.
      $t = get_t();
      backdrop_set_message($t('BackdropAttachmentsCollectorCache is not designed for early page cache! Please switch cache_page to BackdropTaggableCache in settings.php!'), 'error');
    }
    parent::__construct($bin);
  }

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids, $allow_invalid = FALSE) {
    // The parent will properly populate $cids, so we can rely on it.
    $cache = parent::getMultiple($cids, $allow_invalid);

    if (!$this->isValid) {
      return $cache;
    }

    // Unpack the cached data and process attachments.
    foreach ($cache as $cid => $item) {
      if (is_array($item->data) && isset($item->data['#cache_tags_data'])) {
        backdrop_process_attached($item->data);
        $cache[$cid]->data = $item->data['#cache_tags_data'];
      }
    }

    // In case there are no cids left, return.
    if (empty($cids)) {
      return $cache;
    }

    // We have multiple cids, so we need to reset after each cache set.
    $attachments_collector = new BackdropAttachmentsCollector();
    $attachments_collector->count = count($cids);
    $attachments_collector->previousCollector = $this->currentAttachmentsCollector;
    $this->currentAttachmentsCollector = $attachments_collector;

    foreach ($cids as $cid) {
      $this->attachmentsCollectors[$cid] = $attachments_collector;
    }

    return $cache;
  }

  /**
   * {@inheritdoc}
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT, $tags = array()) {
    // Fall back to regular BackdropTaggableCache if we are not in a valid state.
    if (!$this->isValid) {
      parent::set($cid, $data, $expire, $tags);
      return;
    }

    $attachments = array();

    if (isset($this->attachmentsCollectors[$cid])) {
      $attachments_collector = $this->attachmentsCollectors[$cid];
      $attachments = $attachments_collector->getAttachments();
      unset($this->attachmentsCollectors[$cid]);

      // Reset the attachments for re-use.
      $attachments_collector->reset();

      $attachments_collector->count--;
      if ($attachments_collector->count == 0) {
        $this->currentAttachmentsCollector = $attachments_collector->previousCollector;
      }
    }

    // Create a pseudo render array.
    $data = array(
      '#cache_tags_data' => $data,
      '#attached' => $attachments,
    );

    parent::set($cid, $data, $expire, $tags);
  }

  /**
   * Reset the currently active attachments collector - if any.
   */
  public function resetCurrentAttachmentsCollector() {
    if ($this->currentAttachmentsCollector) {
      $this->currentAttachmentsCollector->reset();
    }
  }

}
