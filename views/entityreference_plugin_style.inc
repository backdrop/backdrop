<?php

/**
 * @file
 * Handler for entityreference_plugin_style.
 */
class entityreference_plugin_style extends views_plugin_style {

  function option_definition() {
    $options = parent::option_definition();
    $options['search_fields'] = array('default' => NULL);

    return $options;
  }

  // Create the options form.
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $options = array();

    if (isset($form['grouping'])) {
      $options = $form['grouping'][0]['field']['#options'];
      unset($options['']);
      $form['search_fields'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Search fields'),
        '#options' => $options,
        '#required' => TRUE,
        '#default_value' => $this->options['search_fields'],
        '#description' => t('Select the field(s) that will be searched when using the autocomplete widget.'),
        '#weight' => -3,
      );
    }
  }

  function render() {
    $options = $this->display->handler->get_option('entityreference_options');

    // Play nice with Views UI 'preview' : if the view is not executed through
    // EntityReference_SelectionHandler_Views::getReferencableEntities(), just
    // display the HTML.
    if (empty($options)) {
      return parent::render();
    }

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);
    // Grab the alias of the 'id' field added by entityreference_plugin_display.
    $id_field_alias = $this->display->handler->id_field_alias;

    // @todo We don't display grouping info for now. Could be useful for select
    // widget, though.
    $results = array();
    foreach ($sets as $records) {
      foreach ($records as $index => $values) {
        $this->view->row_index = $index;
        // Sanitize html, remove line breaks and extra whitespace.
        $results[$values->{$id_field_alias}] = filter_xss_admin(preg_replace('/\s\s+/', ' ', str_replace("\n", '', $this->row_plugin->render($values))));
      }
    }
    unset($this->view->row_index);
    return $results;
  }
}
