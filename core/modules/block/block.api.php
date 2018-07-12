<?php
/**
 * @file
 * Describe hooks provided by the Block module.
 */

/**
 * @addtogroup hooks
 * @{
 */

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
 * - Used to define a theming template suggestion of block__MODULE__DELTA, for
 *   advanced theming possibilities.
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
 * @param $delta
 *   Which block is being configured. This is a unique identifier for the block
 *   within the module, defined in hook_block_info().
 *
 * @return
 *   A configuration form, if one is needed for your block beyond the standard
 *   elements that the block module provides (block title, visibility, etc.).
 *
 * For a detailed usage example, see block_example.module.
 *
 * @see hook_block_info()
 * @see hook_block_save()
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
    config_set('mymodule.settings', 'my_global_value', $edit['my_global_value']);
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
 *   - settings: All block settings as defined for this instance of the block.
 *   - contexts: All layout contexts available for the layout.
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
  // provided by the "somemodule" module.
  if (is_array($data['content']) && $block->module == 'somemodule') {
    $data['content']['#theme_wrappers'][] = 'mymodule_special_block';
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
 * @param array $settings
 *   An array of settings for this block.
 *
 * @see hook_block_view_alter()
 * @see hook_block_view()
 */
function hook_block_view_MODULE_DELTA_alter(&$data, $block) {
  // This code will only run for a specific block. For example, if MODULE_DELTA
  // in the function definition above is set to "mymodule_somedelta", the code
  // will only run on the "somedelta" block provided by the "mymodule" module.

  // Change the title of the "somedelta" block provided by the "mymodule"
  // module.
  $data['title'] = t('New title of the block');
}

/**
 * @} End of "addtogroup hooks".
 */
