<?php
/**
 * @file
 * Plugin to provide access control based upon entity bundle.
 */
class EntityFieldValueLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'field' => '',
      'value' => '',
    );

    // Set entity type and bundle.
    $info = layout_get_access_info($plugin_name);
    $this->entity_type = $info['entity_type'];
    $this->bundle = $info['bundle'];
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $entity_type = $this->entity_type;
    $bundle_type = $this->bundle;
    $entity_info = entity_get_info($entity_type);
    $instances   = field_info_instances($entity_type, $bundle_type);

    $options = array();
    foreach ($instances as $field_name => $instance){
      $options[$field_name] = $instance['label'];
    }

    $form['field'] = array(
      '#title' => t('Field name'),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $this->settings['field'],
    );
    $form['value'] = array(
      '#title' => t('Value'),
      '#type' => 'textfield',
      '#default_value' => $this->settings['value'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['field'] = $form_state['values']['field'];
    $this->settings['value'] = trim($form_state['values']['value']);
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    $entity_type = $this->entity_type;
    $bundle_type = $this->bundle;

    $entity_info = entity_get_info($entity_type);
    $bundle_label = $entity_info['bundles'][$bundle_type]['label'];

    $field_name = $this->settings['field'];
    $value = $this->settings['value'];
    $instance = field_info_instance($entity_type, $field_name, $bundle_type);

    $replacements = array(
      ':bundle' => $bundle_label,
      ':field' => $instance['label'],
      '!value' => $value,
    );

    return t('The :field field on :bundle is set to "!value"', $replacements);
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (!$this->settings['field']) {
      return FALSE;
    };

    /* @var Entity $entity */
    $entity = $this->contexts[$this->entity_type]->data;
    $field_name = $this->settings['field'];
    $field = trim($entity->{$field_name}[LANGUAGE_NONE][0]['value']);

    if (!empty($this->settings['value'])) {
      if ($field != trim($this->settings['value'])) {
        return FALSE;
      }
    }

    return TRUE;
  }
}
