<?php
/**
 * @file
 * Plugin to provide access control based upon entity bundle.
 */
class TagLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'tag' => array()
    );
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);


    $form['tag'] = array(
      '#title' => t('Tag'),
      '#type' => 'textfield',
      '#default_value' => $this->settings['tag'],
      '#description' => t('Enter tag to match'),
      '#required' => TRUE,
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['tag'] = $form_state['values']['tag'];
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    return 'Tag is "' . $this->settings['tag'] ; '"';
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (!$this->settings['tag']) {
      return FALSE;
    }
    $entity = $this->contexts['node']->data;
    $fields = field_get_items('node', $entity, 'field_tags');
    if (empty($fields)) {
      return FALSE;
    }

    foreach($fields as $delta => $value) {
      $tid = $value['tid'];
      $term = taxonomy_term_load($tid);
      if ($term->name == $this->settings['tag']) {
        return TRUE;
      }
    }

    return FALSE;
  }
}
