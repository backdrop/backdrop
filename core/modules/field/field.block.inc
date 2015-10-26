<?php

/**
 *  FieldBlock extends Block
 *  
 *  Provides entity field values in configurable blocks.
 */
class FieldBlock extends Block {
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $parts = explode(':', $this->plugin, 3);
    $this->child_id = isset($parts[2]) ? $parts[2] : NULL;

    $this->settings += array(
      'label' => 'hidden',
      'formatter' => '',
      'delta_offset' => 0,
      'delta_limit' => '',
      'delta_reversed' => 0,
    );
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return t('Field Block');
  }

  /**
   *  Sets description text on draggable block in Layout builder.
   */
  function getAdminPreview() {
    list($entity_type, $field) = explode('-', $this->child_id, 2);
    return t('Displays values of <em>"@field"</em> field.', array('@field' => $field));
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    $title = parent::getTitle();

    if (empty($title) && $this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      list($entity_type, $field_name) = explode('-', $this->child_id);
      $bundle_name = $this->contexts[$entity_type]->bundle();
      $instance = field_info_instance($entity_type, $field_name, $bundle_name);
      $title = t($instance['label']);
    }

    return $title;
  }

  /**
   *  Sets block content on block view.
   */
  function getContent() {
    $settings = $this->settings;

    // Get a shortcut to the entity.
    list($entity_type, $field_name) = explode('-', $this->child_id);
    $entity = $this->contexts['entity']->data;

    // Load the entity type's information for this field.
    $field = field_info_instance($entity_type, $field_name, $entity->bundle());

    // Do not render if the entity type does not have this field.
    if (empty($field)) {
      return;
    }
    $language = field_language($entity_type, $entity, $field_name);

    $field_settings = array(
      'label' => $settings['label'],
      'type' => $settings['formatter'],
    );

    $all_values = field_get_items($entity_type, $entity, $field_name, $language);
    if (!is_array($all_values)) {
      // Do not render if the field is empty.
      return;
    }
    // Reverse values.
    if ($settings['delta_reversed']) {
      $all_values = array_reverse($all_values, TRUE);
    }

    if (isset($settings['delta_limit'])) {
      $offset = intval($settings['delta_offset']);
      $limit = !empty($settings['delta_limit']) ? $settings['delta_limit'] : NULL;
      $all_values = array_slice($all_values, $offset, $limit, TRUE);
    }

    $clone = clone $entity;
    $clone->{$field_name}[$language] = $all_values;
    $field_output = field_view_field($entity_type, $clone, $field_name, $field_settings, $language);

    return $field_output; // @todo theme function.
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    // Clarify what the default title type is.
    $form['title_display']['title_display']['#options'][LAYOUT_TITLE_DEFAULT] = t('Use field label as title');
    $form['title_display']['title_display']['#description'] = t('The block title is displayed outside of the block content area.');

    $settings = $this->settings;
    $fields = field_get_block_list();

    list($entity_type, $field_name) = explode('-', $this->child_id);
    $field = field_info_field($field_name);

    // Get formatter info.
    $field_cached = $fields[$this->child_id];
    foreach ($field_cached['formatters'] as $key => $formatter) {
      $formatters[$key] = $formatter['label'];
    }

    $form['label'] = array(
      '#type' => 'select',
      '#title' => t('Field label display'),
      '#default_value' => $settings['label'],
      '#options' => array(
        'hidden' => 'None',
        'above' => 'Above',
        'inline' => 'Inline',
      ),
      '#description' => t('The field label is displayed within the block content area.'),
    );

    $form['formatter'] = array(
      '#type' => 'select',
      '#title' => t('Field display format'),
      '#default_value' => $settings['formatter'],
      '#options' => $formatters,
    );

    // Settings that are only shown for multiple field
    if (isset($field['cardinality']) && $field['cardinality'] != 1) {
      list($prefix, $suffix) = explode('@count', t('Skip the first @count item(s)'));
      $form['trim_output'] = array(
        '#type' => 'fieldset',
        '#title' => t('Trim the output values'),
      );
      $form['trim_output']['delta_offset'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#field_prefix' => $prefix,
        '#field_suffix' => $suffix,
        '#default_value' => $settings['delta_offset'],
      );

      list($prefix, $suffix) = explode('@count', t('Then display at most @count item(s)'));
      $form['trim_output']['delta_limit'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#field_prefix' => $prefix,
        '#field_suffix' => $suffix,
        '#description' => t('Enter 0 to display all items.'),
        '#default_value' => $settings['delta_limit'],
      );

      $form['delta_reversed'] = array(
        '#type' => 'fieldset',
        '#title' => t('Reverse the output values'),
      );
      $form['delta_reversed']['delta_reversed'] = array(
        '#title' => t('Display in reverse order'),
        '#type' => 'checkbox',
        '#default_value' => !empty($settings['delta_reversed']),
        '#description' => t('(start from last values)'),
      );
    }
  }
  
  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $values = $form_state['values'];

    $this->settings['label'] = $values['label'];
    $this->settings['formatter'] = $values['formatter'];
    $this->settings['delta_offset'] = isset($values['trim_output']['delta_offset']) ? $values['trim_output']['delta_offset'] : 0;
    $this->settings['delta_limit'] = !empty($values['trim_output']['delta_limit']) ? $values['trim_output']['delta_limit'] : '';
    $this->settings['delta_reversed'] = isset($values['delta_reversed']['delta_reversed']) ? $values['delta_reversed']['delta_reversed'] : 0;
  }
  
  /**
   * Creates an array of "child blocks" for display on the layouts "Add Block" 
   * dialog. 
   *
   * This allows a single entry in hook_block_info() to provide multiple
   * block instances displaying different fields.
   *
   * @see field_get_block_list()
   */
  function getChildren() {
    return field_get_block_list();
  }
}
