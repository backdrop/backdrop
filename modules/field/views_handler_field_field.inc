<?php
// $Id: views_handler_field_field.inc,v 1.1.2.4 2010/10/25 21:40:46 dereine Exp $

/**
 * Helper function: Return an array of formatter options for a field type.
 *
 * Borrowed from field_ui.
 */
function _field_view_formatter_options($field_type = NULL) {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $field_types = field_info_field_types();
    $options = array();
    foreach (field_info_formatter_types() as $name => $formatter) {
      foreach ($formatter['field types'] as $formatter_field_type) {
        // Check that the field type exists.
        if (isset($field_types[$formatter_field_type])) {
          $options[$formatter_field_type][$name] = $formatter['label'];
        }
      }
    }
  }

  if ($field_type) {
    return !empty($options[$field_type]) ? $options[$field_type] : array();
  }
  return $options;
}

/**
 * A field that displays fields.
 */
class views_handler_field_field extends views_handler_field {
  /**
   * Called to add the field to a query.
   */
  function query() {
    // TODO: we should try to use the data from the join if possible first.
    // $join = $this->get_join();
    // That would avoid joining at all the field table.
    return parent::query();
  }

  function option_definition() {
    $options = parent::option_definition();

    $field = field_info_field($this->definition['field_name']);
    $field_type = field_info_field_types($field['type']);

    $options['type'] = array(
      'default' => $field_type['default_formatter'],
    );
    $options['settings'] = array(
      'default' => array(),
    );

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $field = field_info_field($this->definition['field_name']);
    $formatters = _field_view_formatter_options($field['type']);

    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Formatter'),
      '#options' => $formatters,
      '#default_value' => $this->options['type'],
      '#ajax' => array(
        'path' => views_ui_build_form_url($form_state),
      ),
    );

    // Get the currently selected formatter.
    if (isset($form_state['values']['options']['type'])) {
      $format = $form_state['values']['options']['type'];
    }
    else {
      $format = $this->options['type'];
    }
    $formatter = field_info_formatter_types($format);
    $settings = $this->options['settings'] + field_info_formatter_settings($format);

    // Provide an instance array for hook_field_formatter_settings_form().
    $instance = $this->_fake_instance($this->definition['field_name'], $formatter, $settings);

    // Store the settings in a '_dummy' view mode.
    $instance['display']['_dummy'] = array(
      'type' => $format,
      'settings' => $settings,
    );

    // Get the settings form.
    $settings_form = array('#value' => array());
    $function = $formatter['module'] . '_field_formatter_settings_form';
    if (function_exists($function)) {
      $settings_form = $function($field, $instance, '_dummy', $form, $form_state);
    }
    $form['settings'] = $settings_form;
  }

  /**
  * @TODO
  *    Once views requires ctools change this to the helper method in ctools.
  */
  function _fake_instance($field_name, $formatter, $formatter_settings) {
    $field = field_read_field($field_name);

    $field_type = field_info_field_types($field['type']);

    return array(
      // Build a fake entity type and bundle.
      'field_name' => $field_name,
      'entity_type' => '_dummy',
      'bundle' => '_dummy',

      // Use the default field settings for settings and widget.
      'settings' => field_info_instance_settings($field['type']),
      'widget' => array(
        'type' => $field_type['default_widget'],
        'settings' => array(),
      ),

      // Build a dummy display mode.
      'display' => array(
        '_dummy' => array(
          'type' => $formatter,
          'settings' => $formatter_settings,
        ),
      ),

      // Set the other fields to their default values.
      // @see _field_write_instance().
      'required' => FALSE,
      'label' => $field_name,
      'description' => '',
      'deleted' => 0,
    );
  }



  /**
   * Loads the objects providing the field information necessary to render the
   *   field in the View.
   */
  function pre_render(&$values) {
    static $entity_type_map;

    if (!empty($values)) {
      // Cache the entity type map for repeat usage.
      if (empty($entity_type_map)) {
        $entity_type_map = db_query('SELECT etid, type FROM {field_config_entity_type}')->fetchAllKeyed();
      }

      // Create an array mapping the Views values to their object types.
      $objects_by_type = array();

      foreach ($values as $key => $object) {
        // Derive the entity type. For some field types, etid might be empty.
        if (isset($object->{$this->aliases['etid']}) && isset($entity_type_map[$object->{$this->aliases['etid']}])) {
          $entity_type = $entity_type_map[$object->{$this->aliases['etid']}];
          $entity_id = $object->{$this->field_alias};
          $objects_by_type[$entity_type][$key] = $entity_id;
        }
      }

      // Load the objects.
      foreach ($objects_by_type as $entity_type => $oids) {
        $objects = entity_load($entity_type, $oids);

        foreach ($oids as $key => $entity_id) {
          $values[$key]->_field_cache[$this->field_alias] = array(
            'entity_type' => $entity_type,
            'object' => $objects[$entity_id],
          );
        }
      }
    }
  }

  function render($values) {
    if (isset($values->_field_cache[$this->field_alias])) {
      // Prepare arguments for rendering based on the objects cached in the
      // pre-render phase and the display options for this field.
      $entity_type = $values->_field_cache[$this->field_alias]['entity_type'];
      $object = $values->_field_cache[$this->field_alias]['object'];

      $display = array(
        'type' => $this->options['type'],
        'settings' => $this->options['settings'],
        'label' => 'hidden',
      );

      return drupal_render(field_view_field($entity_type, $object, $this->definition['field_name'], $display));
    }
    else {
      return '';
    }
  }
}
