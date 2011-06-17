<?php

/**
 * @file
 * Views area handlers. Insert a view inside of an area.
 */
class views_handler_area_view extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();

    $options['view_to_insert'] = array('default' => '');
    $options['inherit_arguments'] = array('default' => FALSE, 'boolean' => TRUE);
    return $options;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $views = views_get_all_views();
    foreach ($views as $view) {
      // Exclude the current view
      foreach ($view->display as $display_id => $display) {
        // Exclude the current display
        if (!($view->name == $this->view->name && $display_id == $this->view->current_display)) {
          $options[$view->name . ':' . $display->id] = t('View: @view Display: @display', array('@view' => $view->name, '@display' => $display->id));
        }
      }
    }

    $form['view_to_insert'] = array(
      '#type' => 'select',
      '#title' => t('View to insert'),
      '#default_value' => $this->options['view_to_insert'],
      '#description' => t('The view to insert into this area.'),
      '#options' => $options
    );

    $form['inherit_arguments'] = array(
      '#type' => 'checkbox',
      '#title' => t('Inherit contextual filters'),
      '#default_value' => $this->options['inherit_arguments'],
      '#description' => t('If checked, this view will receive the same contextual filters as its parent.'),
    );
  }

  /**
   * Render the area
   */
  function render($empty = FALSE) {
    if (!empty($this->options['view_to_insert'])) {
      list($view_name, $view_display) = explode(':', $this->options['view_to_insert']);

      $view = views_get_view($view_name);
      if (empty($view)) {
        return;
      }
      $view->set_display($view_display);

      // Avoid recursion
      $view->parent_views += $this->view->parent_views;
      $view->parent_views[] = "$view_name:$view_display";

      // Check if the view is part of the parent views of this view
      $search = "$view_name:$view_display";
      if (in_array($search, $this->view->parent_views)) {
        drupal_set_message(t("Recursion detected in view @view display @display.", array('@view' => $view_name, '@display' => $view_display)), 'error');
      }
      else {
        if (!empty($this->options['inherit_arguments']) && !empty($this->view->args)) {
          return $view->preview($view_display, $this->view->args);
        }
        else {
          return $view->preview($view_display);
        }
      }
    }
    return '';
  }
}
