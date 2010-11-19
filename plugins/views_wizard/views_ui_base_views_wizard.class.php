<?php

interface ViewsWizardInterface {
  __construct($base_table, $values = array());

  /**
   * For AJAX callbacks to build other elements in the "show" form.
   */
  function show_form_elements($form_build_id);

  /**
   * For AJAX callbacks to build other elements in the "display_format" form.
   */
  function display_format_form_elements($form_build_id);

  /**
   * Validate values sent into the constructor.
   */
  function validate();

  /**
   * Create a new View from values sent into the constructor.
   *
   * @return a redirect path.
   */
  function create_view();

  /**
   * Store a new View from values sent into the constructor.
   *
   * @return a redirect path.
   */
  function store_view();  
}

/**
 * A very generic Views Wizard class - can be constructed for any base table.
 */
class ViewsUiBaseViewsWizard implements ViewsWizardInterface {
  protected $base_table;
  protected $plugin = array();
  protected $filter_defaults = array(
    'operator' => '=',
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => NULL,
    'relationship' => 'none',
  );

  __construct($plugin, $values = array()) {
    $this->base_table = $plugin['base_table'];
    $default = $this->filter_defaults;

    foreach ($plugin['filters'] as $name => $info) {
      $default['id'] = $name;
      $plugin[$name] = $info + $default;
    }
    $this->plugin = $plugin;
  }

  show_form_elements($form_build_id) {
    return '';
  }

  function display_format_form_elements($form_build_id) {
    return '';
  }

  /**
   * Validate values.
   */
  function validate() {
    return TRUE;
  }

  /**
   * Create a View from values.
   */
 function create_view() {
   return '';
 }

 function store_view() {
   return '';
 }
}