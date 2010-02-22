<?php
// $Id$

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
      'translatable' => TRUE,
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
    );
  }

  function pre_render(&$values) {
    if (!empty($values) && empty($values[0]->_object)) {

      // OMG, this is *ugly*.
      $obj_type_map = db_query('SELECT etid, type FROM {field_config_entity_type}')->fetchAllKeyed();

      // Load the full objects.
      $objects_by_type = array();
      foreach ($values as $key => $object) {
        // Derive the entity type. For some field types, etid might be empty.
        if (isset($object->{$this->aliases['etid']}) && isset($obj_type_map[$object->{$this->aliases['etid']}])) {
          $obj_type = $obj_type_map[$object->{$this->aliases['etid']}];
          $entity_id = $object->{$this->field_alias};
          $objects_by_type[$obj_type][$key] = $entity_id;
        }
      }

      // Load the objects.
      foreach ($objects_by_type as $obj_type => $oids) {
        $objects = entity_load($obj_type, $oids);
        foreach ($oids as $key => $entity_id) {
          if (isset($objects[$entity_id])) {
            $values[$key]->_obj_type = $obj_type;
            $values[$key]->_object = $objects[$entity_id];
          }
        }
      }
    }
  }

  function render($values) {
    if (isset($values->_obj_type)) {
      $field_name = $this->definition['field_name'];
      $display = array(
        'type' => $this->options['type'],
        'label' => 'hidden',
      );

      return drupal_render(field_view_field($values->_obj_type, $values->_object, $field_name, $display));
    }
    else {
      return '';
    }
  }
}

