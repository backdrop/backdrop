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
      return t('Logged in user account');
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
      // @todo: Backdrop doesn't yet have a comprehensive entity autocomplete
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
}
