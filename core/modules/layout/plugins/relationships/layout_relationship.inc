<?php

/**
 * @file
 * Class that holds information relating to a layouts context.
 *
 * This class does not extend the LayoutHandler class because contexts are not
 * stored directly within configuration. Instead contexts are determined based
 * on the Layout's path
 */
abstract class LayoutRelationship extends LayoutHandler{
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
  function getContext($context) {}

  /**
   * Form.
   */
  function form(&$form, &$form_state) {
    $contexts = $form_state['contexts'];
    $relationships = layout_relationships_get_relevant_info($contexts);
    $relationship = $relationships[$this->plugin];

    $options = $this->filter_contexts($contexts, $relationship);

    $form_state['relationship'] = $relationship;
    $form['context_parent'] = array(
      '#type' => 'select',
      '#default_value' => isset($this->settings) ? $this->settings['context_parent'] : '',
      '#options' => $options,
      '#title' => $relationship['context_label'],
    );
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
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context', 'load_context_nojs'));
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
   * Return a summary for this context.
   */
  function getAdminSummary() {}

}

/**
 * A class to be used for contexts whose handler cannot be found.
 */
class LayoutRelationshipBroken extends LayoutRelationship {
  function type() {
    return 'broken';
  }
  function label() {
    return t('Broken or missing relationship (@plugin)', array('@plugin' => $this->plugin));
  }
}

