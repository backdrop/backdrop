<?php
/**
 * @file
 * Plugin to provide access control based upon node ID.
 */
class NodeIDAccess extends LayoutAccessNegatable {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'node_id' => NULL,
    );
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['node_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Node ID is'),
      '#weight' => 100,
      '#default_value' => (int) $this->settings['node_id'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['node_id'] = $form_state['values']['node_id'];
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $node = $this->contexts['node']->data;
    if ($this->settings['negate']) {
      return ($node->nid != $this->settings['node_id']);
    }
    else {
      return ($node->nid == $this->settings['node_id']);
    }
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    if ($this->settings['negate']) {
      return t('Node ID is NOT: @node_id', array('@node_id'  => $this->settings['node_id']));
    }
    else {
      return t('Node ID is: @node_id', array('@node_id'  => $this->settings['node_id']));
    }
  }
}
