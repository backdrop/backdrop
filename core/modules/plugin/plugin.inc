<?
/**
 * @file plugin.inc
 */

/**
 * Get plugins of $type.
 */
function plugin_info($type = NULL, $reset = FALSE) {
  $plugins = &drupal_static(__FUNCTION__, NULL, $reset);

  // We never need to run this more than once in a request, so if $plugins has
  // the data, we return it earlier.
  if (is_array($plugins)) {
    if ($type === NULL) {
      return $plugins;
    } else {
      // We don't check the key for existence, because if a module asks for a
      // non-existent plugin type, it SHOULD be warned, and we don't want to
      // unnecessarily invoke the hook again.
      return $plugins[$type];
    }
  } elseif (is_object($cached = cache()->get('plugins'))) {
    return $cached->data['type'];
  }

  $plugins = module_invoke_all('plugin_info');

  // This is potentially expensive, so we force modules that need to alter to
  // set this variable, e.g., in their install hook.
  if (in_array(__FUNCTION__, variable_get('drupal_alter_allowed', array()))) {
    drupal_alter(__FUNCTION__, $plugins);
  }

  cache()->set('plugins', $plugins, CACHE_TEMPORARY);

  return $type ? $plugins[$type] : $plugins;
}