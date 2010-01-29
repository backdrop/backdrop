<?php
// $Id$

/**
 * The base plugin to handle exposed filter forms.
 */
class views_plugin_exposed_form extends views_plugin {

  /**
   * Initialize the plugin.
   *
   * @param $view
   *   The view object.
   * @param $display
   *   The display handler.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;

    $exposed_form = $display->handler->get_option('exposed_form');
    $this->unpack_options($this->options, $exposed_form['options']);
  }

  /**
   * Return a string to display as the clickable title for the
   * control.
   */
  function summary_title() {
    return t('Unknown');
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['submit_button'] = array('default' => t('Apply'), 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['submit_button'] = array(
      '#type' => 'textfield',
      '#title' => t('Submit button text'),
      '#description' => t('Text to display in the submit button of the exposed form.'),
      '#default_value' => $this->options['submit_button'],
    );
  }

  /**
   * Render the exposed filter form.
   *
   * This actually does more than that; because it's using FAPI, the form will
   * also assign data to the appropriate handlers for use in building the
   * query.
   */
  function render_exposed_form($block = FALSE) {
    // Deal with any exposed filters we may have, before building.
    $form_state = array(
      'view' => &$this->view,
      'display' => &$this->display,
      'method' => 'get',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      'submit_button' => $this->options['submit_button'],
    );

    // Some types of displays (eg. attachments) may wish to use the exposed
    // filters of their parent displays instead of showing an additional
    // exposed filter form for the attachment as well as that for the parent.
    if (!$this->view->display_handler->displays_exposed() || (!$block && $this->view->display_handler->get_option('exposed_block'))) {
      unset($form_state['rerender']);
    }

    if (!empty($this->ajax)) {
      $form_state['ajax'] = TRUE;
    }

    $form_state['exposed_form_plugin'] = $this;
    $output = drupal_render(drupal_build_form('views_exposed_form', $form_state));
    if (!empty($form_state['js settings'])) {
      $this->view->js_settings = $form_state['js settings'];
    }

    return $output;
  }

  function query() { }

  function pre_render() { }

  function post_render(&$output) { }

  function pre_execute() { }

  function post_execute() { }

  function exposed_form_alter(&$form, &$form_state) { }
  
  function exposed_form_validate(&$form, &$form_state) { }
  
  /**
  * This function is executed when exposed form is submited.
  *
  * @param $form
  *   Nested array of form elements that comprise the form.
  * @param $form_state
  *   A keyed array containing the current state of the form.
  * @param $exclude
  *   Nested array of keys to exclude of insert into 
  *   $view->exposed_raw_input
  */ 
  function exposed_form_submit(&$form, &$form_state, &$exclude) { }
}
