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
    return $title;
  }

  /**
   *  Sets block content on block view.
   */
  function getContent() {
    $settings = $this->settings;

    // Get a shortcut to the entity.
    list($entity_type, $field_name) = explode('-', $this->child_id);
    $block_list = field_get_block_list();
    $context = $block_list[$this->child_id]['required contexts'][$entity_type];
    $entity = menu_get_object($context);

    // Do not render if the entity is not available.
    if (empty($entity)) {
      return;
    }

    // Load the entity type's information for this field.
    $ids = entity_extract_ids($entity_type, $entity);
    $field = field_info_instance($entity_type, $field_name, $ids[2]);

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
    if (isset($settings['delta_reversed']) && $settings['delta_reversed']) {
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
      '#type' => 'radios',
      '#title' => t('Show field label'),
      '#default_value' => isset($settings['label']) ? $settings['label'] : 'above',
      '#options' => array(
        'hidden' => 'Hide label',
        'above' => 'Above',
        'inline' => 'Inline',
      ),
    );
    $form['formatter'] = array(
      '#type' => 'select',
      '#title' => t('Choose display format'),
      '#default_value' => isset($settings['formatter']) ? $settings['formatter'] : '',
      '#options' => $formatters,
    );
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
        '#default_value' => isset($settings['delta_offset']) ? $settings['delta_offset'] : 0,
      );

      list($prefix, $suffix) = explode('@count', t('Then display at most @count item(s)'));
      $form['trim_output']['delta_limit'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#field_prefix' => $prefix,
        '#field_suffix' => $suffix,
        '#description' => t('Enter 0 to display all items.'),
        '#default_value' => isset($settings['delta_limit']) ? $settings['delta_limit'] : 0,
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
    $this->settings['delta_offset'] = isset($values['trim_output']['delta_offset']) ? $values['trim_output']['delta_offset'] : NULL;
    $this->settings['delta_limit'] = isset($values['trim_output']['delta_limit']) ? $values['trim_output']['delta_limit'] : NULL;
    $this->settings['delta_reversed'] = isset($values['delta_reversed']['delta_reversed']) ? $values['delta_reversed']['delta_reversed'] : NULL;
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
