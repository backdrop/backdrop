<?php
/**
 * @file
 * Hooks provided by the Color module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define color information provided by a module.
 *
 * This hook enables modules to expose module-defined element colors to the
 * Color module.
 * 
 * For example:
 *   For a module which creates a DIV element
 *
 *     <div class="my-module-field"></div>
 *
 *   and a CSS file entry:
 *
 *     .my-module-field {
 *       background-color: #abcdef;
 *     }
 *
 *   The background color would be a field which can be now exposed to Color
 *   module, to allow modifying this background color through the color UI in
 *   theme settings pages.
 *
 *   This hook will only have an effect if the CSS file is made known to
 *   Backdrop, by being included in the module's info file, or by calling
 *   backdrop_get_css() or is in a hook_libray_info() implementation.
 *
 * @return
 *   An array of color information. This is an associative array containing the
 *   following items:
 *   - "fields": An associative array of elements to be colorized. The keys
 *     define the fields, while the values define the human-readable name. The
 *     human-readable name will appear in the color UI on the theme settings
 *     pages.
 *   - "default_colors": An associative array which defines the element colors,
 *     where the keys are the element field names and the values are the hex
 *     colors of the element as defined in the module's CSS file.
 *   - "css": An array of CSS file paths relative to the module path.
 */
function hook_color_info() {
  return array(
    'fields' => array(
      'my_module_field' => 'My module field background',
    ),
    'default_colors' => array(
      'my_module_field' => '#abcdef',
    ),
    'css' => array(
      'css/my-module.css',
    ),
  );
}
