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
 *
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
 *     the URL and return the loaded data. The loaded data must be an object,
 *     not a string, array, or other variable type.
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
 *     my_module/my_module.theme.inc) and template files are located in a
 *     subdirectory named templates (for example: my_module/templates/), but if
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
    'block theme' => 'my_module_block',
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
 * Perform alterations to the list of layouts that match the path of a router
 * item.
 *
 * Modules can implement this hook to allow a given layout to be used for
 * multiple or different paths than its defined path.
 *
 * This hook is called on every page load; your implementation should quickly
 * and efficiently determine if it is applicable and return if not.
 *
 * If a layout's path includes positioned contexts (e.g., 'node/%') and you
 * apply the layout to a different path, you may need to set the context
 * data beyond the default setting done by the layout module. The default
 * setting happens after invocations of this hook, taking care of any contexts
 * that were not yet set.
 *
 * If you substitute a layout whose placeholder(s) are in a different position
 * from the path in the router item and you will rely on the system to set the
 * context, you should set the position of the context(s) in the layout to the
 * position in the router. Example: the path in the router item is
 * my_module/node/% and you wish to use the layout with path node/%. Then you
 * would find the context at position 2 in the router item and set its
 *  Context::position variable to 2, so that the layout with path node/% looks
 * at position 2 to set the context for that placeholder.
 *
 * @param Layout[] $layouts
 *   The list of layouts that should be considered for the path
 *   $router_item['path']. Note that the final selection of layout will be based
 *   on contexts and visibility conditions.
 * @param array $router_item
 *   The router item for the path. In addition to obtaining the system path at
 *   $router_item['path'], you can use other elements of $router_item. For
 *   example, $router_item['original_map'] contains an exploded version of the
 *   normal path.
 *
 * @since 1.20.0
 *
 * @see layout_get_layout_by_path()
 * @see node_layout_load_by_router_item_alter()
 */
function hook_layout_load_by_router_item_alter(&$layouts, $router_item) {
  // Example taken from node_layout_load_by_router_item_alter(). When creating
  // node preview pages, the path will be of the form node/preview/<type>/<id>,
  // where <type> is the node type and <id> is the tempstore ID of the node
  // begin previewed. But we want to display it using a layout whose system path
  // is node/%. So we choose those layouts and set the context from the
  // tempstore node.

  // Check path structure before checking node type because
  // node_type_get_types() is expensive; don't call it if we don't need to.
  $map = $router_item['map'];
  if (count($map) != 4 || $map[0] != 'node' || $map[1] != 'preview') {
    return;
  }
  $path_is_preview = FALSE;
  foreach (node_type_get_types() as $type_obj) {
    if ($type_obj->type == $map[2]) {
      $path_is_preview = TRUE;
      break;
    }
  }
  if (!$path_is_preview) {
    return;
  }
  // So now we know that this is a path that we want to handle. We want to use
  // layouts whose path is node/%, and we'll use the node from the tempstore to
  // set the context of those layouts. So start by getting the tempstore node,
  // whose ID is in position 3 of the path.
  $tempstore_id = $map[3];
  $temp_node = node_get_node_tempstore($tempstore_id);
  if ($temp_node) {
    // Get all the layouts that we want to use whose paths are those normally
    // used to display a node.
    $node_layouts = layout_load_multiple_by_path('node/%', TRUE);
    if ($node_layouts) {
      $layouts = $node_layouts;
      foreach ($layouts as $layout) {
        foreach ($layout->getContexts() as $context) {
          if (isset($context->position)) {
            // If we were relying on the system to set the context, we should
            // change $context->position to be the place the data is found in
            // the router item, as here. This isn't actually necessary here,
            // because we're going to go ahead and set the data ourselves, since
            // we need to use the tempstore node as the context data.
            $context->position = 3;
            $context->setData($temp_node);
          }
        }
      }
    }
  }
}

/**
 * Defines to Backdrop what blocks are provided by your module.
 *
 * In hook_block_info(), each block your module provides is given a unique
 * identifier referred to as "delta" (the array key in the return value). Delta
 * values only need to be unique within your module, and they are used in the
 * following ways:
 * - Passed into the other block hooks in your module as an argument to identify
 *   the block being configured or viewed.
 * - Used to construct the default HTML ID of "block-MODULE-DELTA" applied to
 *   each block when it is rendered. This ID may then be used for CSS styling or
 *   JavaScript programming.
 * - Used to define a theme template suggestion of block__MODULE__DELTA, for
 *   advanced theme possibilities.
 * - Used by other modules to identify your block in hook_block_info_alter() and
 *   other alter hooks.
 * The values of delta can be strings or numbers, but because of the uses above
 * it is preferable to use descriptive strings whenever possible, and only use a
 * numeric identifier if you have to (for instance if your module allows users
 * to create several similar blocks that you identify within your module code
 * with numeric IDs). The maximum length for delta values is 32 bytes.
 *
 * @return
 *   An associative array whose keys define the delta for each block and whose
 *   values contain the block descriptions. Each block description is itself an
 *   associative array, with the following key-value pairs:
 *   - info: (required) The human-readable administrative name of the block.
 *     This is used to identify the block on administration screens, and
 *     is not displayed to non-administrative users.
 *   - description: (optional) A human-readable administrative description of
 *     the block. Although intended to be longer than "info", it should still
 *     be no longer than a short sentence or two.
 *   - required contexts: (optional) An array of contexts that this block
 *     requires to display. These contexts are keyed by their internal name that
 *     this block will receive as the key in the $context parameter of
 *     hook_block_view(). The value of each item should be the type of context,
 *     as listed by hook_layout_context_info().
 *   - class: (optional) A class that provides the settings form, save routine,
 *     and display of this block. If specified, the class will be used instead
 *     of the hooks for hook_block_configure(), hook_block_save(), and
 *     hook_block_view(). This class should be a sub-class of the Block class.
 *
 * For a detailed usage example, see block_example.module.
 *
 * @see hook_block_configure()
 * @see hook_block_save()
 * @see hook_block_view()
 * @see hook_block_info_alter()
 */
function hook_block_info() {
  $blocks['syndicate'] = array(
    'info' => t('Syndicate'),
    'description' => t('An RSS icon linking to the feed for the current page (if any).'),
  );

  $blocks['recent'] = array(
    'info' => t('Recent content'),
    'description' => t('A list of recently published content.'),
  );

  $blocks['author_picture'] = array(
    'info' => t('Author picture'),
    'description' => t('The user picture for the current content author.'),
    // A context of type "node" (the value here) will be given the key "node"
    // (specified as the key here) in hook_block_view() in the $context
    // parameter.
    'required contexts' => array('node' => 'node'),
  );

  $blocks['my_node_field'] = array(
    'info' => t('My node field'),
    'description' => t('An arbitrary field from a node.'),
    // Instead of using hook_block_view(), use a class that sub-classes the
    // Block class. Note that this class would also need to be registered in
    // hook_autoload_info().
    'class' => 'MyNodeFieldBlock',
    'required contexts' => array('node' => 'node'),
  );

  return $blocks;
}

/**
 * Modify block definitions after loading form code.
 *
 * @param $blocks
 *   A multidimensional array of blocks keyed by the defining module and delta;
 *   the values are blocks returned by hook_block_info().
 *
 * @see hook_block_info()
 */
function hook_block_info_alter(&$blocks) {
  // Always check that the block is in the list before attempting to alter it.
  if (isset($blocks['user']['login'])) {
    // Change the administrative title the user login block.
    $blocks['user']['login']['info'] = t('Login form');
  }
}

/**
 * Define a configuration form for a block.
 *
 * @param string $delta
 *   Which block is being configured. This is a unique identifier for the block
 *   within the module, defined in hook_block_info().
 * @param array $settings
 *   An array of settings for this block.
 *
 * @return array
 *   A configuration form, if one is needed for your block beyond the standard
 *   elements that the block module provides (block title, visibility, etc.).
 *
 * For a detailed usage example, see block_example.module.
 *
 * @see hook_block_info()
 * @see hook_block_save()
 *
 * @since 1.0.6 $settings parameter added.
 */
function hook_block_configure($delta = '', $settings = array()) {
  // This example comes from node.module.
  $form = array();
  if ($delta == 'recent') {
    $settings += array(
      'node_count' => 10,
    );
    $form['node_count'] = array(
      '#type' => 'select',
      '#title' => t('Number of recent content items to display'),
      '#default_value' => $settings['node_count'],
      '#options' => range(2, 30),
    );
  }
  return $form;
}

/**
 * Save the configuration options from hook_block_configure().
 *
 * This hook allows you to save the block-specific configuration settings
 * defined within your hook_block_configure(). Most settings will automatically
 * be saved if using the Layout module to position blocks, but if your settings
 * should be site-wide or saved in the database instead of the Layout
 * configuration, you may use this hook to save your settings. If you wish
 * Layout module to *not* save your settings, unset the value from the $edit
 * array.
 *
 * @param $delta
 *   Which block is being configured. This is a unique identifier for the block
 *   within the module, defined in hook_block_info().
 * @param $edit
 *   The submitted form data from the configuration form. This is passed by
 *   reference, so values can be changed or removed before they are saved into
 *   the layout configuration.
 *
 * @see hook_block_configure()
 * @see hook_block_info()
 */
function hook_block_save($delta, &$edit = array()) {
  if ($delta == 'my_block_delta') {
    config_set('my_module.settings', 'my_global_value', $edit['my_global_value']);
    // Remove the value so it is not saved by Layout module.
    unset($edit['my_global_value']);
  }
}

/**
 * Return a rendered or renderable view of a block.
 *
 * @param string $delta
 *   Which block to render. This is a unique identifier for the block
 *   within the module, defined in hook_block_info().
 * @param array $settings
 *   An array of settings for this block. Defaults may not be populated, so it's
 *   best practice to merge in defaults within hook_block_view().
 * @param array $contexts
 *   An array of contexts required by this block. Each context will be keyed
 *   by the string specified in this module's hook_block_info().
 *
 * @return
 *   Either an empty array so the block will not be shown or an array containing
 *   the following elements:
 *   - subject: The default localized title of the block. If the block does not
 *     have a default title, this should be set to NULL.
 *   - content: The content of the block's body. This may be a renderable array
 *     (preferable) or a string containing rendered HTML content. If the content
 *     is empty the block will not be shown.
 *
 * For a detailed usage example, see block_example.module.
 *
 * @see hook_block_info()
 * @see hook_block_view_alter()
 * @see hook_block_view_MODULE_DELTA_alter()
 */
function hook_block_view($delta = '', $settings = array(), $contexts = array()) {
  // This example is adapted from node.module.
  $block = array();

  switch ($delta) {
    case 'syndicate':
      $block['subject'] = t('Syndicate');
      $block['content'] = array(
        '#theme' => 'feed_icon',
        '#url' => 'rss.xml',
        '#title' => t('Syndicate'),
      );
      break;

    case 'recent':
      if (user_access('access content')) {
        $settings += array(
          'node_count' => 10,
        );
        $block['subject'] = t('Recent content');
        if ($nodes = node_get_recent($settings['node_count'])) {
          $block['content'] = array(
            '#theme' => 'node_recent_block',
            '#nodes' => $nodes,
          );
        }
        else {
          $block['content'] = t('No content available.');
        }
      }
      break;

    case 'author_picture':
      $author_account = user_load($contexts['node']->uid);
      $block['subject'] = '';
      $block['content'] = theme('user_picture', array('account' => $author_account));
      return $block;
  }
  return $block;
}

/**
 * Perform alterations to the content of a block.
 *
 * This hook allows you to modify blocks before they are rendered.
 *
 * Note that instead of hook_block_view_alter(), which is called for all
 * blocks, you can also use hook_block_view_MODULE_DELTA_alter() to alter a
 * specific block.
 *
 * @param $data
 *   The block title and content as returned by the module that defined the
 *   block. This could be an empty array or NULL value (if the block is empty)
 *   or an array containing the following:
 *   - title: The (localized) title of the block.
 *   - content: Either a string or a renderable array representing the content
 *     of the block. You should check that the content is an array before trying
 *     to modify parts of the renderable structure.
 * @param $block
 *   The Block object. It will have have at least the following properties:
 *   - module: The name of the module that defined the block.
 *   - delta: The unique identifier for the block within that module, as defined
 *     in hook_block_info().
 *   - settings: An array containing all block settings as defined for this
 *     instance of the block.
 *   - contexts: An array containing all layout contexts available for the
 *     layout.
 *
 * @see hook_block_view_MODULE_DELTA_alter()
 * @see hook_block_view()
 */
function hook_block_view_alter(&$data, $block) {
  // Remove the contextual links on all blocks that provide them.
  if (is_array($data['content']) && isset($data['content']['#contextual_links'])) {
    unset($data['content']['#contextual_links']);
  }
  // Add a theme wrapper function defined by the current module to all blocks
  // provided by the "some_module" module.
  if (is_array($data['content']) && $block->module == 'some_module') {
    $data['content']['#theme_wrappers'][] = 'my_module_special_block';
  }
}

/**
 * Perform alterations to a specific block.
 *
 * Modules can implement hook_block_view_MODULE_DELTA_alter() to modify a
 * specific block, rather than implementing hook_block_view_alter().
 *
 * @param $data
 *   The data as returned from the hook_block_view() implementation of the
 *   module that defined the block. This could be an empty array or NULL value
 *   (if the block is empty) or an array containing:
 *   - title: The localized title of the block.
 *   - content: Either a string or a renderable array representing the content
 *     of the block. You should check that the content is an array before trying
 *     to modify parts of the renderable structure.
 * @param $block
 *   The block object, as loaded from the database, having the main properties:
 *   - module: The name of the module that defined the block.
 *   - delta: The unique identifier for the block within that module, as defined
 *     in hook_block_info().
 *   - settings: An array containing all block settings as defined for this
 *     instance of the block.
 *   - contexts: An array containing all layout contexts available for the
 *     layout.
 *
 * @see hook_block_view_alter()
 * @see hook_block_view()
 */
function hook_block_view_MODULE_DELTA_alter(&$data, $block) {
  // This code will only run for a specific block. For example, if MODULE_DELTA
  // in the function definition above is set to "my_module_some_delta", the code
  // will only run on the "some_delta" block provided by the "my_module" module.
  //
  // Change the title of the "some_delta" block provided by the "my_module"
  // module.
  $data['title'] = t('New title of the block');
}

/**
 * @} End of "addtogroup hooks".
 */
