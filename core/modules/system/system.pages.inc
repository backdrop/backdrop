<?php
/**
 * @file
 * System module non-administrative page callbacks.
 */

/**
 * Page callback to output a token tree as an empty page.
 */
function system_token_output() {
  $types = isset($_GET['token_types']) ? backdrop_json_decode($_GET['token_types']) : array();
  $globals = isset($_GET['global_types']) ? backdrop_json_decode($_GET['global_types']) : TRUE;

  $out = system_token_browser_tree_build_content($types, $globals);
  $out = system_token_browser_render($out);

  if (backdrop_is_ajax()) {
    $dialog_options = array(
      'dialogClass' => 'token-browser-dialog',
      'width' => 900,
      'modal' => FALSE,
      'draggable' => TRUE,
      'resizable' => TRUE,
      'autoResize' => FALSE,
    );
    $commands = array(
      ajax_command_open_dialog('#token-dialog', t('Token Browser'), $out, $dialog_options),
    );
    $return = array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
    return $return;
  }
  else {
    return $out;
  }
}

/**
 * Token browser tree access callback.
 */
function system_token_browser_tree_access() {
  return isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'token-browser');
}

/**
 * Token browser JSON endpoint callback.
 *
 * @param string $type
 *   The token type.
 */
function system_token_browser_endpoint($type) {
  $definition = array(
    'ancestors' => array(
      'filter' => FILTER_CALLBACK,
      'options' => 'filter_xss',
    ),
  );

  $input = filter_input_array(INPUT_GET, $definition);
  $ancestors = isset($input['ancestors']) ? backdrop_json_decode($input['ancestors']) : array();

  return system_token_browser_build_level($type, $ancestors);
}

/**
 * Token browser JSON endpoint access callback.
 */
function system_token_browser_endpoint_access() {
  return isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'token-browser-endpoint');
}

/**
 * Token browser JSON endpoint delivery callback.
 */
function system_token_browser_endpoint_output($page_callback_result) {
  if (is_int($page_callback_result)) {
    backdrop_deliver_html_page($page_callback_result);
  }
  elseif (isset($page_callback_result)) {
    backdrop_json_output($page_callback_result);
  }
  else {
    backdrop_page_header();
    backdrop_page_footer();
  }
}

/**
 * Helper function to generate a render array of token types.
 * 
 * @param array|string $type
 *   An array of token types to display in the tree or the string 'all'.
 * @param bool $global_types
 *   Whether or not to include global types in the tree.
 * 
 * @return array
 *   The constructed array of types keyed by type.
 * 
 */
function system_token_browser_tree_build_content($types, $global_types) {
  global $language;

  $info = token_get_info();
  $rows = array();
  $posinset = 1;

  if ($types === 'all') {
    $types = array_keys($info['types']);
  }
  elseif ($global_types) {
    $types = array_merge($types, token_get_global_token_types());
  }

  $hash = hash('sha256', serialize(array(
    'token_types' => $types,
    'global_types' => $global_types,
    'language' => $language->langcode,
  )));

  $cid = 'token-browser-build:' . $hash;

  if ($cache = cache_get($cid, 'cache_token')) {
    return $cache->data;
  }

  foreach ($info['types'] as $type => $type_info) {
    if (!in_array($type, $types) || substr($type, 0, 4) === 'list') {
      continue;
    }

    $rows[] = array(
      'data' => array(
        array(
          'data' => '<button aria-label="Expand">Expand</button>' . $type_info['name'],
          'data-type' => $type,
          'role' => 'gridcell',
          'class' => array('token-name'),
        ),
        array(
          'data' => $type,
          'role' => 'gridcell',
          'class' => array('token-raw'),
        ),
        array(
          'data' => isset($type_info['description']) ? $type_info['description'] : '',
          'role' => 'gridcell',
          'class' => array('token-description'),
        ),
      ),
      'role' => 'row',
      'aria-level' => '1',
      'aria-posinset' => $posinset++,
      'aria-expanded' => 'false',
      'aria-busy' => 'false',
      'class' => array('tree-grid-parent'),
      'no_striping' => TRUE,
    );
  }

  $build = array(
    '#theme' => 'table',
    '#header' => array(
      t('Name'),
      t('Token'),
      t('Description'),
    ),
    '#rows' => $rows,
    '#attributes' => array(
      'role' => 'treetable',
      'aria-label' => 'Token Browser',
      'class' => array(
        'tree-grid',
      ),
    ),
    '#attached' => array(
      'library' => array(
        array('system', 'token'),
      ),
    ),
    '#cache' => array(
      'cid' => 'token-browser-render:' . $hash,
      'bin' => 'cache_token',
    ),
  );

  cache_set($cid, $build, 'cache_token');

  return $build;
}

/**
 * Build a level of the token hierarchy.
 *
 * @param string $type
 *   The type to build a level for.
 * @param array $ancestors
 *   The ancestors of the given type. This allows the complete token to be
 *   constructed.
 * @return array
 *   The constructed level of tokens as an array keyed by token.
 */
function system_token_browser_build_level($type, array $ancestors = array()) {
  global $language;

  $level = array();
  $info = token_get_info();

  if (!isset($info['types'][$type]) || !isset($info['tokens'][$type])) {
    return $level;
  }

  if ($entity_token = entity_get_info($type)) {
    $type = $entity_token['token type'];
  }

  $cid = 'token-browser-level:' . hash('sha256', serialize(array(
    'type' => $type,
    'ancestors' => $ancestors,
    'language' => $language->langcode,
  )));

  if ($cache = cache_get($cid, 'cache_token')) {
    return $cache->data;
  }

  foreach ($info['tokens'][$type] as $child => $child_info) {
    $raw_token_parts = $ancestors;

    if (in_array($child, array_slice($raw_token_parts, 1), TRUE)) {
      continue;
    }

    $raw_token_parts[] = $child;

    if (!empty($child_info['dynamic'])) {
      $raw_token_parts[] = '?';
    }

    $raw_token = '[' . implode(':', $raw_token_parts) . ']';
    $level[$child]['name'] = $child_info['name'];
    $level[$child]['description'] = $child_info['description'];
    $level[$child]['token'] = $child;

    if (isset($child_info['type']) && isset($info['types'][$child_info['type']])) {
      $level[$child]['type'] = $child_info['type'];
    }

    $level[$child]['raw'] = $raw_token;
    $level[$child]['ancestors'] = $ancestors;
  }

  cache_set($cid, $level, 'cache_token');

  return $level;
}

/**
 * Sets the request method to GET before calling backdrop_render().
 *
 * Fools backdrop_render_cache_set() and backdrop_render_cache_get() into
 * working even when the request method is other than GET or HEAD. This is
 * necessary because the jQuery.load() method requests data via POST, which
 * would normally prevent render caching.
 *
 * @param array $elements
 *   The structured array describing the data to be rendered.
 * @return string
 *   The result of backdrop_render().
 */
function system_token_browser_render(array &$elements) {
  $method = $_SERVER['REQUEST_METHOD'];
  $_SERVER['REQUEST_METHOD'] = 'GET';
  $output = backdrop_render($elements);
  $_SERVER['REQUEST_METHOD'] = $method;

  return $output;
}
