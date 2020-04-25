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
    // @todo: Contexts do not yet have settings to make this possible. Once
    // the ability to add manual contexts (e.g. an arbitrary node), then this
    // will actually do something.
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

    $form['entity'] = array(
      '#title' => t('Enter the title or ID of a @entity entity', array('@entity' => $this->plugin)),
      '#type' => 'textfield',
      '#maxlength' => 512,
      '#autocomplete_path' => 'path-autocomplete',
      '#weight' => -10,
    );
  }

  /**
   * Submit handler for the settings form for this context item.
   */
  function formSubmit($form, &$form_state) {
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context'));
    $settings = array_diff_key($form_state['values'], $excluded);
    $this->name = $this->plugin . $settings['entity'];
    // @todo deal with name changes.
    $this->settings = array_merge($this->settings, $settings);
  }

  /**
   * Return a summary for this context.
   */
  function getAdminSummary($path = NULL) {
    if ($this->position) {
      $argument_parts = explode('/', $path);
      $argument_parts[$this->position] = '<strong>' . $argument_parts[$this->position] . '</strong>';

      return t('Position @index:', array('@index' => $this->position + 1)) . ' ' . implode('/', $argument_parts);
    }
    else {
      return t('Special context: @id', array('@id' => $this->settings['entity']));
    }
  }
}
