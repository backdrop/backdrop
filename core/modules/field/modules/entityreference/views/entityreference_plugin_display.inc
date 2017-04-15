<?php

/**
 * @file
 * Handler for entityreference_plugin_display.
 */
class entityreference_plugin_display extends views_plugin_display {

  function option_definition() {
    $options = parent::option_definition();

    // Force the style plugin to 'entityreference_style' and the row plugin to
    // 'fields'.
    $options['style_plugin']['default'] = 'entityreference_style';
    $options['defaults']['default']['style_plugin'] = FALSE;
    $options['defaults']['default']['style_options'] = FALSE;
    $options['row_plugin']['default'] = 'entityreference_fields';
    $options['defaults']['default']['row_plugin'] = FALSE;
    $options['defaults']['default']['row_options'] = FALSE;

    // Set the display title to an empty string (not used in this display type).
    $options['title']['default'] = '';
    $options['defaults']['default']['title'] = FALSE;

    return $options;
  }

  function get_style_type() {
    return 'entityreference';
  }

  function execute() {
    return $this->view->render($this->display->id);
  }

  function render() {
    if (!empty($this->view->result) || !empty($this->view->style_plugin->definition['even empty'])) {
      return $this->view->style_plugin->render($this->view->result);
    }
    return '';
  }

  function uses_exposed() {
    return FALSE;
  }

  function query() {
    $options = $this->get_option('entityreference_options');

    // Play nice with Views UI 'preview' : if the view is not executed through
    // EntityReference_SelectionHandler_Views::getReferencableEntities(),
    // don't alter the query.
    if (empty($options)) {
      return;
    }

    // Make sure the id field is included in the results, and save its alias
    // so that references_plugin_style can retrieve it.
    $this->id_field_alias = $id_field = $this->view->query->add_field($this->view->base_table, $this->view->base_field);
    if (strpos($id_field, '.') === FALSE) {
      $id_field = $this->view->base_table . '.' . $this->id_field_alias;
    }

    // Restrict the autocomplete options based on what's been typed already.
    if (isset($options['match'])) {
      $style_options = $this->get_option('style_options');
      $value = db_like($options['match']) . '%';
      if ($options['match_operator'] != 'STARTS_WITH') {
        $value = '%' . $value;
      }

      // Multiple search fields are OR'd together
      $conditions = db_or();

      // Build the condition using the selected search fields
      foreach ($style_options['search_fields'] as $field_alias) {
        if (!empty($field_alias)) {
          // Get the table and field names for the checked field
          $field = $this->view->query->fields[$this->view->field[$field_alias]->field_alias];
          // Add an OR condition for the field
          $conditions->condition($field['table'] . '.' . $field['field'], $value, 'LIKE');
        }
      }

      $this->view->query->add_where(NULL, $conditions);
    }

    // Add an IN condition for validation.
    if (!empty($options['ids'])) {
      $this->view->query->add_where(NULL, $id_field, $options['ids']);
    }

    $this->view->set_items_per_page($options['limit']);
  }

  /**
   * Extend the default validation.
   */
  function validate() {
    $errors = parent::validate();
    // Verify that search fields are set up.
    $style_options = $this->get_option('style_options');
    if (!isset($style_options['search_fields'])) {
      $errors[] = t('Display "@display" needs a selected search fields to work properly. See the settings for the Entity Reference list format.', array('@display' => $this->display->display_title));
    }
    else {
      // Verify that the search fields used actually exist.
      //$fields = array_keys($this->view->get_items('field'));
      $fields = array_keys($this->handlers['field']);
      foreach ($style_options['search_fields'] as $field_alias => $enabled) {
        if ($enabled && !in_array($field_alias, $fields)) {
          $errors[] = t('Display "@display" uses field %field as search field, but the field is no longer present. See the settings for the Entity Reference list format.', array('@display' => $this->display->display_title, '%field' => $field_alias));
        }
      }
    }
    return $errors;
  }
}
