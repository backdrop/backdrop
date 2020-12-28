<?php

/**
 * @file
 * Class that holds information relating to a layout's context relationships.
 */
abstract class LayoutRelationship extends LayoutHandler{
  /**
   * The name of the plugin that provides this relationship.
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
   * The human-readable label for this relationship if available.
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
   * The type of context this relates to.
   *
   * @var mixed
   */
  var $childDelta;

  /**
   * Constructor for LayoutRelationship objects.
   */
  function __construct($plugin_name, $config) {
    parent::__construct($plugin_name, $config);
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
    if (isset($config['settings']['relationship'])) {
      list(, $child) = array_pad(explode(':', $config['settings']['relationship']), 2, null);
      $this->childDelta = $child;
    }
  }

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {}

  /**
   * Form.
   */
  function form(&$form, &$form_state) {
    $contexts = $form_state['contexts'];

    $relationship_info = $this->getChild($this->childDelta);

    $options = $this->filter_contexts($contexts, $relationship_info);

    $form_state['relationship'] = $relationship_info;
    $form['context_parent'] = array(
      '#type' => 'select',
      '#default_value' => isset($this->settings['context_parent']) ? $this->settings['context_parent'] : '',
      '#options' => $options,
      '#title' => $relationship_info['context_label'],
    );
  }

  /**
   * Validate handler for the settings form for this relationship item.
   */
  function formValidate($form, &$form_state) {}

  /**
   * Submit handler for the settings form for this relationship item.
   */
  function formSubmit($form, &$form_state) {
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context', 'load_relationship_nojs', 'relationship_add_button'));
    $settings = array_diff_key($form_state['values'], $excluded);
    $this->settings = array_merge($this->settings, $settings);
    $this->context = $form_state['relationship']['context'];
  }

  /**
   * Returns an array of appropriate contexts for this relationship.
   */
  function filter_contexts($contexts, $relationship) {
    $options = array();

    foreach ($contexts as $context) {
      if ($context->plugin == $relationship['context']) {
        if (isset($context->position)) {
          $label = $relationship['context_label'] . ' ' . t('at position') . ' ' . $context->position;
          $key = 'position:' . $context->position;
        }
        elseif (isset($context->settings['id'])) {
          $label = $relationship['context_label'] . ' ' . t('ID') . ' ' . $context->settings['id'];
          $key = 'id:' . $context->settings['id'];
        }
        else {
          $label = $relationship['context_label'];
          $key = $context->plugin;
        }
        $options[$key] = $label;
      }
    }
    return $options;
  }


  /**
   * Convert the configuration of this relationship to an array for storage.
   */
  function toArray() {
    $array = array(
      'context' => $this->context,
      'settings' => $this->settings,
    );

    return $array;
  }

  /**
   * Return a summary for this relationship.
   */
  function getAdminSummary() {}

  /**
   * Return child relationship plugins.
   */
  function getChildren() {
    return NULL;
  }

  /**
   * Return child relationship plugins.
   */
  function getChild() {
    return layout_get_relationship_info($this->plugin);
  }

}

/**
 * A class to be used for relationships whose handler cannot be found.
 */
class LayoutRelationshipBroken extends LayoutRelationship {
  function type() {
    return 'broken';
  }
  function label() {
    return t('Broken or missing relationship (@plugin)', array('@plugin' => $this->plugin));
  }
}

