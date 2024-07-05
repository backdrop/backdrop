<?php
/**
 * @file
 * Class that holds information relating to a layout's context.
 */
abstract class LayoutContext extends LayoutHandler {

  /**
   * Declares a LayoutContext to be a manually specified custom context.
   */
  const USAGE_TYPE_CUSTOM = 1;

  /**
   * Declares a LayoutContext to be provided based on a menu item or path.
   */
  const USAGE_TYPE_MENU = 2;

  /**
   * Declares a LayoutContext to be a system-wide context
   *
   * Examples include the "current_user" context and "overrides_path" context.
   */
  const USAGE_TYPE_SYSTEM = 4;

  /**
   * Declares a LayoutContext is provided by a relationship to another context.
   */
  const USAGE_TYPE_RELATIONSHIP = 8;

  /**
   * All possible context types.
   *
   * This is equivalent to the following bitwise addition of all the above:
   *
   * @code
   *   self::USAGE_TYPE_CUSTOM | self::USAGE_TYPE_MENU | self::USAGE_TYPE_SYSTEM | self::USAGE_TYPE_RELATIONSHIP
   * @endcode
   *
   * PHP 5.6 and higher is required to do this math in a constant however, so
   * it is defined here as the equivalent integer value.
   */
  const USAGE_TYPE_ALL = 15;

  /**
   * The name of the plugin that provides this context.
   *
   * @var string
   */
  var $plugin;

  /**
   * An array of plugin names by which this context may also be called.
   *
   * For example the "node" plugin is an alias of the "entity" plugin.
   *
   * @var array
   *
   * @see EntityLayoutContext
   */
  var $aliases = array();

  /**
   * Indicator whether this context is required and cannot be removed.
   *
   * Required contexts typically are those provided by a menu path placeholder.
   * While these contexts are required to exist, they can be changed to a
   * different type of context. e.g. A layout at path "foo/%" could have a
   * required context at position 1 with either a "node" or "string" plugin. The
   * required characteristic means that this context must exist, but its type
   * may be user-defined.
   *
   * @var bool
   */
  var $required = FALSE;

  /**
   * Indicator whether this context may not be changed to a different one.
   *
   * Locked contexts are those that are provided by a menu path and tied to a
   * particular context through hook_layout_context_info(). e.g. A layout at
   * path "node/%" will have a locked context of type "node" in position 1.
   * Locked contexts must always exist and may not be changed to a different
   * type through the UI.
   *
   * @var bool
   */
  var $locked = FALSE;

  /**
   * The order of this context in its containing layout.
   *
   * If this context is bound to an argument in a path, this will match that
   * position in the path.
   *
   * @var int
   */
  var $position;

  /**
   * A machine-name for this context.
   *
   * Usually this is auto-generated, but may be customizable in the future.
   */
  var $name;

  /**
   * The human-readable label for this context if available.
   *
   * @var string
   */
  var $label;

  /**
   * The data within this context.
   *
   * @var mixed
   */
  var $data;

  /**
   * The argument value from the URL or passed in directly.
   *
   * @var string
   */
  var $argument;

  /**
   * A list of values that must match in order for this context to be valid.
   *
   * This list is compared against the $data object to ensure that this context
   * applies to the current situation. The key of each entry maps to a property
   * on the $data object. And the values of each entry must match the $data
   * property value in order to be considered valid.
   *
   * @var array
   */
  var $restrictions = array();

  /**
   * The storage state of the context.
   *
   * This represents whether this context is stored in configuration (TRUE) or
   * if it's generated on the fly (FALSE).
   *
   * @var bool
   */
  var $storage = FALSE;

  /**
   * Declares how this context is used by a Layout.
   *
   * For example a "user" context might be available in a Layout through
   * multiple sources. If the layout overrides the path "user/%" one user
   * context would have the $usageType LayoutContext::USAGE_TYPE_MENU.
   *
   * If the author of a node is added to a Layout through a relationship,
   * a second context would be available with the $usageType
   * LayoutContext::USAGE_TYPE_RELATIONSHIP.
   *
   * Checking a context's $usageType can help determine where and how a context
   * was added to a Layout object.
   *
   * This value must be one of the USAGE_TYPE_* constants defined in the
   * LayoutContext class.
   *
   * @var int
   */
  var $usageType = self::USAGE_TYPE_CUSTOM;

  /**
   * Constructor for LayoutContext objects.
   */
  function __construct($plugin_name, $config = array()) {
    parent::__construct($plugin_name, $config);
    $this->plugin = $plugin_name;
    $properties = array(
      'required',
      'locked',
      'position',
      'name',
      'label',
      'storage',
      'usageType',
    );
    foreach ($properties as $property) {
      if (isset($config[$property])) {
        $this->$property = $config[$property];
      }
    }
  }

  /**
   * Return the indicator for this context, i.e. "entity", "string".
   */
  abstract function type();

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    if ($this->label) {
      return $this->label;
    }
    $info = layout_get_context_info($this->plugin);
    if (isset($this->position)) {
      $substitutions = array(
        '@label' => $info['title'], 
        '@position' => $this->position + 1,
      );
      return t('@label from the path (in position @position)', $substitutions);
    }
    else {
      return check_plain($info['title']);
    }
  }

  /**
   * Set the data for this context.
   */
  function setData($data) {
    $this->data = $data;
  }

  /**
   * Set the context data from the router item map.
   *
   * @param array $router_map
   *   A router item map array.
   *
   * @deprecated Please use setDataFromRouter() instead.
   */
  function setDataFromRouterMap($router_map) {
    if (isset($this->position) && !empty($router_map[$this->position])) {
      // If the menu router set the data for a context object or a client did
      // so via hook_layout_load_by_router_item_alter(), we don't need to do
      // any setting here. But if $this->data is not an object, then it is
      // either a raw string that must be converted to an object by a load
      // callback, or it is a string pass-through.
      if (!is_object($this->data)) {
        // If clients replaced any layout with one whose placeholders are in a
        // new position, they need to have altered $context->position so that
        // it points to the right place in the router item to get the data
        // object.
        $context_data = $router_map[$this->position];
        // If the router item contains the context object in the right
        // position, then we can just set the data from that.
        if (is_object($context_data)) {
          $this->setData($context_data);
        }
        else {
          // If the context data was not an object, then we can try to set
          // it using the plugin's load callback. If there is no callback,
          // then it must be a string pass-through, so we leave it as is.
          $context_info = layout_get_context_info($this->plugin);

          // CHECK: the following 3 lines don't seem to be needed. By this point
          // of the page request all context data is already loaded onto
          // $router_item['map'].
          // if (isset($context_info['load callback'])) {
          //   $context_data = call_user_func_array($context_info['load callback'], array($router_item['original_map'][$this->position]));
          // }

          $this->setData($context_data);
        }
      }
    }
  }

  /**
   * Set the context data from the router item.
   *
   * @param array $router_item
   *   A router item.
   */
  public function setDataFromRouter(array $router_item) {
    $router_map = $router_item['map'];

    if (isset($this->position) && !empty($router_map[$this->position])) {
      // If the menu router set the data for a context object or a client did
      // so via hook_layout_load_by_router_item_alter(), we don't need to do
      // any setting here. But if $this->data is not an object, then it is
      // either a raw string that must be converted to an object by a load
      // callback, or it is a string pass-through.
      if (!is_object($this->data)) {
        // If clients replaced any layout with one whose placeholders are in a
        // new position, they need to have altered $context->position so that
        // it points to the right place in the router item to get the data
        // object.
        $context_data = $router_map[$this->position];
        // If the router item contains the context object in the right
        // position, then we can just set the data from that.
        if (is_object($context_data)) {
          $this->setData($context_data);
        }
        else {
          // If the context data was not an object, then we can try to set
          // it using the plugin's load callback. If there is no callback,
          // then it must be a string pass-through, so we leave it as is.
          $context_info = layout_get_context_info($this->plugin);
          $this->setData($context_data);
        }
      }

      // Set the raw argument in the context if router_original_map.
      $router_original_map = $router_item['original_map'];

      if (!empty($router_original_map[$this->position])) {
        $this->argument = $router_original_map[$this->position];
      }
    }
  }

  /**
   * Check if this context matches a plugin name.
   *
   * This helps identify contexts which have aliases, such as a "node" context
   * which is also considered an "entity" context.
   *
   * For example:
   * @code
   * $context = layout_create_context('node');
   *
   * var_dump($context->isA('node')); // TRUE.
   * var_dump($context->isA('entity')); // TRUE.
   * var_dump($context->isA('user')); // FALSE.
   * @endcode
   *
   * @param string $plugin
   *
   * @return bool
   */
  function isA($plugin) {
    return $plugin === $this->plugin || in_array($plugin, $this->aliases);
  }

  /**
   * Settings form for configuring this context item.
   */
  function form(&$form, &$form_state) {
  }

  /**
   * Validate handler for the settings form for this context item.
   */
  function formValidate($form, &$form_state) {
  }

  /**
   * Submit handler for the settings form for this context item.
   */
  function formSubmit($form, &$form_state) {
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context'));
    $settings = array_diff_key($form_state['values'], $excluded);
    $this->settings = array_merge($this->settings, $settings);
  }

  /**
   * Return a summary for this context.
   *
   * The summary is displayed in the context table on Layout settings pages.
   */
  function getAdminSummary() {}
}

/**
 * A class to be used for contexts whose handler cannot be found.
 */
class LayoutContextBroken extends LayoutContext {
  function type() {
    return 'broken';
  }

  function label() {
    return t('Broken or missing context (@plugin)', array('@plugin' => $this->plugin));
  }
}

/**
 * Pass-through context used when an unknown argument is encountered.
 */
class LayoutOverridesPathContext extends LayoutContext {
  var $usageType = LayoutContext::USAGE_TYPE_SYSTEM;

  function type() {
    return 'overrides_path';
  }

  function label() {
    return t('Layout overrides path');
  }
}

/**
 * Pass-through context used when an unknown argument is encountered.
 */
class LayoutStringContext extends LayoutContext {
  function type() {
    return 'string';
  }

  function label() {
    return t('String pass-through');
  }

  /**
   * Return a summary for this context.
   */
  public function getAdminSummary($path = NULL) {
    $argument_parts = explode('/', $path);
    $argument_parts[$this->position] = '<strong>' . $argument_parts[$this->position] . '</strong>';

    return t('Position @index:', array('@index' => $this->position + 1)) . ' ' . implode('/', $argument_parts);
  }
}
