<?php

/**
 * @defgroup themeable Default theme implementations
 * @{
 * Functions and templates for the user interface to be implemented by themes.
 *
 * Backdrop's presentation layer is a pluggable system known as the theme
 * layer. Each theme can take control over most of Backdrop's output, and
 * has complete control over the CSS.
 *
 * Inside Backdrop, the theme layer is utilized by the use of the theme()
 * function, which is passed the name of a component (the theme hook)
 * and an array of variables. For example,
 * theme('table', array('header' => $header, 'rows' => $rows));
 * Additionally, the theme() function can take an array of theme
 * hooks, which can be used to provide 'fallback' implementations to
 * allow for more specific control of output. For example, the function:
 * theme(array('table__foo', 'table'), $variables) would look to see if
 * 'table__foo' is registered anywhere; if it is not, it would 'fall back'
 * to the generic 'table' implementation. This can be used to attach specific
 * theme functions to named objects, allowing the themer more control over
 * specific types of output.
 *
 * Every theme hook is required to be registered by the module that owns it, so
 * that Backdrop can tell what to do with it and to make it simple for themes to
 * identify and override the behavior for these calls.
 *
 * The theme hooks are registered via hook_theme(), which returns an
 * array of arrays with information about the hook. It describes the
 * arguments the function or template will need, and provides
 * defaults for the template in case they are not filled in. If the default
 * implementation is a function, by convention it is named theme_HOOK().
 *
 * Each module should provide a default implementation for theme_hooks that
 * it registers. This implementation may be either a function or a template;
 * if it is a function it must be specified via hook_theme(). By convention,
 * default implementations of theme hooks are named theme_HOOK. Default
 * template implementations are stored in the module directory.
 *
 * Backdrop's default template renderer is a simple PHP parsing engine that
 * includes the template and stores the output. Backdrop's theme engines
 * can provide alternate template engines, such as XTemplate, Smarty and
 * PHPTal. The most common template engine is PHPTemplate (included with
 * Backdrop and implemented in phptemplate.engine, which uses Backdrop's default
 * template renderer.
 *
 * In order to create theme-specific implementations of these hooks, themes can
 * implement their own version of theme hooks, either as functions or templates.
 * These implementations will be used instead of the default implementation. If
 * using a pure .theme without an engine, the .theme is required to implement
 * its own version of hook_theme() to tell Backdrop what it is implementing;
 * themes utilizing an engine will have their well-named theme functions
 * automatically registered for them. While this can vary based upon the theme
 * engine, the standard set by phptemplate is that theme functions should be
 * named THEMENAME_HOOK. For example, for Backdrop's default theme (Bartik) to
 * implement the 'table' hook, the phptemplate.engine would find
 * bartik_table().
 *
 * The theme system is described and defined in theme.inc.
 *
 * @see theme()
 * @see hook_theme()
 * @see hooks
 * @see callbacks
 *
 * @} End of "defgroup themeable".
 */

/**
 * Allow themes to alter the theme-specific settings form.
 *
 * With this hook, themes can alter the theme-specific settings form in any way
 * allowable by Backdrop's Form API, such as adding form elements, changing
 * default values and removing form elements. See the Form API documentation on
 * api.drupal.org for detailed information.
 *
 * Note that the base theme's form alterations will be run before any sub-theme
 * alterations.
 *
 * @param $form
 *   Nested array of form elements that comprise the form.
 * @param $form_state
 *   A keyed array containing the current state of the form.
 */
function hook_form_system_theme_settings_alter(&$form, &$form_state) {
  // Add a checkbox to toggle the breadcrumb trail.
  $form['toggle_breadcrumb'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display the breadcrumb'),
    '#default_value' => theme_get_setting('toggle_breadcrumb'),
    '#description'   => t('Show a trail of links from the homepage to the current page.'),
  );
}

/**
 * Preprocess theme variables for templates.
 *
 * This hook allows modules to preprocess theme variables for theme templates.
 * It is called for all theme hooks implemented as templates, but not for theme
 * hooks implemented as functions. hook_preprocess_HOOK() can be used to
 * preprocess variables for a specific theme hook, whether implemented as a
 * template or function.
 *
 * For more detailed information, see theme().
 *
 * @param $variables
 *   The variables array (modify in place).
 * @param $hook
 *   The name of the theme hook.
 */
function hook_preprocess(&$variables, $hook) {
 static $hooks;

  // Add contextual links to the variables, if the user has permission.

  if (!user_access('access contextual links')) {
    return;
  }

  if (!isset($hooks)) {
    $hooks = theme_get_registry();
  }

  // Determine the primary theme function argument.
  if (isset($hooks[$hook]['variables'])) {
    $keys = array_keys($hooks[$hook]['variables']);
    $key = $keys[0];
  }
  else {
    $key = $hooks[$hook]['render element'];
  }

  if (isset($variables[$key])) {
    $element = $variables[$key];
  }

  if (isset($element) && is_array($element) && !empty($element['#contextual_links'])) {
    $variables['title_suffix']['contextual_links'] = contextual_links_view($element);
    if (!empty($variables['title_suffix']['contextual_links'])) {
      $variables['classes'][] = 'contextual-links-region';
    }
  }
}

/**
 * Preprocess theme variables for a specific theme hook.
 *
 * This hook allows modules to preprocess theme variables for a specific theme
 * hook. It should only be used if a module needs to override or add to the
 * theme preprocessing for a theme hook it didn't define.
 *
 * For more detailed information, see theme().
 *
 * @param $variables
 *   The variables array (modify in place).
 */
function hook_preprocess_HOOK(&$variables) {
  // This example adds an RDF attribute to the image hook's variables.
  $variables['attributes']['typeof'] = array('foaf:Image');
}

/**
 * Respond to themes being enabled.
 *
 * @param array $theme_list
 *   Array containing the names of the themes being enabled.
 *
 * @see theme_enable()
 */
function hook_themes_enabled($theme_list) {
  foreach ($theme_list as $theme) {
    mymodule_prepare_theme($theme);
  }
}

/**
 * Respond to themes being disabled.
 *
 * @param array $theme_list
 *   Array containing the names of the themes being disabled.
 *
 * @see theme_disable()
 */
function hook_themes_disabled($theme_list) {
 // Clear all update module caches.
  _update_cache_clear();
}
