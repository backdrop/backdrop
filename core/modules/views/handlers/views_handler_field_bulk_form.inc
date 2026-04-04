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
    $options['enable_select_all_pages'] = array('default' => TRUE);
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

    $form['enable_select_all_pages'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable "Select all items on all pages"'),
      '#default_value' => $this->options['enable_select_all_pages'],
      '#description' => t('Check this box to enable the ability to select all items on all pages.'),
    );
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
    $form['#attached']['js'][] = 'core/misc/tableselect.js';

    // Render checkboxes for all rows.
    $form[$this->options['id']]['#tree'] = TRUE;
    foreach ($this->view->result as $row_index => $row) {
      $form[$this->options['id']][$row_index] = array(
        '#type' => 'checkbox',
        // We are not able to determine a main "title" for each row, so we can
        // only output a generic label.
        '#title' => t('Update this item'),
        '#title_display' => 'invisible',
        '#return_value' => $row->{$this->field_alias},
        '#default_value' => !empty($form_state['values'][$this->options['id']][$row_index]) ? 1 : NULL,
        '#attributes' => array('data-tableselect-id' => $this->options['id'])
      );
    }

    // Ensure a consistent container for filters/operations in the view header.
    $form['header'] = array(
      '#weight' => -100,
    );

    // Build the bulk operations action widget for the header.
    $bulk_options = $this->get_bulk_options();
    if (!empty($bulk_options)) {
      // Allow themes to apply .container-inline on this separate container.
      $form['header'][$this->options['id']] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('views-bulk-form')),
      );

      $form['header'][$this->options['id']]['action'] = array(
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#type' => 'select',
        '#options' => $bulk_options,
        '#empty_option' => '- ' . t('Choose an action') . ' -',
        '#required' => TRUE,
      );

      // Set the form actions container in the header.
      $actions['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Execute'),
      );
      $form['header'][$this->options['id']]['actions'] = $actions;

      // If the view has results, add the form element and markup to select all.
      if (!empty($this->view->result)) {
        $form['#attached']['js'][] = backdrop_get_path('module', 'views') . '/js/bulk_form.js';
        // Set by JS to indicate that all rows on all pages are selected.
        $form['select_all_pages'] = array(
          '#type' => 'hidden',
          '#attributes' => array('class' => 'views-select-all-pages--value'),
          '#default_value' => FALSE,
        );
        $enable_select_all_pages = FALSE;
        // If the view is paginated, and "select all items on all pages" is
        // enabled, tell that to the theme function.
        if ($this->options['enable_select_all_pages'] && isset($this->view->total_rows) && count($this->view->result) != $this->view->total_rows) {
          $enable_select_all_pages = TRUE;
        }
        $form['select_all_pages_markup'] = array(
          '#type' => 'markup',
          '#markup' => theme('views_select_all_pages', array(
            'view' => $this->view,
            'enable_select_all_pages' => $enable_select_all_pages,
          )),
        );
      }
    }
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
      $action_name = $form_state['values']['action'];
      $action_info = actions_get_info($form_state['values']['action']);
      $count = 0;

      // If the flag to select all results in all pages is true, retrieve all
      // entity IDs.
      if ($form_state['values']['select_all_pages'] && ($this->view->query->pager->has_more_records() || $this->view->query->pager->get_current_page() > 0)) {
        // We need to build a new view with the same parameters, but without a
        // pager.
        $new_view = views_get_view($this->view->name);
        $new_view->set_exposed_input($this->view->get_exposed_input());
        $new_view->set_arguments($this->view->args);
        $new_view->set_display($this->view->current_display);
        // Remove the pager to get all results.
        $new_view->display_handler->set_option('pager', array(
          'type' => 'none',
          'options' => array(),
        ));
        $new_view->build();
        $new_view->execute($this->view->current_display);
        $selected = array();
        // Retrieve all entity IDs.
        foreach ($new_view->result as $result) {
          $selected[] = $result->{$this->real_field};
        }
      }
      else {
        // Filter only selected checkboxes.
        $selected = array_filter($form_state['values'][$this->options['id']]);
      }

      if (!empty($selected)) {
        $entity_type = $this->get_entity_type();
        $entities = entity_load($entity_type, $selected);
        $context = array();
        foreach ($entities as $entity) {
          if (actions_execute($action_name, $entity, $context) !== FALSE) {
            $count++;
          }
        }

        // Redirect the user the same as we would for $form_state['redirect'].
        if (isset($context['redirect'])) {
          actions_redirect($context['redirect']);
        }
      }
      else {
        backdrop_set_message(t('No items selected.'), 'error');
      }

      if ($count) {
        backdrop_set_message(format_plural($count, '%action was applied to @count item.', '%action was applied to @count items.', array('%action' => $action_info['label'])));
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
    $actions = actions_get_info();

    $entity_type = $this->get_entity_type();
    $options = array();
    // Filter the action list.
    foreach ($actions as $action_name => $action_info) {
      if ($filtered) {
        $in_selected = in_array($action_name, $this->options['selected_actions']);
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
      if ($action_info['type'] == $entity_type) {
        $options[$action_name] = $action_info['label'];
      }
    }

    return $options;
  }

  /**
   * Determine the entity type of the bulk operation field.
   */
  protected function get_entity_type() {
    $base_table = isset($this->table) ? $this->table : $this->view->base_table;
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
