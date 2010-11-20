<?php

interface ViewsWizardInterface {
  function __construct($plugin);

  /**
   * For AJAX callbacks to build other elements in the "show" form.
   */
  function build_form($form, &$form_state);

  /**
   * Validate form and values.
   *
   * @return an array of form errors.
   */
  function validate($form, &$form_state);

  /**
   * Create a new View from form values.
   *
   * @return a view object.
   */
  function create_view($form, &$form_state);
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

  function __construct($plugin) {
    $this->base_table = $plugin['base_table'];
    $default = $this->filter_defaults;

    foreach ($plugin['filters'] as $name => $info) {
      $default['id'] = $name;
      $plugin[$name] = $info + $default;
    }
    $this->plugin = $plugin;
  }

  function build_form($form, &$form_state) {
    // Temporary markup to monitor effect of form updates.
    // The inline dynamic elements will go here.
    $form['show']['base_table'] = array(
      '#markup' => '<div style="float: right">Base table: ' . $this->base_table . '</div>',
    );
    $form['page'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    $form['page']['create'] = array(
      '#title' => t('Create a page'),
      '#type' => 'checkbox',
      '#attributes' => array('class' => array('strong')),
    );
    $form['page']['title'] = array(
      '#title' => t('Page title'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['path'] = array(
      '#title' => t('Path'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    // This may change by AJAX as we change the base table of the selected wizard.
    $form['page']['display_format']['style'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => views_fetch_plugin_names('style', 'normal', array($this->base_table)),
      '#default_value' => 'default',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#size' => 5,
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['link'] = array(
      '#title' => t('Create a menu link'),
      '#type' => 'checkbox',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['link_properties'] = array();
    if (module_exists('menu')) {
      $menu_options = menu_get_menus();
    }
    else {
      // These are not yet translated.
      $menu_options = menu_list_system_menus();
      foreach ($menu_options as $name => $title) {
        $menu_options[$name] = t($title);
      }
    }
    $form['page']['link_properties']['menu_name'] = array(
      '#title' => t('Menu'),
      '#type' => 'select',
      '#options' => $menu_options,
      '#states' => array(
        'visible' => array(
        ':input[name="page[link]"]' => array('checked' => TRUE),
        ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['link_properties']['title'] = array(
      '#title' => t('Link text'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
        ':input[name="page[link]"]' => array('checked' => TRUE),
        ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['rss'] = array(
      '#title' => t('Include an RSS feed'),
      '#type' => 'checkbox',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['rss_properties']['path'] = array(
      '#title' => t('RSS path'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
          ':input[name="page[rss]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['block'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    $form['block']['create'] = array(
      '#title' => t('Create a block'),
      '#type' => 'checkbox',
      '#attributes' => array('class' => array('strong')),
    );
    $form['block']['title'] = array(
      '#title' => t('Block title'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="block[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    // This may change by AJAX as we change the base table of the selected wizard.
    $form['block']['display_format']['style'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => views_fetch_plugin_names('style', 'normal', array($this->base_table)),
      '#default_value' => 'default',
      '#states' => array(
        'visible' => array(
          ':input[name="block[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['block']['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#size' => 5,
      '#states' => array(
        'visible' => array(
          ':input[name="block[create]"]' => array('checked' => TRUE),
        ),
      ),
    );
    return $form;
  }

  /**
   * Validate values.
   */
  function validate($from, &$form_state) {
    return array();
  }

  /**
   * Create a View from values.
   */
 function create_view($from, &$form_state) {
   return NULL;
 }

}