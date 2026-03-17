<?php
/**
 * @file
 * Administrative page callbacks for the Icon Picker module.
 */

/**
 * Form constructor: Simple test form for the icon_select element.
 *
 */
function icon_picker_test_form($form, &$form_state) {
  $form['my_icon'] = array(
    '#type' => 'icon_select',
    '#title' => t('Choose an icon'),
    '#description' => t('Select an icon using the picker, or type a name directly.'),
    '#default_value' => '',
  );
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for the icon_select test form.
 *
 */
function icon_picker_test_form_submit($form, &$form_state) {
  backdrop_set_message(t('Selected icon: %icon', array('%icon' => $form_state['values']['my_icon'])));
}

/**
 * Page callback: Returns a JSON list of available icons.
 *
 * Accepts GET parameters:
 *   - search: Partial icon name to filter by (optional).
 *   - page:   Zero-based page number (optional, default 0).
 *   - limit:  Results per page, 1–100 (optional, default 40).
 *
 * @return array
 *   A structured array passed to backdrop_json_output().
 */
function icon_picker_json() {
  // icon() uses $GLOBALS['theme_key'] to locate theme icon overrides. Ensure
  // it is set when this endpoint is called before a full page bootstrap.
  _icon_picker_ensure_theme_key();

  $limit = isset($_GET['limit']) ? (int) $_GET['limit'] : 40;
  $limit = max(1, min(100, $limit));
  $page  = isset($_GET['page']) ? max(0, (int) $_GET['page']) : 0;
  $query = isset($_GET['search']) ? trim(backdrop_strtolower($_GET['search'])) : '';

  $icons = _icon_picker_get_all_names();

  if ($query !== '') {
    $icons = array_values(array_filter($icons, function ($name) use ($query) {
      return strpos(backdrop_strtolower($name), $query) !== FALSE;
    }));
  }

  $total      = count($icons);
  $page_icons = array_slice($icons, $page * $limit, $limit);

  $results = array();
  foreach ($page_icons as $name) {
    $results[] = array(
      'name'   => $name,
      'markup' => icon($name),
    );
  }

  return array(
    'total'   => $total,
    'count'   => count($results),
    'page'    => $page,
    'limit'   => $limit,
    'results' => $results,
  );
}

/**
 * Returns a sorted list of all available icon names.
 *
 * Collects names from core's icons directory, the active theme's icons
 * directory, and any enabled modules that implement hook_icon_info().
 * Results are cached temporarily and cleared on module or theme changes.
 *
 * @return array
 *   A sorted array of icon name strings.
 */
function _icon_picker_get_all_names() {
  $cache = cache()->get('icon_picker_all_names');
  if ($cache) {
    return $cache->data;
  }

  _icon_picker_ensure_theme_key();

  $names = array();

  // Core icons.
  $names = array_merge($names, _icon_picker_names_from_directory(BACKDROP_ROOT . '/core/misc/icons'));

  // Active theme icons.
  if (!empty($GLOBALS['theme_key'])) {
    $theme_path = backdrop_get_path('theme', $GLOBALS['theme_key']);
    if ($theme_path) {
      $names = array_merge($names, _icon_picker_names_from_directory(BACKDROP_ROOT . '/' . $theme_path . '/icons'));
    }
  }

  // Module-provided icons via hook_icon_info().
  $icon_info = icon_get_info();
  if (!empty($icon_info)) {
    $names = array_merge($names, array_keys($icon_info));
  }

  $names = array_unique($names);
  sort($names, SORT_STRING);

  cache()->set('icon_picker_all_names', $names, CACHE_TEMPORARY);

  return $names;
}

/**
 * Returns icon names found in a directory of SVG files.
 *
 * @param string $directory
 *   Absolute filesystem path to scan.
 *
 * @return array
 *   Icon name strings (SVG filenames without the .svg extension).
 */
function _icon_picker_names_from_directory($directory) {
  $names = array();
  if (!is_dir($directory)) {
    return $names;
  }
  $files = file_scan_directory($directory, '/\.svg$/');
  foreach ($files as $file) {
    $names[] = basename($file->filename, '.svg');
  }
  return $names;
}

/**
 * Ensures $GLOBALS['theme_key'] is set before the theme system bootstraps.
 *
 * The icon() API resolves theme-provided icon overrides using
 * $GLOBALS['theme_key']. When icon_picker_json() is called from an AJAX
 * request, the theme may not yet be initialised. This function sets a
 * sensible default so icon lookups work correctly.
 *
 * @see _icon_from_theme()
 */
function _icon_picker_ensure_theme_key() {
  if (!empty($GLOBALS['theme_key'])) {
    return;
  }
  $config = config('system.core');
  foreach (array($config->get('admin_theme'), $config->get('theme_default')) as $candidate) {
    if (!empty($candidate)) {
      $GLOBALS['theme_key'] = $candidate;
      return;
    }
  }
}
