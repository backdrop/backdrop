<?php

/**
 * @file
 * Definition of views_handler_field_bulk_form.
 */

/**
 * Defines a actions-based bulk operation form element.
 *
 * This handler is not exposed directly in the UI, instead individual entity
 * types that support bulk operations should extend this class and expose their
 * own implementation via hook_views_data().
 */
class views_handler_field_bulk_form extends views_handler_field {

  /**
   * Overrides views_handler_field::option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['include_exclude'] = array(
      'default' => 'exclude',
    );
    $options['selected_actions'] = array(
      'default' => array(),
    );
    return $options;
  }

  /**
   * Overrides views_handler_field::options_form().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['include_exclude'] = array(
      '#type' => 'radios',
      '#title' => t('Available actions'),
      '#options' => array(
        'exclude' => t('All actions, except selected'),
        'include' => t('Only selected actions'),
      ),
      '#default_value' => $this->options['include_exclude'],
    );
    $form['selected_actions'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Selected actions'),
      '#options' => $this->get_bulk_options(FALSE),
      '#default_value' => $this->options['selected_actions'],
    );
  }

  /**
   * Overrides views_handler::options_validate().
   */
  function options_validate(&$form, &$form_state) {
    parent::options_validate($form, $form_state);

    $form_state['values']['options']['selected_actions'] = array_filter($form_state['values']['options']['selected_actions']);
  }

  /**
   * Overrides views_handler_field::render().
   */
  function render($values) {
    return '<!--form-item-' . $this->options['id'] . '--' . $this->view->row_index . '-->';
  }

  /**
   * Overrides views_handler_field::pre_render().
   */
  function pre_render(&$values) {
    parent::pre_render($values);

    // If the view is using a table style, provide a placeholder for a
    // "select all" checkbox.
    if (!empty($this->view->style_plugin) && $this->view->style_plugin instanceof views_plugin_style_table) {
      // Add the tableselect css classes.
      $this->options['element_label_class'] .= 'select-all';
      // Hide the actual label of the field on the table header.
      $this->options['label'] = '';
    }
  }

  /**
   * Form constructor for the bulk form.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   */
  function views_form(&$form, &$form_state) {
    // Add the tableselect javascript.
    $form['#attached']['library'][] = array('system', 'drupal.tableselect');

    // Render checkboxes for all rows.
    $form[$this->options['id']]['#tree'] = TRUE;
    foreach ($this->view->result as $row_index => $row) {
      $form[$this->options['id']][$row_index] = array(
        '#type' => 'checkbox',
        // We are not able to determine a main "title" for each row, so we can
        // only output a generic label.
        '#title' => t('Update this item'),
        '#title_display' => 'invisible',
        '#default_value' => !empty($form_state['values'][$this->options['id']][$row_index]) ? 1 : NULL,
      );
    }

    // Replace the form submit button label.
    $form['actions']['submit']['#value'] = t('Apply');

    // Ensure a consistent container for filters/operations in the view header.
    $form['header'] = array(
      '#type' => 'container',
      '#weight' => -100,
    );

    // Build the bulk operations action widget for the header.
    // Allow themes to apply .container-inline on this separate container.
    $form['header'][$this->options['id']] = array(
      '#type' => 'container',
    );
    $form['header'][$this->options['id']]['action'] = array(
      '#type' => 'select',
      '#title' => t('With selection'),
      '#options' => $this->get_bulk_options(),
    );

    // Duplicate the form actions into the action container in the header.
    $form['header'][$this->options['id']]['actions'] = $form['actions'];
  }

  /**
   * Submit handler for the bulk form.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   */
  function views_form_submit(&$form, &$form_state) {
    if ($form_state['step'] == 'views_form_views_form') {
      $action = $form_state['values']['action'];
      $action = actions_load($action);
      $count = 0;

      // Filter only selected checkboxes.
      $selected = array_filter($form_state['values'][$this->options['id']]);

      if (!empty($selected)) {
        $entity_type = $this->get_entity_type();
        $entities = entity_load($entity_type, $selected);
        foreach ($entities as $entity) {
          $entity = entity_create($entity_type, (array) $entity);

          actions_do($action->aid, $entity);
          $entity->save();
          //$storage = new EntityDatabaseStorageController($entity_type);
          //$storage->save($entity);

          $count++;
        }
      }

      if ($count) {
        drupal_set_message(format_plural($count, '%action was applied to @count item.', '%action was applied to @count items.', array(
          '%action' => $action->label,
        )));
      }
    }
  }

  /**
   * Returns the available operations for this form.
   *
   * @param bool $filtered
   *   (optional) Whether to filter actions to selected actions.
   *
   * @return array
   *   An associative array of operations, suitable for a select element.
   */
  protected function get_bulk_options($filtered = TRUE) {
    // Get all available actions.
    $actions = actions_get_all_actions();
    $entity_type = $this->get_entity_type();
    $options = array();
    // Filter the action list.
    foreach ($actions as $id => $action) {
      if ($filtered) {
        $in_selected = in_array($id, $this->options['selected_actions']);
        // If the field is configured to include only the selected actions,
        // skip actions that were not selected.
        if (($this->options['include_exclude'] == 'include') && !$in_selected) {
          continue;
        }
        // Otherwise, if the field is configured to exclude the selected
        // actions, skip actions that were selected.
        elseif (($this->options['include_exclude'] == 'exclude') && $in_selected) {
          continue;
        }
      }
      // Only allow actions that are valid for this entity type.
      if (($action['type'] == $entity_type) && empty($action['configurable'])) {
        $options[$id] = $action['label'];
      }
    }

    return $options;
  }

  /**
   * Determine the base table of the bulk operation field.
   */
  protected function get_entity_type() {
    $base_table = $this->view->base_table;

    // If the current field is under a relationship you can't be sure that the
    // base table of the view is the base table of the current field.
    // For example a field from a node author on a node view does have users as base table.
    if (!empty($this->options['relationship']) && $this->options['relationship'] != 'none') {
      $relationships = $this->view->display_handler->get_option('relationships');
      $options = $relationships[$this->options['relationship']];
      $data = views_fetch_data($options['table']);
      $base_table = $data[$options['field']]['relationship']['base'];
    }
    // The base table is now known, use it to determine the entity type.
    foreach (entity_get_info() as $entity_type => $info) {
      if (isset($info['base table']) && $info['base table'] == $base_table) {
        return $entity_type;
      }
      elseif (isset($info['revision table']) && $info['revision table'] == $base_table) {
        $this->revision = TRUE;
        return $entity_type;
      }
    }
    return FALSE;
  }

}
