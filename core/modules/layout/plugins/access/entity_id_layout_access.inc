<?php
/**
 * @file
 * Plugin to provide access control based upon entity ID.
 */
class EntityIDLayoutAccess extends LayoutAccessNegatable {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'entity_id' => NULL
    );

    // Set entity info.
    $info = layout_get_access_info($plugin_name);
    $this->entity_type = $info['entity_type'];
    $this->entity_info = entity_get_info($this->entity_type);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['entity_id'] = array(
      '#title' => t('@entity @entity_id_key is', array(
        '@entity' => $this->entity_info['label'],
        '@entity_id_key' => strtoupper($this->entity_info['entity keys']['id']),
      )),
      '#type' => 'number',
      '#required' => TRUE,
      '#min' => 1,
      '#default_value' => is_null($this->settings['entity_id']) ? NULL : (int) $this->settings['entity_id'],
      '#weight' => 100,
    );
  }

  /**
   * {@inheritdoc}
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    $entity_id = $form_state['values']['entity_id'];
    if (is_numeric($entity_id) && round($entity_id) == $entity_id && $entity_id >= 1) {
      $entity = entity_load($this->entity_type, $entity_id);
      if (!$entity) {
        $label = $this->entity_info['label'];
        form_set_error('entity_id', t('There is no @label with ID @entity_id.', array(
          '@label' => $label,
          '@entity_id' => $entity_id,
        )));
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['entity_id'] = $form_state['values']['entity_id'];
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    if ($this->settings['negate']) {
      return t('@entity ID is NOT: @entity_id', array(
        '@entity' => $this->entity_info['label'],
        '@entity_id' => $this->settings['entity_id'],
      ));
    }
    else {
      return t('@entity ID is: @entity_id', array(
        '@entity' => $this->entity_info['label'],
        '@entity_id' => $this->settings['entity_id'],
      ));
    }
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $key = $this->entity_info['entity keys']['id'];
    $entity = $this->contexts[$this->entity_type]->data;

    if ($this->settings['negate']) {
      return ($entity->$key != $this->settings['entity_id']);
    }
    else {
      return ($entity->$key == $this->settings['entity_id']);
    }
  }
}
