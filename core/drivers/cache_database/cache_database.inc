<?php

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
