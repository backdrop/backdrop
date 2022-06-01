<?php
/**
 * @file
 * Layout context handler for entities.
 */
class EntityLayoutContext extends LayoutContext {
  /**
   * The entity type for this context.
   *
   * @var string
   */
  protected $entity_type;

  function __construct($plugin_name, $config = array()) {
    parent::__construct($plugin_name, $config);

    $this->entity_type = $plugin_name;
    $this->aliases = array('entity');
  }

  function type() {
    return 'entity';
  }

  function label() {
    $info = entity_get_info($this->entity_type);
    if (isset($this->settings['id'])) {
      if ($entity = entity_load($this->entity_type, $this->settings['id'])) {
        return t('@type: @id (@title)', array('@type' => $info['label'], '@id' => $entity->id(), '@title' => $entity->label()));
      }
      else {
        return t('Unknown @type: @id', array('@type' => $info['label'], '@id' =>$this->settings['id']));
      }
    }
    if ($this->entity_type === 'user' && isset($this->name) && $this->name === 'current_user') {
      return t('Current user');
    }
    else {
      return parent::label();
    }
  }

  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $info = entity_get_info($this->entity_type);
    $input = isset($form_state['handler']->settings['id']) ? $form_state['handler']->settings['id'] : NULL;
    $form_state['input']['id'] = $input;
    $form['id'] = array(
      '#title' => t('Enter the ID of a @entity', array('@entity' => $info['label'])),
      '#type' => 'number',
      '#default_value' => $input,
      '#step' => 1,
      '#min' => 1,
      // @todo: Backdrop doesnt yet have a comprehensive entity autocomplete
      // function. We'd need to write a custom autocomplete function that
      // handles all entity types. This would be best as a core function
      // rather than just for this code.
      // '#autocomplete_path' => 'node/autocomplete',
      '#weight' => -10,
    );
  }

  /**
   * Validate handler for the settings form for this context item.
   */
  function formValidate($form, &$form_state) {
    if (empty($form_state['values']['id'])) {
      form_set_error('id', t('An ID is required.'));
    }
    elseif (!entity_load($this->entity_type, $form_state['values']['id'])) {
      $info = entity_get_info($this->entity_type);
      form_set_error('id', t('The selected @entity does not exist.', array('@entity' => $info['label'])));
    }
  }

  /**
   * Submit handler for the settings form for this context item.
   */
  function formSubmit($form, &$form_state) {
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context', 'load_context_nojs', 'context_add_button'));
    $settings = array_diff_key($form_state['values'], $excluded);
    // This produces config keys for contexts in the format "node1" or "user3".
    $this->name = $this->plugin . $settings['id'];
    $this->settings = $settings;
  }

  /**
   * Return a summary for this context.
   */
  function getAdminSummary($path = NULL) {
    $info = entity_get_info($this->entity_type);
    if ($this->position) {
      $argument_parts = explode('/', $path);
      $argument_parts[$this->position] = '<strong>' . $argument_parts[$this->position] . '</strong>';

      return t('Position @index:', array('@index' => $this->position + 1)) . ' ' . implode('/', $argument_parts);
    }
    else {
      return t('@entity ID: @id', array('@id' => $this->settings['id'], '@entity' => $info['label']));
    }
  }

   /** 
   * Set the context data from the router item array.
   * 
   * @param array $router_item
   *   A router item array.
   */
  function setDataFromRouterItem($router_item) {
    if (isset($this->position) && !empty($router_item['map'][$this->position])) {
      // If the menu router set the data for a context object or a client did
      // so via hook_layout_load_by_router_item_alter(), we don't need to do
      // any setting here. But if $context->data is not an object, then it is
      // either a raw string that must be converted to an object by a load
      // callback or it is a string pass-through.
      if (!is_object($this->data)) {
        // If clients replaced any layout with one whose placeholders are in a
        // new position, they need to have altered $context->position so that
        // it points to the right place in the router item to get the data
        // object.
        $context_data = $router_item['map'][$this->position];
        // If the router item contains the context object in the right
        // position, then we can just set the data from that.
        if (is_object($context_data)) {
          $this->setData($context_data);
        }
        else { // TODO: this if/else may not be necessary
          // If the context data was not an object, then we can try to set
          // it using the plugin's load callback. If there is no callback,
          // then it must be a string pass-through, so we leave it as is.
          $context_info = layout_get_context_info($this->plugin);
          if (isset($context_info['load callback'])) {
            $context_data = call_user_func_array($context_info['load callback'], array($router_item['original_map'][$this->position]));
          }
          $this->setData($context_data);
        }
      }
    }
  }
}
