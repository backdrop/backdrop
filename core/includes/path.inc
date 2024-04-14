<?php
/**
 * @file
 * Functions to handle paths in Backdrop, including URL aliasing.
 *
 * These functions are not loaded for cached pages, but modules that need
 * to use them in hook_boot() or hook exit() can make them available, by
 * executing "backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);".
 */

/**
 * Initialize the $_GET['q'] variable to the proper normal path.
 */
function backdrop_path_initialize() {
  // Ensure $_GET['q'] is set before calling backdrop_get_normal_path(), to
  // support path caching with hook_url_inbound_alter().
  if (empty($_GET['q'])) {
    $_GET['q'] = config_get('system.core', 'site_frontpage');
  }
  $_GET['q'] = backdrop_get_normal_path($_GET['q']);
}

/**
 * Given an alias, return its Backdrop system URL if one exists. Given a Backdrop
 * system URL return one of its aliases if such a one exists. Otherwise,
 * return FALSE.
 *
 * @param $action
 *   One of the following values:
 *   - wipe: delete the alias cache.
 *   - alias: return an alias for a given Backdrop system path (if one exists).
 *   - source: return the Backdrop system path for a URL alias (if one exists).
 * @param $path
 *   The path to investigate for corresponding aliases or system URLs.
 * @param $langcode
 *   Optional language code to search the path with. Defaults to the page language.
 *   If there's no path defined for that language it will search paths without
 *   language.
 *
 * @return
 *   Either a Backdrop system path, an aliased path, or FALSE if no path was
 *   found.
 */
function backdrop_lookup_path($action, $path = '', $langcode = NULL) {
  global $language_url;
  // Use the advanced backdrop_static() pattern, since this is called very often.
  static $backdrop_static_fast;
  if (!isset($backdrop_static_fast)) {
    $backdrop_static_fast['cache'] = &backdrop_static(__FUNCTION__);
  }
  $cache = &$backdrop_static_fast['cache'];

  if (!isset($cache)) {
    $cache = array(
      'map' => array(),
      'no_source' => array(),
      'allowlist' => NULL,
      'system_paths' => array(),
      'no_aliases' => array(),
      'first_call' => TRUE,
    );
  }

  // If no language is explicitly specified we default to the current URL
  // language. If we used a language different from the one conveyed by the
  // requested URL, we might end up being unable to check if there is a URL
  // alias matching the system path.
  $langcode = $langcode ? $langcode : $language_url->langcode;

  if ($action == 'wipe') {
    $cache = array();
  }
  elseif ($path != '') {
    if ($action == 'alias') {
      // During the first call to backdrop_lookup_path() per language, load the
      // expected system paths for the page from cache.
      if (!empty($cache['first_call'])) {
        $cache['first_call'] = FALSE;

        $cache['map'][$langcode] = array();
        // Load system paths from cache.
        $cid = current_path();
        if ($cached = cache('path')->get($cid)) {
          $cache['system_paths'] = $cached->data;
          // Now fetch the aliases corresponding to these system paths.
          $args = array(
            ':system' => $cache['system_paths'],
            ':langcode' => $langcode,
            ':language_none' => LANGUAGE_NONE,
          );
          // Always get the language-specific alias before the language-neutral
          // one. For example 'de' is less than 'und' so the order needs to be
          // ASC, while 'xx-lolspeak' is more than 'und' so the order needs to
          // be DESC. We also order by pid ASC so that fetchAllKeyed() returns
          // the most recently created alias for each source. Subsequent queries
          // using fetchField() must use pid DESC to have the same effect.
          // For performance reasons, the query builder is not used here.
          if ($langcode == LANGUAGE_NONE) {
            // Prevent PDO from complaining about a token the query doesn't use.
            unset($args[':langcode']);
            $result = db_query('SELECT source, alias FROM {url_alias} WHERE source IN (:system) AND langcode = :language_none ORDER BY pid ASC', $args);
          }
          elseif ($langcode < LANGUAGE_NONE) {
            $result = db_query('SELECT source, alias FROM {url_alias} WHERE source IN (:system) AND langcode IN (:langcode, :language_none) ORDER BY langcode ASC, pid ASC', $args);
          }
          else {
            $result = db_query('SELECT source, alias FROM {url_alias} WHERE source IN (:system) AND langcode IN (:langcode, :language_none) ORDER BY langcode DESC, pid ASC', $args);
          }
          $cache['map'][$langcode] = $result->fetchAllKeyed();
          // Keep a record of paths with no alias to avoid querying twice.
          $cache['no_aliases'][$langcode] = array_flip(array_diff_key($cache['system_paths'], array_keys($cache['map'][$langcode])));
        }
      }
      // If the alias has already been loaded, return it.
      if (isset($cache['map'][$langcode][$path])) {
        return $cache['map'][$langcode][$path];
      }
      // For system paths which were not cached, query aliases individually.
      elseif (!isset($cache['no_aliases'][$langcode][$path])) {
        $args = array(
          ':source' => $path,
          ':langcode' => $langcode,
          ':language_none' => LANGUAGE_NONE,
        );
        // See the queries above.
        if ($langcode == LANGUAGE_NONE) {
          unset($args[':langcode']);
          $alias = db_query("SELECT alias FROM {url_alias} WHERE source = :source AND langcode = :language_none ORDER BY pid DESC", $args)->fetchField();
        }
        elseif ($langcode > LANGUAGE_NONE) {
          $alias = db_query("SELECT alias FROM {url_alias} WHERE source = :source AND langcode IN (:langcode, :language_none) ORDER BY langcode DESC, pid DESC", $args)->fetchField();
        }
        else {
          $alias = db_query("SELECT alias FROM {url_alias} WHERE source = :source AND langcode IN (:langcode, :language_none) ORDER BY langcode ASC, pid DESC", $args)->fetchField();
        }
        $cache['map'][$langcode][$path] = $alias;
        return $alias;
      }
    }
    // Check $no_source for this $path in case we've already determined that there
    // isn't a path that has this alias
    elseif ($action == 'source' && !isset($cache['no_source'][$langcode][$path])) {
      // Look for the value $path within the cached $map
      if (!isset($cache['map'][$langcode]) || !($source = array_search($path, $cache['map'][$langcode]))) {
        $args = array(
          ':alias' => $path,
          ':langcode' => $langcode,
          ':language_none' => LANGUAGE_NONE,
        );
        // See the queries above.
        if ($langcode == LANGUAGE_NONE) {
          unset($args[':langcode']);
          $result = db_query("SELECT source FROM {url_alias} WHERE alias = :alias AND langcode = :language_none ORDER BY pid DESC", $args);
        }
        elseif ($langcode > LANGUAGE_NONE) {
          $result = db_query("SELECT source FROM {url_alias} WHERE alias = :alias AND langcode IN (:langcode, :language_none) ORDER BY langcode DESC, pid DESC", $args);
        }
        else {
          $result = db_query("SELECT source FROM {url_alias} WHERE alias = :alias AND langcode IN (:langcode, :language_none) ORDER BY langcode ASC, pid DESC", $args);
        }
        if ($source = $result->fetchField()) {
          $cache['map'][$langcode][$source] = $path;
        }
        else {
          // We can't record anything into $map because we do not have a valid
          // index and there is no need because we have not learned anything
          // about any Backdrop path. Thus cache to $no_source.
          $cache['no_source'][$langcode][$path] = TRUE;
        }
      }
      return $source;
    }
  }

  return FALSE;
}

/**
 * Cache system paths for a page.
 *
 * Cache an array of the system paths available on each page. We assume
 * that aliases will be needed for the majority of these paths during
 * subsequent requests, and load them in a single query during
 * backdrop_lookup_path().
 */
function backdrop_cache_system_paths() {
  // Check if the system paths for this page were loaded from cache in this
  // request to avoid writing to cache on every request.
  $cache = &backdrop_static('backdrop_lookup_path', array());
  if (empty($cache['system_paths']) && !empty($cache['map'])) {
    // Generate a cache ID (cid) specifically for this page.
    $cid = current_path();
    // The static $map array used by backdrop_lookup_path() includes all
    // system paths for the page request.
    if ($paths = current($cache['map'])) {
      $data = array_keys($paths);
      $expire = REQUEST_TIME + (60 * 60 * 24);
      cache('path')->set($cid, $data, $expire);
    }
  }
}

/**
 * Given an internal Backdrop path, return the alias set by the administrator.
 *
 * If no path is provided, the function will return the alias of the current
 * page.
 *
 * @param $path
 *   An internal Backdrop path.
 * @param $langcode
 *   An optional language code to look up the path in.
 *
 * @return
 *   An aliased path if one was found, or the original path if no alias was
 *   found.
 *
 * @since 1.28.0: Added support for sub-path aliases.
 */
function backdrop_get_path_alias($path = NULL, $langcode = NULL) {
  // If no path is specified, use the current page's path.
  if ($path == NULL) {
    $path = $_GET['q'];
  }

  $path_alias = $path;

  // Sub-path alias functionality. Start by assuming that this is not a
  // sub-path.
  $sub_path = '';
  $path_parts = explode('/', $path);
  // Only consider providing sub-path aliases for non-admin paths and paths that
  // have a depth of greater then 2 (entity type path / entity ID / sub-path).
  $is_admin_path = in_array($path_parts[0], array('admin', 'batch', 'js'));

  // Exclude the login and logout paths.
  $exclude = count($path_parts) > 1 && $path_parts[0] == 'user' && in_array($path_parts[1], array('login', 'logout'));

  if (!$is_admin_path && !$exclude && count($path_parts) > 2) {
    // Special handling for taxonomy term sub-paths. Makes sure that the
    // $path_parts array is reduced to 3 items, which is what the equivalent
    // array for a user or node path would have:
    // - $path_parts[0] holds the entire path for the specific entity type.
    // - $path_parts[1] holds the entity ID.
    // - $path_parts[2] and the rest of the array items hold the sub-path.
    if ($path_parts[0] == 'taxonomy' && $path_parts[1] == 'term' && count($path_parts) > 3) {
      // Append the "/term" path part to "taxonomy" in the first item of the
      // array.
      $path_parts[0] = 'taxonomy/term';
      // Remove the array item that used to hold the individual "term" part.
      unset($path_parts[1]);
      // Re-index the array.
      $path_parts = array_values($path_parts);
    }

    $path_parent_part = $path_parts[0];
    // Check if the first path part is any of the paths for the known/supported
    // core entity types.
    $known_entity_type_paths = array('node', 'user', 'taxonomy/term');
    // @to-do:
    // - Consider providing a hook to allow checking for and handling
    //   sub-paths for other entity types.
    // - Can we get the list of possible entity type paths programmatically
    //   instead of hard-coding it? Calling entity_get_info() here in order to
    //   get a list of entity types is too expensive. Perhaps a helper function
    //   that caches the results?
    // - Consider sub-path support to files (when we have file paths) and
    //   comments?
    if (in_array($path_parent_part, $known_entity_type_paths)) {
      // Check whether the second part of the path is an integer (which is what
      // entity IDs are). is_int() does not work as expected here, since the
      // expected numeric path part is actually a string; and is_numeric() is
      // also less than ideal, since it returns TRUE for more than just
      // integers. Since the path part we are checking is a actually a string
      // representation of an integer, we are checking if the string consists of
      // all digits instead.
      if (ctype_digit($path_parts[1])) {
        // Safe to assume that this is a sub-path. Reconstruct the main part
        // from the entity type path and the entity ID.
        $path_entity_id = $path_parts[1];
        $path = $path_parent_part . '/' . $path_entity_id;
        // Remove the parent and entity ID from the array, and use the rest of
        // the path parts to construct the sub-path.
        unset($path_parts[0]);
        unset($path_parts[1]);
        $sub_path = '/' . implode('/', $path_parts);
      }
    }
  }

  // Lookup the alias for the path. In the case of sub-paths, the check is
  // performed against the "parent" path + the entity ID portion (constructed
  // earlier), and then the sub-path portion is appended to the parent alias if
  // found.
  if ($alias = backdrop_lookup_path('alias', $path, $langcode)) {
    $path_alias = $alias . $sub_path;
  }

  return $path_alias;
}

/**
 * Given a URL alias, return the internal path it represents.
 *
 * @param $path
 *   A Backdrop URL alias.
 * @param $langcode
 *   An optional language code to look up the path in.
 *
 * @return
 *   The internal path represented by the alias, or the original alias if no
 *   internal path was found.
 */
function backdrop_get_normal_path($path, $langcode = NULL) {
  $original_path = $path;

  // First, lookup the source path of the entire provided alias and use that if
  // found.
  if ($source = backdrop_lookup_path('source', $path, $langcode)) {
    $path = $source;
  }
  else {
    // If no matching source path was found for the provided alias, consider
    // whether this is a sub-path alias.
    $path_parts = explode('/', $path);
    $is_admin_path = in_array($path_parts[0], array('admin', 'batch', 'js'));

    // Exclude the login and logout paths.
    $exclude = count($path_parts) > 1 && $path_parts[0] == 'user' && in_array($path_parts[1], array('login', 'logout'));

    // Only consider non-admin paths and paths that have a depth of greater then
    // 1 (we need at the very least a path alias + a sub-path).
    if (!$is_admin_path && !$exclude && count($path_parts) > 1) {
      // Start with the simplest possible case of a single path item, and assume
      // that the rest of the path items are the sub-path.
      $sub_path_parts = array();
      $source = FALSE;
      while (count($path_parts) > 1 && $source == FALSE) {
        // Assume that the last array item is the sub-path. Store that and
        // remove it from the array.
        $sub_path_parts[] = array_pop($path_parts);
        // Assume that the rest of the array items are the actual path alias.
        $path = implode('/', $path_parts);
        // Try to find a source match for that remaining assumed path alias.
        if ($source = backdrop_lookup_path('source', $path, $langcode)) {
          $path = $source;
          $sub_path = implode('/', array_reverse($sub_path_parts));
          // If a match is found, append the sub-path to it.
          $path .= '/' . $sub_path;
        }
        else {
          // If no match is fount, fall back to the original path.
          $path = $original_path;
        }
      }
    }
  }

  // Allow other modules to alter the inbound URL. backdrop_alter() cannot be
  // used here, because we need to run hook_url_inbound_alter() in the reverse
  // order of hook_url_outbound_alter().
  foreach (array_reverse(module_implements('url_inbound_alter')) as $module) {
    $function = $module . '_url_inbound_alter';
    $function($path, $original_path, $langcode);
  }

  return $path;
}

/**
 * Check if the current page is the home page.
 *
 * @return
 *   Boolean value: TRUE if the current page is the home page; FALSE if otherwise.
 */
function backdrop_is_front_page() {
  // Use the advanced backdrop_static() pattern, since this is called very often.
  static $backdrop_static_fast;
  if (!isset($backdrop_static_fast)) {
    $backdrop_static_fast['is_front_page'] = &backdrop_static(__FUNCTION__);
  }
  $is_front_page = &$backdrop_static_fast['is_front_page'];

  if (!isset($is_front_page)) {
    // As backdrop_path_initialize updates $_GET['q'] with the 'site_frontpage' path,
    // we can check it against the 'site_frontpage' variable.
    $is_front_page = ($_GET['q'] == config_get('system.core', 'site_frontpage'));
  }

  return $is_front_page;
}

/**
 * Check if a path matches any pattern in a set of patterns.
 *
 * @param $path
 *   The path to match.
 * @param $patterns
 *   String containing a set of patterns separated by \n, \r or \r\n.
 *
 * @return
 *   Boolean value: TRUE if the path matches a pattern, FALSE otherwise.
 */
function backdrop_match_path($path, $patterns) {
  $regexps = &backdrop_static(__FUNCTION__);

  if (!isset($regexps[$patterns])) {
    // Convert path settings to a regular expression.
    // Therefore replace newlines with a logical or, /* with asterisks and the <front> with the frontpage.
    $to_replace = array(
      '/(\r\n?|\n)/', // newlines
      '/\\\\\*/',     // asterisks
      '/(^|\|)\\\\<front\\\\>($|\|)/' // <front>
    );
    $replacements = array(
      '|',
      '.*',
      '\1' . preg_quote(config_get('system.core', 'site_frontpage'), '/') . '\2'
    );
    $patterns_quoted = preg_quote($patterns, '/');
    $regexps[$patterns] = '/^(' . preg_replace($to_replace, $replacements, $patterns_quoted) . ')$/';
  }
  return (bool) preg_match($regexps[$patterns], (string) $path);
}

/**
 * Return the current URL path of the page being viewed.
 *
 * Examples:
 * - http://example.com/node/306 returns "node/306".
 * - http://example.com/backdrop_folder/node/306 returns "node/306" while
 *   base_path() returns "/backdrop_folder/".
 * - http://example.com/url-alias (which is a URL alias for node/306) returns
 *   "node/306" as opposed to the URL alias.
 *
 * This function is not available in hook_boot() so use $_GET['q'] instead.
 * However, be careful when doing that because in the case of Example #3
 * $_GET['q'] will contain "path/alias". If "node/306" is needed, calling
 * backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL) makes this function available.
 *
 * @return
 *   The current Backdrop URL path. The path is untrusted user input and must be
 *   treated as such.
 *
 * @see request_path()
 */
function current_path() {
  if (isset($_GET['q'])) {
    return $_GET['q'];
  }
}

/**
 * Fetches a specific URL alias from the database.
 *
 * @param $conditions
 *   A string representing the source, a number representing the pid, or an
 *   array of query conditions.
 *
 * @return
 *   FALSE if no alias was found or an associative array containing the
 *   following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: Unique URL alias identifier.
 *   - langcode: The language code of the alias.
 *   - auto: Boolean indicating if this alias was created from a pattern.
 */
function path_load($conditions) {
  if (is_numeric($conditions)) {
    $conditions = array('pid' => $conditions);
  }
  elseif (is_string($conditions)) {
    $conditions = array('source' => $conditions);
  }
  elseif (!is_array($conditions)) {
    return FALSE;
  }
  $select = db_select('url_alias');
  foreach ($conditions as $field => $value) {
    $select->condition($field, $value);
  }
  $path = $select
    ->fields('url_alias')
    ->orderBy('pid', 'DESC')
    ->execute()
    ->fetchAssoc();
  // Cast the auto column to boolean or NULL if unknown.
  if ($path) {
    $path['auto'] = is_null($path['auto']) ? NULL : (bool) $path['auto'];
  }
  return $path;
}

/**
 * Load a collection of URL aliases from the database all at once.
 *
 * @param array $keys
 *   An array of keys by which the aliases should be loaded. This array may
 *   contain a list of PIDs, sources, or aliases. The type of content within
 *   this array is determined by the $field parameter.
 * @param string $field
 *   The type of data used within the $keys array. May be either "pid",
 *   "source", or "alias".
 * @param string $langcode
 *   The langcode of the paths to be loaded.
 *
 * @return array
 *   An array of the loaded paths, keyed by the $field parameter values.
 */
function path_load_multiple(array $keys, $field, $langcode) {
  $subquery = db_select('url_alias')
    ->condition($field, $keys, 'IN')
    ->condition('langcode', $langcode)
    ->groupBy($field);
  $subquery->addExpression('MAX(pid)', 'pid');

  $paths = db_select('url_alias')
    ->fields('url_alias')
    ->condition('pid', $subquery, 'IN')
    ->execute()
    ->fetchAllAssoc($field, PDO::FETCH_ASSOC);

  // Cast the auto column to boolean or NULL if unknown.
  foreach ($paths as $key => $path) {
    $paths[$key]['auto'] = is_null($path['auto']) ? NULL : (bool) $path['auto'];
  }

  return $paths;
}

/**
 * Save a URL alias to the database.
 *
 * @param $path
 *   An associative array containing the following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: (optional) Unique URL alias identifier.
 *   - langcode: (optional) The language code of the alias.
 */
function path_save(&$path) {
  $path += array('langcode' => LANGUAGE_NONE);

  // Load the stored alias, if any.
  if (!empty($path['pid']) && !isset($path['original'])) {
    $path['original'] = path_load($path['pid']);
  }

  if (empty($path['pid'])) {
    backdrop_write_record('url_alias', $path);
    module_invoke_all('path_insert', $path);
  }
  else {
    backdrop_write_record('url_alias', $path, array('pid'));
    module_invoke_all('path_update', $path);
  }

  // Clear internal properties.
  unset($path['original']);

  // Clear the static alias cache.
  backdrop_clear_path_cache($path['source']);
}

/**
 * Delete a URL alias.
 *
 * @param $criteria
 *   A number representing the pid or an array of criteria.
 */
function path_delete($criteria) {
  if (!is_array($criteria)) {
    $criteria = array('pid' => $criteria);
  }
  // Load and then delete each path until there are no more aliases left.
  while ($path = path_load($criteria)) {
    module_invoke_all('path_delete', $path);
    backdrop_clear_path_cache($path['source']);
    db_delete('url_alias')->condition('pid', $path['pid'])->execute();
  }
}

/**
 * Determines whether a path is in the administrative section of the site.
 *
 * By default, paths are considered to be non-administrative. If a path does
 * not match any of the patterns in path_get_admin_paths(), or if it matches
 * both administrative and non-administrative patterns, it is considered
 * non-administrative.
 *
 * @param $path
 *   A Backdrop path.
 *
 * @return
 *   TRUE if the path is administrative, FALSE otherwise.
 *
 * @see path_get_admin_paths()
 * @see hook_admin_paths()
 * @see hook_admin_paths_alter()
 */
function path_is_admin($path) {
  $path_map = &backdrop_static(__FUNCTION__);
  if (!isset($path_map['admin'][$path])) {
    $patterns = path_get_admin_paths();
    $path_map['admin'][$path] = backdrop_match_path($path, $patterns['admin']);
    $path_map['non_admin'][$path] = backdrop_match_path($path, $patterns['non_admin']);
  }
  return $path_map['admin'][$path] && !$path_map['non_admin'][$path];
}

/**
 * Gets a list of administrative and non-administrative paths.
 *
 * @return array
 *   An associative array containing the following keys:
 *   'admin': An array of administrative paths and regular expressions
 *            in a format suitable for backdrop_match_path().
 *   'non_admin': An array of non-administrative paths and regular expressions.
 *
 * @see hook_admin_paths()
 * @see hook_admin_paths_alter()
 */
function path_get_admin_paths() {
  $patterns = &backdrop_static(__FUNCTION__);
  if (!isset($patterns)) {
    $paths = module_invoke_all('admin_paths');
    backdrop_alter('admin_paths', $paths);
    // Combine all admin paths into one array, and likewise for non-admin paths,
    // for easier handling.
    $patterns = array();
    $patterns['admin'] = array();
    $patterns['non_admin'] = array();
    foreach ($paths as $path => $enabled) {
      if ($enabled) {
        $patterns['admin'][] = $path;
      }
      else {
        $patterns['non_admin'][] = $path;
      }
    }
    $patterns['admin'] = implode("\n", $patterns['admin']);
    $patterns['non_admin'] = implode("\n", $patterns['non_admin']);
  }
  return $patterns;
}

/**
 * Checks a path exists and the current user has access to it.
 *
 * @param $path
 *   The path to check.
 * @param $dynamic_allowed
 *   Whether paths with menu wildcards (like user/%) should be allowed.
 *
 * @return
 *   TRUE if it is a valid path AND the current user has access permission,
 *   FALSE otherwise.
 */
function backdrop_valid_path($path, $dynamic_allowed = FALSE) {
  global $menu_admin;
  // We indicate that a menu administrator is running the menu access check.
  $menu_admin = TRUE;
  if ($path == '<front>' || url_is_external($path)) {
    $item = array('access' => TRUE);
  }
  elseif ($dynamic_allowed && preg_match('/\/\%/', $path)) {
    // Path is dynamic (ie 'user/%'), so check directly against menu_router table.
    if ($item = db_query("SELECT * FROM {menu_router} where path = :path", array(':path' => $path))->fetchAssoc()) {
      $item['link_path']  = $item['path'];
      $item['link_title'] = $item['title'];
      $item['external']   = FALSE;
      $item['options'] = '';
      _menu_link_translate($item);
    }
  }
  else {
    $item = menu_get_item($path);
  }
  $menu_admin = FALSE;
  return $item && $item['access'];
}

/**
 * Clear the path cache.
 *
 * @param $source
 *   An optional system path for which an alias is being changed.
 */
function backdrop_clear_path_cache($source = NULL) {
  // Clear the backdrop_lookup_path() static cache.
  backdrop_static_reset('backdrop_lookup_path');
}
