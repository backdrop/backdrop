<?php
/**
 * @file
 * API for loading and interacting with Backdrop modules, themes and layout
 * templates.
 */

/**
 * Builds a list of modules, themes, layout templates.
 *
 * @param $type
 *   The type of list to return:
 *   - module_enabled: All enabled modules.
 *   - bootstrap: All enabled modules required for bootstrap.
 *   - theme: All themes.
 *
 * @return
 *   An associative array of modules, themes, or layout templates, keyed by
 *   name. For $type 'bootstrap', the array values equal the keys. For $type
 *   'module', 'theme', or 'layout', the array values are objects representing
 *   the respective database row, with the 'info' property already unserialized.
 *
 * @see module_list()
 * @see list_themes()
 */
function system_list($type) {
  $lists = &backdrop_static(__FUNCTION__);

  // For bootstrap modules, attempt to fetch the list from cache if possible.
  // if not fetch only the required information to fire bootstrap hooks
  // in case we are going to serve the page from cache.
  if ($type == 'bootstrap') {
    if (isset($lists['bootstrap'])) {
      return $lists['bootstrap'];
    }
    if ($cached = cache('bootstrap')->get('bootstrap_modules')) {
      $bootstrap_list = $cached->data;
    }
    else {
      $bootstrap_list = db_query("SELECT name, filename FROM {system} WHERE status = 1 AND bootstrap = 1 AND type = 'module' ORDER BY weight ASC, name ASC")->fetchAllAssoc('name');
      cache('bootstrap')->set('bootstrap_modules', $bootstrap_list);
    }
    // To avoid a separate database lookup for the filepath, prime the
    // backdrop_get_filename() static cache for bootstrap modules only.
    // The rest is stored separately to keep the bootstrap module cache small.
    foreach ($bootstrap_list as $module) {
      backdrop_get_filename('module', $module->name, $module->filename);
    }
    // We only return the module names here since module_list() doesn't need
    // the filename itself.
    $lists['bootstrap'] = array_keys($bootstrap_list);
  }
  // Otherwise build the list for enabled modules and themes.
  elseif (!isset($lists['module_enabled'])) {
    if ($cached = cache('bootstrap')->get('system_list')) {
      $lists = $cached->data;
    }
    else {
      $lists = array(
        'module_enabled' => array(),
        'theme' => array(),
        'filepaths' => array(),
      );
      // The module name (rather than the filename) is used as the fallback
      // weighting in order to guarantee consistent behavior across different
      // Backdrop installations, which might have modules installed in different
      // locations in the file system. The ordering here must also be
      // consistent with the one used in module_implements().
      $result = db_query("SELECT * FROM {system} WHERE type = 'theme' OR (type = 'module' AND status = 1) ORDER BY weight ASC, name ASC");
      foreach ($result as $record) {
        $record->info = unserialize($record->info);
        // Build a list of all enabled modules.
        if ($record->type == 'module') {
          $lists['module_enabled'][$record->name] = $record;
        }
        // Build a list of themes.
        if ($record->type == 'theme') {
          $lists['theme'][$record->name] = $record;
        }
        // Build a list of filenames so backdrop_get_filename can use it.
        if ($record->status) {
          $lists['filepaths'][] = array('type' => $record->type, 'name' => $record->name, 'filepath' => $record->filename);
        }
      }
      foreach ($lists['theme'] as $key => $theme) {
        if (!empty($theme->info['base theme'])) {
          // Make a list of the theme's base themes.
          require_once BACKDROP_ROOT . '/core/includes/theme.inc';
          $lists['theme'][$key]->base_themes = backdrop_find_base_themes($lists['theme'], $key);
          // Don't proceed if there was a problem with the root base theme.
          if (!current($lists['theme'][$key]->base_themes)) {
            continue;
          }
          // Determine the root base theme.
          $base_key = key($lists['theme'][$key]->base_themes);
          // Add to the list of sub-themes for each of the theme's base themes.
          foreach (array_keys($lists['theme'][$key]->base_themes) as $base_theme) {
            $lists['theme'][$base_theme]->sub_themes[$key] = $lists['theme'][$key]->info['name'];
          }
          // Add the base theme's theme engine info.
          $lists['theme'][$key]->info['engine'] = $lists['theme'][$base_key]->info['engine'];
        }
        else {
          // A plain theme is its own base theme.
          $base_key = $key;
        }
        // Set the theme engine prefix.
        $lists['theme'][$key]->prefix = ($lists['theme'][$key]->info['engine'] == 'theme') ? $base_key : $lists['theme'][$key]->info['engine'];
      }
      cache('bootstrap')->set('system_list', $lists);
    }
    // To avoid a separate database lookup for the filepath, prime the
    // backdrop_get_filename() static cache with all enabled modules and themes.
    foreach ($lists['filepaths'] as $item) {
      backdrop_get_filename($item['type'], $item['name'], $item['filepath']);
    }
  }

  return $lists[$type];
}
