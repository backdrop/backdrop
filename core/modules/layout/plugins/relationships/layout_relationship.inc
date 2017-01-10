<?php

/**
 * @file
 * Class that holds information relating to a layouts context.
 *
 * This class does not extend the LayoutHandler class because contexts are not
 * stored directly within configuration. Instead contexts are determined based
 * on the Layout's path
 */
abstract class LayoutRelationship {
  /**
   * The name of the plugin that provides this context.
   *
   * @var string
   */
  var $plugin;

  /**
   * A machine-name for this relationship.
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
   * The type of context this relates to.
   *
   * @var mixed
   */
  var $context;

  /**
   * Constructor for LayoutContext objects.
   */
  function __construct($plugin_name, $context) {
    $this->plugin = $plugin_name;
    $properties = array(
      'name',
      'label',
      'context',
    );
    foreach ($properties as $property) {
      if (isset($config[$property])) {
        $this->$property = $config[$property];
      }
    }
  }

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    if ($this->label) {
      return $this->label;
    }
    $info = layout_get_context_info($this->plugin);
    if (isset($this->position)) {
      return t('@label from path (position @position)', array('@label' => $info['title'], '@position' => $this->position + 1));
    }
    else {
      return check_plain($info['title']);
    }
  }

  /**
   * Get the context from this relationship.
   */
  function getContext() {}

}
