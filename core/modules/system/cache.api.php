<?php

/**
 * @file
 * Hooks added by Backdrop core related to cache handling.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Allows to react to emitting a cache tags header.
 *
 * @param array $tags
 *   The tags to emit to the reverse proxy.
 */
function hook_emit_cache_tags(array $tags) {
  backdrop_add_http_header('Surrogate-Key', implode(' ', $tags));
}

/**
 * Allows to alter the cache tags prior to emitting them.
 *
 * @param array $tags
 *   The tags that will be emitted via hook_emit_cache_tags().
 */
function hook_pre_emit_cache_tags_alter(array &$tags) {
  if (arg(0) == 'mymodule') {
    $tags[] = 'myspecial-tag';
  }
}

/**
 * Allows to react to emitting a cache max-age header.
 *
 * @param int $max_age
 *   The maximum age to set or CACHE_MAX_AGE_PERMANENT.
 */
function hook_emit_cache_max_age($max_age) {
  $page_cache_maximum_age = variable_get('page_cache_maximum_age', 0);
  if ($max_age == CACHE_MAX_AGE_PERMANENT || $max_age > $page_cache_maximum_age) {
    $max_age = $page_cache_maximum_age;
  }
  if (!isset($_COOKIE[session_name()])) {
    header('Cache-Control', 'public, max-age=' . $max_age);
  }
}

/**
 * Allows to alter the cache max-age prior to emitting it.
 *
 * @param int $max_age
 *   The max_age that will be emitted via hook_emit_cache_max_age().
 */
function hook_pre_emit_cache_max_age_alter(&$max_age) {
  if (arg(0) == 'mymodule') {
    $max_age = CACHE_MAX_AGE_PERMANENT;
  }
}

/**
 * Allows to react to invalidating cache tags.
 *
 * This hook is useful to e.g. invalidate the tags within an external reverse
 * proxy like Varnish.
 *
 * @param array $tags
 *   The cache tags that should be invalidated.
 */
function hook_invalidate_cache_tags(array $tags) {
  mymodule_varnish_clear_cache_tags($tags);
}

/**
 * Allows to alter the cache tags before invalidating them.
 *
 * @param array $tags
 *   The tags that will be invalidated via hook_invalidate_cache_tags().
 */
function hook_pre_invalidate_cache_tags_alter(array &$tags) {
  $index_tags = array_flip($tags);

  // Remove the node_list tag as it invalidates too much.
  if (isset($index_tags['node_list'])) {
    unset($tags[$index_tags['node_list']]);
  }
}

/**
 * @} End of "addtogroup hooks".
 */
