<?php

/**
 * @file
 * Hooks provided by the Views module.
 */

/**
 * @mainpage Views 3 API Manual
 *
 * Much of this information is actually stored in the advanced help; please
 * check the API topic. This help will primarily be aimed at documenting
 * classes and function calls.
 *
 * Topics:
 * - @link views_lifetime The life of a view @endlink
 * - @link views_hooks Views hooks @endlink
 * - @link views_handlers About Views handlers @endlink
 * - @link views_plugins About Views plugins @endlink
 * - @link views_templates Views template files @endlink
 * - @link views_module_handlers Views module handlers @endlink
 */

/**
 * @defgroup views_lifetime The life of a view
 * @{
 * This page explains the basic cycle of a view and what processes happen.
 *
 * @todo.
 * @}
 */

/**
 * @defgroup views_handlers About Views handlers
 * @{
 * In Views, a handler is an object that is part of the view and is part of the
 * query building flow.
 *
 * Handlers are objects; much of the time, the base handlers will work, but
 * often you'll need to override the handler for something. One typical handler
 * override will be views_handler_filter_operator_in which allows you to have a
 * filter select from a list of options; you'll need to override this to provide
 * your list.
 *
 * Handlers have two distint code flows; the UI flow and the view building flow.
 *
 * For the query flow:
 * - handler->construct()
 *   - Create the initial handler; at this time it is not yet attached to a
 *     view. It is here that you can set basic defaults if needed, but there
 *     will be no knowledge of the environment yet.
 * - handler->set_definition()
 *   - Set the data from hook_views_data() relevant to the handler.
 * - handler->init()
 *   - Attach the handler to a view, and usually provides the options from the
 *     display.
 * - handler->pre_query()
 *   - Run prior to the query() stage to do early processing.
 * - handler->query()
 *   - Do the bulk of the work this handler needs to do to add itself to the
 *     query.
 *
 * Fields, being the only handlers concerned with output, also have an extended
 * piece of the flow:
 *
 * - handler->pre_render(&$values)
 *   - Called prior to the actual rendering, this allows handlers to query for
 *     extra data; the entire resultset is available here, and this is where
 *     items that have "multiple values" per record can do their extra query for
 *     all of the records available. There are several examples of this at work
 *     in the code.
 * - handler->render()
 *   - This does the actual work of rendering the field.
 *
 * Most handlers are just extensions of existing classes with a few tweaks that
 * are specific to the field in question. For example:
 *
 * @code
 * class views_handler_filter_node_type extends views_handler_filter_in_operator {
 *   function get_value_options() {
 *     if (!isset($this-&gt;value_options)) {
 *       $this-&gt;value_title = t('Node type');
 *       $types = node_get_types();
 *       foreach ($types as $type => $info) {
 *         $options[$type] = $info-&gt;name;
 *       }
 *       $this-&gt;value_options = $options;
 *     }
 *   }
 * }
 * @endcode
 *
 * <i>views_handler_filter_in_operator</i> provides a simple mechanism to set
 * the list used and the rest of the handler is perfectly fine for this.
 *
 * Handlers are stored in their own files and loaded on demand.
 * Like all other module files, they must first be registered through the
 * module's info file. For example:
 *
 * @code
 * name = Example module
 * description = "Gives an example of a module."
 * core = 7.x
 * files[] = example.module
 * files[] = example.install
 *
 * ; Views handlers
 * files[] = includes/views/handlers/example_handler_argument_string.inc
 * @endcode
 *
 * The best place to learn more about handlers and how they work is to explore
 * @link views_handlers Views' handlers @endlink and use existing handlers as a
 * guide and a model. Understanding how views_handler and its child classes work
 * is handy but you can do a lot just following these models. You can also
 * explore the views module directory, particularly node.views.inc.
 *
 * Please note that while all handler names in views are prefixed with views_,
 * you should use your own module's name to prefix your handler names in order
 * to ensure namespace safety. Note that the basic pattern for handler naming
 * goes like this:
 *
 * [module]_handler_[type]_[tablename]_[fieldname].
 *
 * Sometimes table and fieldname are not appropriate, but something that
 * resembles what the table/field would be can be used.
 *
 * See also:
 * - @link views_field_handlers Views field handlers @endlink
 * - @link views_sort_handlers Views sort handlers @endlink
 * - @link views_filter_handlers Views filter handlers @endlink
 * - @link views_argument_handlers Views argument handlers @endlink
 * - @link views_relationship_handlers Views relationship handlers @endlink
 * @}
 */

/**
 * @defgroup views_plugins About Views plugins
 *
 * In Views, a plugin is a bit like a handler, but plugins are not directly
 * responsible for building the query. Instead, they are objects that are used
 * to display the view or make other modifications.
 *
 * There are 10 types of plugins in Views:
 * - Display
 *   - Display plugins are responsible for controlling <strong>where</strong> a
 *     view lives. Page and block are the most common displays, as well as the
 *     ubiquitous 'default' display which is likely what will be embedded.
 * - Style
 *   - Style plugins control how a view is displayed. For the most part they are
 *     object wrappers around theme templates.
 * - Row style
 *   - Row styles handle each individual record from a node.
 * - Argument default
 *   - Argument default plugins allow pluggable ways of providing arguments for
 *     blocks. Views includes plugins to extract node and user IDs from the URL;
 *     additional plugins could be used for a wide variety of tasks.
 * - Argument validator
 *   - Validator plugins can ensure arguments are valid, and even do
 *     transformations on the arguments.
 * - Access
 *   - Access plugins are responsible for controlling access to the view.
 * - Query
 *   - Query plugins generate and execute a query, it can be seen as a data
 *     backend. The default implementation is using sql.
 * - Cache
 *   - Cache plugins control the storage and loading of caches. Currently they
 *     can do both result and render caching, but maybe one day cache the
 *     generated query
 * - Pager plugins
 *   - Pager plugins take care of everything regarding pagers. From getting and
 *     setting the total amount of items to render the pager and setting the
 *     global pager arrays.
 * - Exposed form plugins
 *   - Exposed form plugins are responsible for building, rendering and
 *     controlling exposed forms. They can expose new parts of the view to the
 *     user and more.
 * - Localization plugins
 *   - Localization plugins take care how the view options are translated. There
 *     are example implementations for t(), none translation and i18n.
 *
 * Plugins are registered by implementing <strong>hook_views_plugins()</strong
 * in your modulename.views.inc file and returning an array of data.
 * For examples please look at views_views_plugins() in
 * views/includes/plugins.inc as it has examples for all of them.
 *
 * For example plugins please look at the one provided by views, too.
 *
 * Similar to handlers take sure that you added the plugin file to the
 * module.info.
 *
 * The array will look something like this:
 * @code
 * return array(
 *   'display' => array(
 *     // ... list of display plugins,
 *    ),
 *   'style' => array(
 *     // ... list of style plugins,
 *    ),
 *   'row' => array(
 *     // ... list of row style plugins,
 *    ),
 *   'argument default' => array(
 *     // ... list of argument default plugins,
 *    ),
 *   'argument validator' => array(
 *     // ... list of argument validator plugins,
 *    ),
 *    'access' => array(
 *     // ... list of access plugins,
 *    ),
 *    'query' => array(
 *      // ... list of query plugins,
 *     ),,
 *    'cache' => array(
 *      // ... list of cache plugins,
 *     ),,
 *    'pager' => array(
 *      // ... list of pager plugins,
 *     ),,
 *    'exposed_form' => array(
 *      // ... list of exposed_form plugins,
 *     ),,
 *    'localization' => array(
 *      // ... list of localization plugins,
 *     ),
 * );
 * @endcode
 *
 * Each plugin will be registered with an identifier for the plugin, plus a
 * fairly lengthy list of items that can define how and where the plugin is
 * used. Here is an example from Views core:
 *
 * @code
 *     'node' => array(
 *       'title' => t('Node'),
 *       'help' => t('Display the node with standard node view.'),
 *       'handler' => 'views_plugin_row_node_view',
 *       'path' => drupal_get_path('module', 'views') . '/modules/node', // not necessary for most modules
 *       'theme' => 'views_view_row_node',
 *       'base' => array('node'), // only works with 'node' as base.
 *       'uses options' => TRUE,
 *       'type' => 'normal',
 *     ),
 * @endcode
 *
 * Of particular interest is the <em>path</em> directive, which works a little
 * differently from handler registration; each plugin must define its own path,
 * rather than relying on a global info for the paths. For example:
 *
 * @code
 *    'feed' => array(
 *       'title' => t('Feed'),
 *       'help' => t('Display the view as a feed, such as an RSS feed.'),
 *       'handler' => 'views_plugin_display_feed',
 *       'uses hook menu' => TRUE,
 *       'use ajax' => FALSE,
 *       'use pager' => FALSE,
 *       'accept attachments' => FALSE,
 *       'admin' => t('Feed'),
 *       'help topic' => 'display-feed',
 *     ),
 * @endcode
 *
 * Please be sure to prefix your plugin identifiers with your module name to
 * ensure namespace safety; after all, two different modules could try to
 * implement the 'grid2' plugin, and that would cause one plugin to completely
 * fail.
 *
 * @todo Finish this document.
 *
 * See also:
 * - @link views_display_plugins Views display plugins @endlink
 * - @link views_style_plugins Views style plugins @endlink
 * - @link views_row_plugins Views row plugins @endlink
 */

/**
 * @defgroup views_hooks Views hooks
 * @{
 * Hooks that can be implemented by other modules in order to implement the
 * Views API.
 */

/**
 * Describe table structure to Views.
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * MODULENAME.views.inc must be in the directory specified by the 'path' key
 * returned by MODULENAME_views_api(), or the same directory as the .module
 * file, if 'path' is unspecified.
 *
 * The full documentation for this hook is in the advanced help.
 * @link http://views-help.doc.logrus.com/help/views/api-tables @endlink
 */
function hook_views_data() {
  // This example describes how to write hook_views_data() for the following
  // table:
  //
  // CREATE TABLE example_table (
  //   nid INT(11) NOT NULL         COMMENT 'Primary key; refers to {node}.nid.',
  //   plain_text_field VARCHAR(32) COMMENT 'Just a plain text field.',
  //   numeric_field INT(11)        COMMENT 'Just a numeric field.',
  //   boolean_field INT(1)         COMMENT 'Just an on/off field.',
  //   timestamp_field INT(8)       COMMENT 'Just a timestamp field.',
  //   PRIMARY KEY(nid)
  // );

  // The 'group' index will be used as a prefix in the UI for any of this
  // table's fields, sort criteria, etc. so it's easy to tell where they came
  // from.
  $data['example_table']['table']['group'] = t('Example table');

  // Define this as a base table. In reality this is not very useful for
  // this table, as it isn't really a distinct object of its own, but
  // it makes a good example.
  $data['example_table']['table']['base'] = array(
    'field' => 'nid',
    'title' => t('Example table'),
    'help' => t("Example table contains example content and can be related to nodes."),
    'weight' => -10,
  );

  // This table references the {node} table.
  // This creates an 'implicit' relationship to the node table, so that when 'Node'
  // is the base table, the fields are automatically available.
  $data['example_table']['table']['join'] = array(
    // Index this array by the table name to which this table refers.
    // 'left_field' is the primary key in the referenced table.
    // 'field' is the foreign key in this table.
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );

  // Next, describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Node ID field.
  $data['example_table']['nid'] = array(
    'title' => t('Example content'),
    'help' => t('Some example content that references a node.'),
    // Because this is a foreign key to the {node} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
    'relationship' => array(
      'base' => 'node',
      'field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Example node'),
    ),
  );

  // Example plain text field.
  $data['example_table']['plain_text_field'] = array(
    'title' => t('Plain text field'),
    'help' => t('Just a plain text field.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // Example numeric text field.
  $data['example_table']['numeric_field'] = array(
    'title' => t('Numeric field'),
    'help' => t('Just a numeric field.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Example boolean field.
  $data['example_table']['boolean_field'] = array(
    'title' => t('Boolean field'),
    'help' => t('Just an on/off field.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
      // use boolean_field = 1 instead of boolean_field <> 0 in WHERE statment
      'use equal' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Example timestamp field.
  $data['example_table']['timestamp_field'] = array(
    'title' => t('Timestamp field'),
    'help' => t('Just a timestamp field.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  return $data;
}

/**
 * Alter table structure.
 *
 * You can add/edit/remove to existing tables defined by hook_views_data().
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * MODULENAME.views.inc must be in the directory specified by the 'path' key
 * returned by MODULENAME_views_api(), or the same directory as the .module
 * file, if 'path' is unspecified.
 *
 * The full documentation for this hook is in the advanced help.
 * @link http://views-help.doc.logrus.com/help/views/api-tables @endlink
 */
function hook_views_data_alter(&$data) {
  // This example alters the title of the node: nid field for the admin.
  $data['node']['nid']['title'] = t('Node-Nid');

  // This example adds a example field to the users table
  $data['users']['example_field'] = array(
    'title' => t('Example field'),
    'help' => t('Some examüple content that references a user'),
    'handler' => 'hook_handlers_field_example_field',
  );

  // This example changes the handler of the node title field.
  // In this handler you could do stuff, like preview of the node, when clicking the node title.

  $data['node']['title']['handler'] = 'modulename_handlers_field_node_title';
}


/**
 * The full documentation for this hook is now in the advanced help.
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * MODULENAME.views.inc must be in the directory specified by the 'path' key
 * returned by MODULENAME_views_api(), or the same directory as the .module
 * file, if 'path' is unspecified.
 *
 * This is a stub list as a reminder that this needs to be doc'd and is not used
 * in views anywhere so might not be remembered when this is formally documented:
 * - style: 'even empty'
 */
function hook_views_plugins() {
  // example code here
}

/**
 * Alter existing plugins data, defined by modules.
 */
function hook_views_plugins_alter(&$plugins) {
  // Add apachesolr to the base of the node row plugin.
  $plugins['row']['node']['base'][] = 'apachesolr';
}

/**
 * Register View API information. This is required for your module to have
 * its include files loaded; for example, when implementing
 * hook_views_default_views().
 *
 * @return
 *   An array with the following possible keys:
 *   - api:  (required) The version of the Views API the module implements.
 *   - path: (optional) If includes are stored somewhere other than within
 *       the root module directory, specify its path here.
 *   - template path: (optional) A path where the module has stored it's views template files.
 *        When you have specificed this key views automatically uses the template files for the views.
 *        You can use the same naming conventions like for normal views template files.
 */
function hook_views_api() {
  return array(
    'api' => 3,
    'path' => drupal_get_path('module', 'example') . '/includes/views',
    'template path' => drupal_get_path('module', 'example') . 'themes',
  );
}

/**
 * This hook allows modules to provide their own views which can either be used
 * as-is or as a "starter" for users to build from.
 *
 * This hook should be placed in MODULENAME.views_default.inc and it will be
 * auto-loaded. MODULENAME.views_default.inc must be in the directory specified
 * by the 'path' key returned by MODULENAME_views_api(), or the same directory
 * as the .module file, if 'path' is unspecified.
 *
 * The $view->disabled boolean flag indicates whether the View should be
 * enabled or disabled by default.
 *
 * @return
 *   An associative array containing the structures of views, as generated from
 *   the Export tab, keyed by the view name. A best practice is to go through
 *   and add t() to all title and label strings, with the exception of menu
 *   strings.
 */
function hook_views_default_views() {
  // Begin copy and paste of output from the Export tab of a view.
  $view = new view;
  $view->name = 'frontpage';
  $view->description = t('Emulates the default Drupal front page; you may set the default home page path to this view to make it your front page.');
  $view->tag = t('default');
  $view->base_table = 'node';
  $view->api_version = 2;
  $view->disabled = FALSE; // Edit this to true to make a default view disabled initially
  $view->display = array();
    $display = new views_display;
    $display->id = 'default';
    $display->display_title = t('Master');
    $display->display_plugin = 'default';
    $display->position = '1';
    $display->display_options = array (
    'style_plugin' => 'default',
    'style_options' =>
    array (
    ),
    'row_plugin' => 'node',
    'row_options' =>
    array (
      'teaser' => 1,
      'links' => 1,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
      'sticky' =>
      array (
        'id' => 'sticky',
        'table' => 'node',
        'field' => 'sticky',
        'order' => 'ASC',
      ),
      'created' =>
      array (
        'id' => 'created',
        'table' => 'node',
        'field' => 'created',
        'order' => 'ASC',
        'relationship' => 'none',
        'granularity' => 'second',
      ),
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
      'promote' =>
      array (
        'id' => 'promote',
        'table' => 'node',
        'field' => 'promote',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
      ),
      'status' =>
      array (
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
      ),
    ),
    'items_per_page' => 10,
    'use_pager' => '1',
    'pager_element' => 0,
    'title' => '',
    'header' => '',
    'header_format' => '1',
    'footer' => '',
    'footer_format' => '1',
    'empty' => '',
    'empty_format' => '1',
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Page');
    $display->display_plugin = 'page';
    $display->position = '2';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => true,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => true,
      'style_options' => true,
      'row_plugin' => true,
      'row_options' => true,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
      'use_ajax' => true,
      'distinct' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'path' => 'frontpage',
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'feed';
    $display->display_title = t('Feed');
    $display->display_plugin = 'feed';
    $display->position = '3';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => false,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'use_ajax' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'use_more' => true,
      'distinct' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => false,
      'style_options' => false,
      'row_plugin' => false,
      'row_options' => false,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'displays' =>
    array (
      'default' => 'default',
      'page' => 'page',
    ),
    'style_plugin' => 'rss',
    'style_options' =>
    array (
      'description' => '',
    ),
    'row_plugin' => 'node_rss',
    'row_options' =>
    array (
      'item_length' => 'default',
    ),
    'path' => 'rss.xml',
    'title' => t('Front page feed'),
  );
  $view->display['feed'] = $display;
  // End copy and paste of Export tab output.

  // Add view to list of views to provide.
  $views[$view->name] = $view;

  // ...Repeat all of the above for each view the module should provide.

  // At the end, return array of default views.
  return $views;
}

/**
 * Alter default views defined by other modules.
 *
 * This hook is called right before all default views are cached to the
 * database. It takes a keyed array of views by reference.
 *
 * Example usage to add a field to a view:
 * @code
 *   $handler =& $view->display['DISPLAY_ID']->handler;
 *   // Add the user name field to the view.
 *   $handler->display->display_options['fields']['name']['id'] = 'name';
 *   $handler->display->display_options['fields']['name']['table'] = 'users';
 *   $handler->display->display_options['fields']['name']['field'] = 'name';
 *   $handler->display->display_options['fields']['name']['label'] = 'Author';
 *   $handler->display->display_options['fields']['name']['link_to_user'] = 1;
 * @endcode
 */
function hook_views_default_views_alter(&$views) {
  if (isset($views['taxonomy_term'])) {
    $views['taxonomy_term']->display['default']->display_options['title'] = 'Categories';
  }
}

/**
 * Stub hook documentation
 */
function hook_views_query_substitutions() {
  // example code here
}

/**
 * This hook is called to get a list of placeholders and their substitutions,
 * used when preprocessing a View with form elements.
 */
function hook_views_form_substitutions() {
  return array(
    '<!--views-form-example-substitutions-->' => 'Example Substitution',
  );
}

/**
 * This hook is called at the very beginning of views processing,
 * before anything is done.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after.
 */
function hook_views_pre_view(&$view, &$display_id, &$args) {
  // example code here
}

/**
 * This hook is called right before the build process, but after displays
 * are attached and the display performs its pre_execute phase.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after.
 */
function hook_views_pre_build(&$view) {
  // example code here
}

/**
 * This hook is called right after the build process. The query is
 * now fully built, but it has not yet been run through db_rewrite_sql.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after.
 */
function hook_views_post_build(&$view) {
  // example code here
}

/**
 * This hook is called right before the execute process. The query is
 * now fully built, but it has not yet been run through db_rewrite_sql.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after.
 */
function hook_views_pre_execute(&$view) {
  // example code here
}

/**
 * This hook is called right after the execute process. The query has
 * been executed, but the pre_render() phase has not yet happened for
 * handlers.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after. Altering the
 * content can be achieved by editing the items of $view->result.
 */
function hook_views_post_execute(&$view) {
  // example code here
}

/**
 * This hook is called right before the render process. The query has
 * been executed, and the pre_render() phase has already happened for
 * handlers, so all data should be available.
 *
 * Adding output to the view can be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after. Altering the
 * content can be achieved by editing the items of $view->result.
 *
 * This hook can be utilized by themes.
 */
function hook_views_pre_render(&$view) {
  // example code here
}

/**
 * Post process any rendered data.
 *
 * This can be valuable to be able to cache a view and still have some level of
 * dynamic output. In an ideal world, the actual output will include HTML
 * comment based tokens, and then the post process can replace those tokens.
 *
 * Example usage. If it is known that the view is a node view and that the
 * primary field will be a nid, you can do something like this:
 *
 * <!--post-FIELD-NID-->
 *
 * And then in the post render, create an array with the text that should
 * go there:
 *
 * strtr($output, array('<!--post-FIELD-1-->', 'output for FIELD of nid 1');
 *
 * All of the cached result data will be available in $view->result, as well,
 * so all ids used in the query should be discoverable.
 *
 * This hook can be utilized by themes.
 */
function hook_views_post_render(&$view, &$output, &$cache) {

}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * MODULENAME.views.inc must be in the directory specified by the 'path' key
 * returned by MODULENAME_views_api(), or the same directory as the .module
 * file, if 'path' is unspecified.
 *
 */
function hook_views_query_alter(&$view, &$query) {
  // example code here
}

/**
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * MODULENAME.views.inc must be in the directory specified by the 'path' key
 * returned by MODULENAME_views_api(), or the same directory as the .module
 * file, if 'path' is unspecified.
 *
 * Alter the rows that appear with a view preview, which include query and
 * performance statistics. $rows is an associative array with two keys:
 * - query: An array of rows suitable for theme('table'), containing information
 *   about the query and the display title and path.
 * - statistics: An array of rows suitable for theme('table'), containing
 *   performance statistics.
 *
 * Warning: $view is not a reference in PHP4 and cannot be modified here. But it IS
 * a reference in PHP5, and can be modified. Please be careful with it.
 *
 * @see theme_table()
 */
function hook_views_preview_info_alter(&$rows, $view) {
  // example code here
}

/**
 * This hooks allows to alter the links at the top of the view edit form.
 * Some modules might want to add links there.
 *
 * @param $links
 *   The links which will be displayed at the top of the view edit form.
 * @param view $view
 *   The full view object which is currently changed.
 * @param $display_id
 *   The current display id which is edited. For example that's 'default' or 'page_1'.
 */
function hook_views_ui_display_top_links_alter(&$links, $view, $display_id) {
  // example code here
}

/**
 * This hook allows to alter the commands which are used on a views ajax
 * request.
 *
 * @param $commands
 *   An array of ajax commands
 * @param $view view
 *   The view which is requested.
 */
function hook_views_ajax_data_alter(&$commands, $view) {
}

/**
 * @}
 */

/**
 * @defgroup views_module_handlers Views module handlers
 * @{
 * Handlers exposed by various modules to Views.
 * @}
 */
