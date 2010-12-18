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
    parent::query();

    $base_table = $this->view->base_table;

    // If the current field is under a relationship you can't be sure that the
    // base table of the view is the base table of the current field.
    // For example a field from a node author on a node view does have users as base table.
    if (!empty($this->relationship)) {
      foreach ($this->view->relationship as $relationship) {
        if ($relationship->alias == $this->relationship) {
          $base_table = $relationship->definition['base'];
        }
      }
    }

    // Get the entity type according to the base table of the field.
    $this->entity_type = $entity_type = $this->definition['entity_tables'][$base_table];
    $this->entity_info = entity_get_info($entity_type);

    // Query for bundle, revision fields necessary for rendering fields.
    // @TODO
    // The bundle system will be fixed in d8, so no special cases should be needed.
    switch ($entity_type) {
      case 'taxonomy_term':
      case 'comment':
        $this->aliases[$this->definition['field_name'] . '_' . 'bundle'] = $this->query->add_field($this->table_alias, 'bundle');
        break;
      default:
        foreach ($this->entity_info['entity keys'] as $key => $column) {
          if (!empty($column)) {
            if ($key == 'bundle') {
              $this->aliases[$this->definition['field_name'] . '_' . $key] = $this->query->add_field($this->table_alias, 'bundle');
            }
            elseif ($key == 'revision') {
              $this->aliases[$this->definition['field_name'] . '_' . $key] = $this->query->add_field($this->table_alias, 'revision_id');
            }
          }
        }
        break;
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $column = _field_sql_storage_columnname($this->definition['field_name'], $this->options['click_sort_column']);
    // The field has already been added to the query, just add a sort on it.
    $this->query->add_orderby(NULL, NULL, $order, $this->aliases[$column]);
  }

  function option_definition() {
    $options = parent::option_definition();

    $field = field_info_field($this->definition['field_name']);
    $field_type = field_info_field_types($field['type']);

    // If the field has a "value" column, we probably need that one.
    $options['click_sort_column'] = array(
      'default' => in_array('value', $field['columns']) ? 'value' : '',
    );
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
    $column_names = array_keys($field['columns']);

    // No need to ask the user anything if the field has only one column.
    if (count($field['columns']) == 1) {
      $form['click_sort_column'] = array(
        '#type' => 'value',
        '#value' => $column_names[0],
      );
    }
    else {
      $form['click_sort_column'] = array(
        '#type' => 'select',
        '#title' => t('Click sort column'),
        '#options' => drupal_map_assoc($column_names),
        '#default_value' => $this->options['click_sort_column'],
        '#description' => t('Used by Style: Table to determine the actual column to click sort the field on. The default is usually fine.'),
      );
    }

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
    ctools_include('fields');
    $instance = ctools_fields_fake_field_instance($this->definition['field_name'], '_dummy', $formatter, $settings);

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
   * Builds a fake entity object for each row fetched from the database,
   * creating the field structure required for rendering.
   */
  function pre_render(&$values) {
    if (!empty($values)) {
      $entity_info = $this->entity_info;
      $entity_type = $this->entity_type;

      $objects = array();
      foreach ($values as $key => $row) {
        // This entity doesn't contain our field, move along.
        if (!$row->{$this->field_alias}) {
          continue;
        }

        $object = new stdClass();
        $object->{$entity_info['entity keys']['id']} = $row->{$this->field_alias};
        // Not every entity type has bundle and revision.
        $bundle = '';
        if (!empty($entity_info['entity keys']['bundle'])) {
          $object->{$entity_info['entity keys']['bundle']} = $bundle = $row->{$this->aliases[$this->definition['field_name'] . '_bundle']};
        }
        if (!empty($entity_info['entity keys']['revision'])) {
          $object->{$entity_info['entity keys']['revision']} = $row->{$this->aliases[$this->definition['field_name'] . '_revision']};
        }

        $language = $row->{$this->aliases['language']};
        $delta = $row->{$this->aliases['delta']};

        // The entity object needs to contain all fields, not just the one
        // being displayed, in order for field_view_field() to work properly.
        foreach (field_info_instances($entity_type, $bundle) as $instance) {
          $field_values = array();
          $field_info = field_info_field($instance['field_name']);
          foreach (array_keys($field_info['columns']) as $column_name) {
            $sql_column = _field_sql_storage_columnname($instance['field_name'], $column_name);

            // We might not have this field in the database results
            if (isset($this->aliases[$sql_column])) {
              $alias = $this->aliases[$sql_column];
              $field_values[$language][$delta][$column_name] = $row->{$alias};
            }
            else {
              $field_values[$language][$delta][$column_name] = '';
            }
          }
          $object->{$instance['field_name']} = $field_values;
        }
        $objects[$row->{$this->field_alias}] = $object;
      }

      // Invoke field-type module's hook_field_load().
      _field_invoke_multiple('load', $entity_type, $objects);

      // Invoke hook_field_attach_load(): let other modules act on loading the
      // entity.
      module_invoke_all('field_attach_load', $entity_type, $objects);

      foreach ($values as $key => $row) {
        if ($row->{$this->field_alias}) {
          $values[$key]->_field_data[$this->field_alias] = array(
            'entity_type' => $entity_type,
            'object' => $objects[$row->{$this->field_alias}],
          );
        }
      }
    }
  }

  function render($values) {
    if (isset($values->_field_data[$this->field_alias])) {
      // Prepare arguments for rendering based on the objects cached in the
      // pre-render phase and the display options for this field.
      $entity_type = $values->_field_data[$this->field_alias]['entity_type'];
      $object = $values->_field_data[$this->field_alias]['object'];

      $display = array(
        'type' => $this->options['type'],
        'settings' => $this->options['settings'],
        'label' => 'hidden',
      );

      $render_array = field_view_field($entity_type, $object, $this->definition['field_name'], $display);
      return drupal_render($render_array);
    }
    else {
      return '';
    }
  }
}
