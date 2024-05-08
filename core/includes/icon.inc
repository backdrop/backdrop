<?php
/**
 * @file
 * Provides the Backdrop API for placing icons.
 */

/**
 * Return the markup for outputting an icon by a machine name.
 *
 * This will first check if an icon is provided by the active theme (in the
 * theme's "icons" directory). If so, that icon will be used. Next, the
 * icon will be pulled from any module that provided the icon through
 * hook_icon_info(). If the icon is not provided by a theme or module, a
 * matching icon will be returned from the /core/misc/icons directory.
 *
 * If the "immutable" option is set to TRUE, the selection order will be
 * reversed, that is the original provider of an icon will return the icon
 * rather than allowing any overriding by a theme or module.
 *
 * @param string $icon_name
 *   The icon name such as "home", "info", etc. Do not include the file
 *   extension.
 * @param array $options
 *   An array of options to pass to the icon. Keys include:
 *   - alt: An alternative text to be used for screen readers. If not specified
 *     the icon will be marked as decorative with aria-hidden="true". Note that
 *     in the case of SVG icons, this will rendered as a <title> attribute
 *     within the <svg> tag.
 *   - attributes: Any additional attributes to add to the icon, including
 *     classes (as an array), name, id, etc.
 *   - immutable: Boolean value if this icon is not allowed to be overridden
 *     by themes or modules. If an icon is provided by core, it cannot be
 *     overridden, but an icon can still be added.
 *
 * @return string
 */
function icon($icon_name, array $options = array()) {
  // Populate default options.
  $options += array(
    'alt' => NULL,
    'attributes' => array(),
    'immutable' => FALSE,
  );

  $immutable = !empty($options['immutable']);
  $icon_path = icon_get_path($icon_name, $immutable);

  if ($icon_path) {
    unset($options['immutable']);
    return theme('icon', array(
      'name' => $icon_name,
      'path' => $icon_path,
      'alt' => $options['alt'],
      'attributes' => $options['attributes'],
    ));
  }

  return '<!-- ' . t('Icon @name not found.', array('@name' => $icon_name)) . ' -->';
}

/**
 * Locates an icon and returns its path.
 *
 * Icons can be provided by themes, modules, and core. Normally, icons are
 * provided by core (from the core/misc/icons directory), but modules can
 * provide additional icons or override core ones. Modules must implement
 * hook_icon_info() to provide icons. Themes can also provide icons or override
 * existing ones by placing icons in their "icons" subdirectory.
 *
 * @param string $icon_name
 *   The name of an icon without a file extension. Most icon names can be
 *   determined by looking in the core/misc/icons directory.
 * @param boolean $immutable
 *   Immutable icons cannot be modified by themes or modules. Instead of
 *   allowing overrides, the first system that defines an icon name defines
 *   its path. This allows certain icons to lock to the icon provided by core
 *   or a module without allowing a theme to override it. Useful in situations
 *   where an icon is used across multiple themes (like in the admin bar).
 *
 * @return string|NULL
 *   The path to an icon, relative to the Backdrop installation root, with the
 *   file extension (usually .svg). Or NULL if an icon is not found.
 *
 * @see hook_icon_info()
 * @see hook_icon_info_alter()
 */
function icon_get_path($icon_name, $immutable = FALSE) {
  // Save into a static cache with both normal and immutable paths.
  $icon_paths = &backdrop_static(__FUNCTION__, array(
    'normal' => array(),
    'immutable' => array(),
  ));

  $immutable_type = $immutable ? 'immutable' : 'normal';
  if (isset($icon_paths[$immutable_type][$icon_name])) {
    return $icon_paths[$immutable_type][$icon_name];
  }

  // Normal search order searches themes, then modules, then core.
  $search_functions = array(
    '_icon_from_theme',
    '_icon_from_module',
    '_icon_from_core',
  );
  // Immutable searches core, then modules, then themes.
  if ($immutable) {
    $search_functions = array_reverse($search_functions);
  }

  $icon_path = NULL;
  foreach ($search_functions as $function) {
    if ($icon_path = $function($icon_name)) {
      break;
    }
  }

  // Save in the static cache and return.
  $icon_paths[$immutable_type][$icon_name] = $icon_path;
  return $icon_path;
}

/**
 * Checks if the current theme provides an icon.
 *
 * Do not call this function directly. Use icon() instead.
 *
 * @param string $icon_name
 *   The icon name to be located.
 *
 * @return string|void
 *   The path to the icon if found.
 *
 * @see icon()
 * @private
 */
function _icon_from_theme($icon_name, $theme = NULL) {
  $theme = isset($theme) ? $theme : $GLOBALS['theme_key'];

  // Check if the theme provides a non-default icon path.
  $theme_icon_directory = theme_get_setting('icon_directory', $theme);

  // Otherwise just check in the theme "icons" directory.
  if (!$theme_icon_directory) {
    $theme_icon_directory = 'icons';
  }

  // Append the filename and extension, and check for a theme-provided icon.
  $theme_icon_path = backdrop_get_path('theme', $theme) . '/' . $theme_icon_directory . '/' . $icon_name . '.svg';

  // If the icon exists in this theme return it.
  if (file_exists($theme_icon_path)) {
    return $theme_icon_path;
  }

  // If icon does not exist in this theme, but this theme has a base theme,
  // check that location for an icon. Loop recursively through all base themes.
  $themes = list_themes();
  $theme_info = $themes[$theme];
  if (isset($theme_info->info['base theme'])) {
    return _icon_from_theme($icon_name, $theme_info->info['base theme']);
  }
}

/**
 * Checks if a theme provides an icon.
 *
 * Do not call this function directly. Use icon() instead.
 *
 * @param string $icon_name
 *   The icon name to be located.
 *
 * @return string|void
 *   The path to the icon if found.
 *
 * @see icon()
 * @private
 */
function _icon_from_module($icon_name) {
  $icon_info = icon_get_info($icon_name);
  if ($icon_info) {
    if (isset($icon_info['directory'])) {
      $directory = $icon_info['directory'];
    }
    else {
      $directory = backdrop_get_path('module', $icon_info['module']) . '/icons';
    }
    if (isset($icon_info['name'])) {
      $filename = $icon_info['name'] . '.svg';
    }
    else {
      $filename = $icon_name . '.svg';
    }
    $module_icon_path = $directory . '/' . $filename;
    if (file_exists($module_icon_path)) {
      return $module_icon_path;
    }
  }
}

/**
 * Checks if core provides an icon.
 *
 * Do not call this function directly. Use icon() instead.
 *
 * @param string $icon_name
 *   The icon name to be located.
 *
 * @return string|void
 *   The path to the icon if found.
 *
 * @see icon()
 * @private
 */
function _icon_from_core($icon_name) {
  // Finally, check for an icon in the misc/icons directory.
  $misc_icon_path = 'core/misc/icons/' . $icon_name . '.svg';
  if (file_exists($misc_icon_path)) {
    return $misc_icon_path;
  }
}

/**
 * Get info about a module-provided icons.
 *
 * Note this will not return information about core icons or theme-provided
 * icons.
 *
 * @param string|null $icon_name
 *   An icon name, with no extension, such as "home" or "info". If omitted,
 *   all icon info will be returned.
 * @return array
 */
function icon_get_info($icon_name = NULL) {
  $icon_info = &backdrop_static(__FUNCTION__);

  if (!isset($icon_info)) {
    $icon_info = array();
    foreach (module_implements('icon_info') as $module) {
      $module_icon_info = module_invoke($module, 'icon_info');
      // Populate the 'module' key for each returned item.
      foreach (array_keys($module_icon_info) as $module_icon_name) {
        $module_icon_info[$module_icon_name]['module'] = $module;
      }
      $icon_info = array_merge($icon_info, $module_icon_info);
    }
    backdrop_alter('icon_info', $icon_info);
  }
  if ($icon_name) {
    return isset($icon_info[$icon_name]) ? $icon_info[$icon_name] : NULL;
  }
  else {
    return $icon_info;
  }
}

/**
 * Returns HTML for an inline-icon.
 *
 * This effectively returns the contents of an SVG file. But it could
 * potentially be override to replace inlined SVGs with other mechanisms, like
 * an icon font.
 *
 * @param array $variables
 *   An associative array containing:
 *   - name: The machine name for the icon being displayed.
 *   - path: The full path to the icon file, relative to installation root.
 *   - alt: Alternative text for this icon. Default icons are SVGs and this
 *     value is added as a <title> tag within the SVG. If not specified, the
 *     icon will automatically be assigned the aria-hidden="true" attribute.
 *   - attributes: Attributes to be added to the icon itself.
 */
function theme_icon(array $variables) {
  // Ensure the filename is .svg.
  if (image_is_svg($variables['path'])) {
    // Ensure the file contents are an SVG.
    $svg_contents = file_get_contents($variables['path']);
    if (strpos($svg_contents, '<svg') === 0) {
      // Clean out any embedded XSS within the SVG. This very-restrictive set
      // of options should be adequate for icons.
      $svg_contents = filter_xss($svg_contents, array('svg', 'use', 'title',
        'desc', 'defs', 'linearGradient', 'stop', 'rect', 'circle', 'path'));

      // Move the "alt" text to an attribute.
      if ($variables['alt']) {
        $variables['attributes']['alt'] = $variables['alt'];
      }
      else {
        $variables['attributes']['aria-hidden'] = 'true';
      }

      // Add default icon classes.
      $variables['attributes']['class'] = array_merge(
        array('icon', 'icon--' . $variables['name']),
        isset($variables['attributes']['class']) ? $variables['attributes']['class'] : array()
      );

      return image_add_svg_attributes($svg_contents, $variables['attributes']);
    }
  }
}
