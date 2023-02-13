<?php
/**
 * @file
 * Definition of views_plugin_cache.
 */

/**
 * @defgroup views_cache_plugins Views cache plugins
 * @{
 * @todo.
 *
 * @see hook_views_plugins()
 */

/**
 * The base plugin to handle caching.
 */
class views_plugin_cache extends views_plugin {
  /**
   * Contains all data that should be written/read from cache.
   */
  var $storage = array();

  /**
   * What bin to store data in.
   */
  var $bin = 'views_data';

  /**
   * Initialize the plugin.
   *
   * @param $view
   *   The view object.
   * @param $display
   *   The display handler.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;

    if (is_object($display->handler)) {
      $options = $display->handler->get_option('cache');
      // Overlay incoming options on top of defaults
      $this->unpack_options($this->options, $options);
    }
  }

  /**
   * Return a string to display as the clickable title for the
   * access control.
   */
  function summary_title() {
    return t('Unknown');
  }

  /**
   * Determine the expiration time of the cache type, or NULL if no expire.
   *
   * Plugins must override this to implement expiration.
   *
   * @param $type
   *   The cache type, either 'query', 'result' or 'output'.
   */
   function cache_expire($type) { }

   /**
    * Determine expiration time in the cache table of the cache type
    * or CACHE_PERMANENT if item shouldn't be removed automatically from cache.
    *
    * Plugins must override this to implement expiration in the cache table.
    *
    * @param $type
    *   The cache type, either 'query', 'result' or 'output'.
    */
  function cache_set_expire($type) {
    return CACHE_PERMANENT;
  }


  /**
   * Save data to the cache.
   *
   * A plugin should override this to provide specialized caching behavior.
   */
  function cache_set($type) {
    switch ($type) {
      case 'query':
        // Not supported currently, but this is certainly where we'd put it.
        break;
      case 'results':
        $data = array(
          'result' => $this->view->result,
          'total_rows' => isset($this->view->total_rows) ? $this->view->total_rows : 0,
          'current_page' => $this->view->get_current_page(),
        );
        cache($this->bin)->set($this->get_results_key(), $data, $this->cache_set_expire($type));
        break;
      case 'output':
        $this->gather_headers();
        $this->storage['output'] = $this->view->display_handler->output;
        cache($this->bin)->set($this->get_output_key(), $this->storage, $this->cache_set_expire($type));
        break;
    }
  }


  /**
   * Retrieve data from the cache.
   *
   * A plugin should override this to provide specialized caching behavior.
   */
  function cache_get($type) {
    $cutoff = $this->cache_expire($type);
    switch ($type) {
      case 'query':
        // Not supported currently, but this is certainly where we'd put it.
        return FALSE;
      case 'results':
        // Values to set: $view->result, $view->total_rows, $view->execute_time,
        // $view->current_page.
        if ($cache = cache($this->bin)->get($this->get_results_key())) {
          if (!$cutoff || $cache->created > $cutoff) {
            $this->view->result = $cache->data['result'];
            $this->view->total_rows = $cache->data['total_rows'];
            $this->view->set_current_page($cache->data['current_page']);
            $this->view->execute_time = 0;
            return TRUE;
          }
        }
        return FALSE;
      case 'output':
        if ($cache = cache($this->bin)->get($this->get_output_key())) {
          if (!$cutoff || $cache->created > $cutoff) {
            $this->storage = $cache->data;
            $this->view->display_handler->output = $cache->data['output'];
            $this->restore_headers();
            return TRUE;
          }
        }
        return FALSE;
    }
  }

  /**
   * Clear out cached data for a view.
   *
   * We're just going to nuke anything related to the view, regardless of display,
   * to be sure that we catch everything. Maybe that's a bad idea.
   */
  function cache_flush() {
    cache($this->bin)->deletePrefix($this->view->name . ':');
  }

  /**
   * Post process any rendered data.
   *
   * This can be valuable to be able to cache a view and still have some level of
   * dynamic output. In an ideal world, the actual output will include HTML
   * comment based tokens, and then the post process can replace those tokens.
   *
   * Example usage. If it is known that the view is a node view and that the
   * primary field will be a nid, you can do something like this:
   *
   * <!--post-FIELD-NID-->
   *
   * And then in the post render, create an array with the text that should
   * go there:
   *
   * strtr($output, array('<!--post-FIELD-1-->', 'output for FIELD of nid 1');
   *
   * All of the cached result data will be available in $view->result, as well,
   * so all ids used in the query should be discoverable.
   */
  function post_render(&$output) { }

  /**
   * Start caching javascript, css and other out of band info.
   *
   * This takes a snapshot of the current system state so that we don't
   * duplicate it. Later on, when gather_headers() is run, this information
   * will be removed so that we don't hold onto it.
   */
  function cache_start() {
    $this->storage['head'] = backdrop_add_html_head();
    $this->storage['css'] = backdrop_add_css();
    $this->storage['js'] = backdrop_add_js();
    $this->storage['headers'] = backdrop_get_http_header();
  }

  /**
   * Gather out of band data, compare it to what we started with and store the difference.
   */
  function gather_headers() {
    // Check if the advanced mapping function of D 7.23 is available.
    $array_mapping_func = function_exists('backdrop_array_diff_assoc_recursive') ? 'backdrop_array_diff_assoc_recursive' : 'array_diff_assoc';

    // Get difference for head.
    $this->storage['head'] = $array_mapping_func(backdrop_add_html_head(), $this->storage['head']);

    // Slightly less simple for CSS:
    $css = backdrop_add_css();
    $css_start = isset($this->storage['css']) ? $this->storage['css'] : array();
    $this->storage['css'] = $this->assetDiff($css, $css_start, $array_mapping_func);

    // Get javascript after/before views renders.
    $js = backdrop_add_js();
    $js_start = isset($this->storage['js']) ? $this->storage['js'] : array();
    // If there are any differences between the old and the new javascript then
    // store them to be added later.
    $this->storage['js'] = $this->assetDiff($js, $js_start, $array_mapping_func);

    // Special case the settings key and get the difference of the data.
    $settings = isset($js['settings']['data']) ? $js['settings']['data'] : array();
    $settings_start = isset($js_start['settings']['data']) ? $js_start['settings']['data'] : array();
    $this->storage['js']['settings'] = $array_mapping_func($settings, $settings_start);

    // Get difference of HTTP headers.
    $this->storage['headers'] = $array_mapping_func(backdrop_get_http_header(), $this->storage['headers']);
  }

  /**
   * Computes the differences between two JS/CSS asset arrays.
   *
   * @param array $assets
   *   The current asset array.
   * @param array $start_assets
   *   The original asset array.
   * @param string $diff_function
   *   The function that should be used for computing the diff.
   *
   * @return array
   *   A CSS or JS asset array that contains all entries that are new/different
   *   in $assets.
   */
  protected function assetDiff(array $assets, array $start_assets, $diff_function) {
    $diff = $diff_function($assets, $start_assets);

    // Clean the resulting array up, since backdrop_array_diff_assoc_recursive()
    // can leave half-populated arrays behind.
    foreach ($diff as $key => $entry) {
      // If only the weight was different, we can remove this entry.
      if (count($entry) == 1 && isset($entry['weight'])) {
        unset($diff[$key]);
      }
      // If there are other differences, we override with the latest entry.
      elseif ($entry != $assets[$key]) {
        $diff[$key] = $assets[$key];
      }
    }
    return $diff;
  }

  /**
   * Restore out of band data saved to cache. Copied from Panels.
   */
  function restore_headers() {
    if (!empty($this->storage['head'])) {
      foreach ($this->storage['head'] as $key => $data) {
        backdrop_add_html_head($this->storage['head']);
      }
    }
    if (!empty($this->storage['css'])) {
      foreach ($this->storage['css'] as $args) {
        backdrop_add_css($args['data'], $args);
      }
    }
    if (!empty($this->storage['js'])) {
      foreach ($this->storage['js'] as $key => $args) {
        if ($key !== 'settings') {
          backdrop_add_js($args['data'], $args);
        }
        else {
          foreach ($args as $setting) {
            backdrop_add_js($setting, 'setting');
          }
        }
      }
    }
    if (!empty($this->storage['headers'])) {
      foreach ($this->storage['headers'] as $name => $value) {
        backdrop_add_http_header($name, $value);
      }
    }
  }

  public function get_results_key() {
    if (!isset($this->_results_key)) {
      $key_data = array();
      foreach (array('exposed_info', 'page', 'sort', 'order', 'items_per_page', 'offset') as $key) {
        if (isset($_GET[$key])) {
          $key_data[$key] = $_GET[$key];
        }
      }

      $this->_results_key = $this->view->name . ':' . $this->display->id . ':results:' . $this->get_cache_key($key_data);
    }

    return $this->_results_key;
  }

  public function get_output_key() {
    if (!isset($this->_output_key)) {
      $key_data = array(
        'result' => $this->view->result,
        'theme' => $GLOBALS['theme'],
      );
      $this->_output_key = $this->view->name . ':' . $this->display->id . ':output:' . $this->get_cache_key($key_data);
    }

    return $this->_output_key;
  }

  /**
   * Returns cache key.
   *
   * @param array $key_data
   *   Additional data for cache segmentation and/or overrides for default
   *   segmentation.
   *
   * @return string
   */
  public function get_cache_key($key_data = array()) {
    global $user;

    $key_data += array(
      'roles' => array_keys($user->roles),
      'super-user' => $user->uid == 1, // Special caching for super user.
      'language' => $GLOBALS['language']->langcode,
      'language_content' => $GLOBALS['language_content']->langcode,
      'base_url' => $GLOBALS['base_url'],
      'display_options' => $this->display->display_options,
    );

    if (empty($key_data['build_info'])) {
      $build_info = $this->view->build_info;
      foreach (array('query', 'count_query') as $index) {
        // If the default query back-end is used generate SQL query strings from
        // the query objects.
        if ($build_info[$index] instanceof SelectQueryInterface) {
          $query = $build_info[$index];

          // If the query was not yet prepared, work on a clone and run
          // preExecute().
          if (!$query->isPrepared()) {
            $query = clone $build_info[$index];
            $query->preExecute();
          }

          $key_data['build_info'][$index] = array(
            'sql' => (string) $query,
            'arguments' => $query->getArguments(),
          );
        }
      }
    }
    return md5(serialize($key_data));
  }
}

/**
 * @}
 */
