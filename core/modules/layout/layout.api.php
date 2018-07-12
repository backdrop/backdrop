<?php
/**
 * @file
 * Describe hooks provided by the Layout module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Provides a list of layouts that can be used within the Layout module.
 *
 * This hook returns an array keyed by a unique identifier for a layout name.
 *
 * The contents of this hook are merged with layout information provided by
 * stand-alone layouts with their own .info files. Generally, the data returned
 * here matches the keys used within layout .info files.
 *
 * @return array
 *   Each item in the returned array of info should have the following keys:
 *   - title: The human-readable name of the layout.
 *   - path: A local path within the providing module to files needed by this
 *     layout, such as associated CSS, the icon image, and template file.
 *   - regions: A list of regions this layout provides, keyed by a machine name
 *     with a human label value.
 *   - preview: Optional. An image representing the appearance of this layout.
 *     If left empty, "preview.png" will be used.
 *   - stylesheets: An array of CSS file used whenever this layout is presented.
 *     If left empty, "one-column.css" will be used for all media types.
 *   - template: The name of the template file (without the extension) used for
 *     this layout. All layouts should always be named with a "layout--" prefix,
 *     so that the default variables may be provided in
 *     template_preprocess_layout(). If left empty, "layout--[key]" will be
 *     used, with underscores converted to hyphens in the layout key.
 *   - file: The name of a PHP file to be included prior to any rendering of
 *     this layout. This may be used to provide preprocess functions to prepare
 *     variables for the use of the layout.
 */
function hook_layout_info() {
  $layouts['my_layout'] = array(
    'title' => t('A custom layout'),
    'path' => 'layouts/my_layout',
    'regions' => array(
      'header' => t('Header'),
      'content' => t('Content'),
      'sidebar' => t('Sidebar'),
      'footer' => t('Footer'),
    ),

    // Optional information that populates using defaults.
    // 'preview' => 'preview.png',
    // 'stylesheets' => array('all' => array('one-column.css')),
    // 'template' => 'layout--my-layout',

    // Specify a file containing preprocess functions if needed.
    // 'file' => 'my_layout.php',
  );
  return $layouts;
}

/**
 * Provides a list of all "contexts" available to Layout module.
 *
 * A context is a named type of data, such as a "node" or "user". When creating
 * a new context through this hook, you are providing a mapping between certain
 * paths and a type of data. For example this hook may identify "node/%" as a
 * known path that maps to node data. Any path that starts with "node/%" will
 * automatically be assigned the node context, because its path is known.
 * Besides defining paths that map to a certain kind of data, this hook must
 * also specify how that content may be loaded.
 *
 * Each type of context requires a class that provides information about the
 * context. See the LayoutContext base class for additional documentation.

 * @return array
 *   Each item in the returned array of info should have the following keys:
 *   - title: The human-readable name of the context.
 *   - class: The name of a class to handle this context. This class should
 *     extend the LayoutContext class. The class should be registered in
 *     hook_autoload_info().
 *   - menu paths: Optional. An array of paths at which this context should be
 *     available. If left empty, this context can only be assigned through the
 *     UI by the user.
 *   - path placeholder: Optional. A string identifying the part of the URL from
 *     the menu paths array that contains this context's argument. This is only
 *     necessary if menu paths are also provided.
 *   - load callback: The name of a function that will load the argument from
 *     the URL and return the loaded data.
 *   - hidden: Optional. Boolean if this context should be shown in the UI.
 *
 * @see hook_autoload_info()
 * @see layout_layout_context_info()
 * @see LayoutContext
 */
function hook_layout_context_info() {
  $info['node'] = array(
    'title' => t('Node'),
    // Define the class which is used to handle this context.
    'class' => 'EntityLayoutContext',
    // Define menu paths where the node ID is a "known" context.
    'menu paths' => array(
      'node/%node',
      'node/%node/view',
    ),
    // Given the menu paths defined above, identify the part of the path that
    // is needed to generate this context.
    'path placeholder' => '%node',

    // Given an argument, the callback that will be responsible for loading the
    // main context data.
    'load callback' => 'node_load',
  );
  return $info;
}

/**
 * Provides information on rendering styles that can be used by layouts.
 *
 * This hook provides a list of styles that can be used both by regions and
 * individual blocks. A style can modify the output of a layout if needed for
 * specialized presentation.
 *
 * @return array
 *   An info array keyed by a unique machine name for that style. Possible keys
 *   include:
 *   - title: The translated title of the style.
 *   - description: The translated description of the style.
 *   - block theme: Optional. If this style modifies the display of blocks,
 *     specify the theme function or template key that would be passed into the
 *     theme function.
 *   - class: Optional. The name of a class which contains any advanced methods
 *     to configure and save settings for this display style. If not specified,
 *     the default class of LayoutStyle will be used. Each class must also be
 *     registered in hook_autoload_info().
 *   - file: Optional. The name of the file the implementation resides in. This
 *     file path is NOT the path to the class. Class paths and loading is done
 *     through hook_autoload_info().
 *   - path: Optional. Override the path to the file to be used. Ordinarily
 *     theme functions are located in a file in the module path (for example:
 *     mymodule/mymodule.theme.inc) and template files are located in a
 *     subdirectory named templates (for example: mymodule/templates/), but if
 *     your file will not be in the default location, include it here. This
 *     path should be relative to the Backdrop root directory.
 *   - template: If specified, this theme implementation is a template, and
 *     this is the template file name without an extension. Do not include the
 *     extension .tpl.php; it will be added automatically. If 'path' is
 *     specified, then the template should be located in this path.
 *   - hook theme: Optional. If specified, additional information to be merged
 *     into hook_theme() on behalf of this style. This may be necessary if the
 *     values provided for "block theme" have not already been registered.
 *
 * @see LayoutStyle
 * @see hook_autoload_info()
 * @see layout_layout_style_info()
 */
function hook_layout_style_info() {
  $info['custom_style'] = array(
    'title' => t('A new style'),
    'description' => t('An advanced style with settings.'),
    // The theme key for rendering an individual block.
    'block theme' => 'mymodule_block',
    // Provide a class name if this style has settings. The class should extend
    // the LayoutStyle class.
    'class' => 'MyModuleLayoutStyle',
    // Override the path to the file to be used.
    'path' => 'templates/subdir',
    // Name of template file (with or without path).
    'template' => 'templates/my-filename',
  );
  return $info;
}

/**
 * Returns information about Layout renderers.
 *
 * Layout renderers are classes which are responsible for processing a $layout
 * object and rendering it as HTML. By default, Layout module provides two
 * renderers, one for the front-end display and one for the administration of
 * a layout. Alternative renderers could be provided to provide data in
 * different formats, provide alternative UIs, or return specific blocks rather
 * than the entire layout.
 *
 * @return array
 *   Each item in the returned array of info should have the following keys:
 *   - class: The name of a class that provides the renderer. Classes usually
 *     extend the LayoutStandard class.
 *
 * @see hook_autoload_info()
 * @see layout_layout_renderer_info()
 */
function hook_layout_renderer_info() {
  $info['my_renderer'] = array(
    'class' => 'MyModuleLayoutRenderer',
  );

  return $info;
}

/**
 * Respond to a layout being reverted.
 *
 * Layouts can be reverted only if the configuration is provided
 * by a module. Layouts created in the Layout Builder User Interface
 * cannot be reverted.
 * A layout revert operation results in deletion of the existing
 * layout configuration and replacement with the default configuration
 * from the providing module.
 * This hook is invoked from Layout::revert() after a layout has been
 * reverted and the new configuration has been inserted into the live
 * config directory and the layout cache has been cleared.
 *
 * @param Layout $layout
 *   The old layout object that has just been deleted.
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_revert(Layout $old_layout) {
  if ($old_layout->name == 'my_layout') {
    my_custom_function();
  }
  // Get the new (reverted) configuration.
  $new_config = config_get('layout.layout.' . $old_layout->name);
}

/**
 * Respond to a layout being deleted.
 *
 * This hook is invoked from Layout::delete() after a layout has been
 * deleted.
 *
 * @param Layout $layout
 *   The layout object that was deleted.
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_delete(Layout $layout) {
  if ($layout->getPath() == 'my_path') {
    my_custom_function();
  }
}

/**
 * Respond to a layout being enabled.
 *
 * This hook is invoked from Layout::enable() after a layout has been
 * enabled.
 *
 * @param Layout $layout
 *   The layout object that was enabled.
 *
 * @see hook_layout_disable()
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_enable(Layout $layout) {
  if ($layout->getPath() == 'my_path') {
    my_custom_function();
  }
}

/**
 * Respond to a layout being disabled.
 *
 * This hook is invoked from Layout::disable() after a layout has been
 * disabled.
 * A layout configuration may be disabled by a user from the administrative
 * list of layouts. A disabled layout will not affect pages at its configured
 * path, but will retain its configuration so that it may be enabled later.
 *
 * @param Layout $layout
 *   The layout object that was disabled.
 *
 * @see hook_layout_enable()
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_disable(Layout $layout) {
  if ($layout->getPath() == 'my_path') {
    my_custom_function();
  }
}

/**
 * Respond to updates to a layout.
 *
 * This hook is invoked from Layout::save() after a layout has been saved
 * to configuration.
 *
 * @param Layout $layout
 *   The layout object that was saved.
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_update(Layout $layout) {
  if ($layout->getPath() == 'my_path') {
    my_custom_function();
  }
}

/**
 * Respond to initial creation of a layout.
 *
 * This hook is invoked from Layout::save() after a layout has been saved
 * to configuration.
 *
 * @param Layout $layout
 *   The layout object that was saved.
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_insert(Layout $layout) {
  if ($layout->getPath() == 'my_path') {
    my_custom_function();
  }
}

/**
 * Act on a layout being inserted or updated.
 *
 * This hook is invoked from Layout::save() before the layout is saved to
 * configuration.
 *
 * @param Layout $layout
 *   The layout that is being inserted or updated.
 *
 * @ingroup layout_api_hooks
 */
function hook_layout_presave(Layout $layout) {
  if ($layout->name == 'default') {
    $my_block_uuid = get_my_uuid();
    $my_block = $layout->content[$my_block_uuid];
    $my_block->data['settings']['title'] = 'New Title';
  }
}

/**
 * @} End of "addtogroup hooks".
 */
