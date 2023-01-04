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
      'field' => 'field_tags',
      'tag' => '',
      'tid' => 0,
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
      '#autocomplete_path' => 'taxonomy/autocomplete/' . $this->settings['field'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $tag = $form_state['values']['tag'];
    $this->settings['tag'] = $tag;
    $terms =  taxonomy_term_load_multiple_by_name($tag);
    foreach($terms as $delta => $term) {
      $this->settings['tid'] = $term->tid;
      break;
    }
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    return t('Tag is "@tag"', array('@tag' => $this->settings['tag']));
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (!$this->settings['tid']) {
      return FALSE;
    }
    $entity = $this->contexts['node']->data;
    $fields = field_get_items('node', $entity, $this->settings['field']);
    if (empty($fields)) {
      return FALSE;
    }

    foreach($fields as $delta => $value) {
      $tid = $value['tid'];
      if ($tid == $this->settings['tid']) {
        return TRUE;
      }
    }

    return FALSE;
  }
}
