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
   *
   * @throws ViewsWizardException in the event of a problem.
   */
  function create_view($form, &$form_state);
}

/**
 * A custom exception class for our errors.
 */
class ViewsWizardException extends Exception {
}

/**
 * A very generic Views Wizard class - can be constructed for any base table.
 */
class ViewsUiBaseViewsWizard implements ViewsWizardInterface {
  protected $base_table;
  protected $validated_views = array();
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
    $style_options = views_fetch_plugin_names('style', 'normal', array($this->base_table));
    $feed_row_options = views_fetch_plugin_names('row', 'feed', array($this->base_table));
    // Temporary markup to monitor effect of form updates.
    // The inline dynamic elements will go here.
    $form['show']['base_table'] = array(
      '#markup' => '<div style="float: right">Base table: ' . $this->base_table . '</div>',
      '#prefix' => '<div id="edit-view-ajax-wrapper">',
      '#suffix' => '</div>',
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
      '#options' => $style_options,
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
    // Only offer a feed if we have at least one available feed row style.
    if ($feed_row_options) {
      $form['page']['feed'] = array(
        '#title' => t('Include an RSS feed'),
        '#type' => 'checkbox',
        '#states' => array(
          'visible' => array(
            ':input[name="page[create]"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['page']['feed_properties']['path'] = array(
        '#title' => t('Feed path'),
        '#type' => 'textfield',
        '#states' => array(
          'visible' => array(
            ':input[name="page[create]"]' => array('checked' => TRUE),
            ':input[name="page[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
      // This will almost never be visible.
      $form['page']['feed_properties']['row_style'] = array(
        '#title' => t('Feed row style'),
        '#type' => 'select',
        '#options' => $feed_row_options,
        '#default_value' => reset($feed_row_options),
        '#access' => (count($feed_row_options) > 1),
        '#states' => array(
          'visible' => array(
            ':input[name="page[create]"]' => array('checked' => TRUE),
            ':input[name="page[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
    }
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
    // Only offer a feed if we have at least one available feed row style.
    if ($feed_row_options) {
      $form['block']['feed'] = array(
        '#title' => t('Include an RSS feed'),
        '#type' => 'checkbox',
        '#states' => array(
          'visible' => array(
            ':input[name="block[create]"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['block']['feed_properties']['path'] = array(
        '#title' => t('Feed path'),
        '#type' => 'textfield',
        '#states' => array(
          'visible' => array(
            ':input[name="block[create]"]' => array('checked' => TRUE),
            ':input[name="block[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
      // This will almost never be visible.
      $form['block']['feed_properties']['row_style'] = array(
        '#title' => t('Feed row style'),
        '#type' => 'select',
        '#options' => $feed_row_options,
        '#default_value' => reset($feed_row_options),
        '#access' => (count($feed_row_options) > 1),
        '#states' => array(
          'visible' => array(
            ':input[name="block[create]"]' => array('checked' => TRUE),
            ':input[name="block[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
    }
    return $form;
  }

  protected function instantiate_view($from, &$form_state) {
    $view = views_new_view();
    $view->name = $form_state['values']['name'];
    $view->description = $form_state['values']['description'];
    $view->tag = 'default';
    $view->base_table = $this->base_table;

    /* Display: Defaults */
    $handler = $view->new_display('default', 'Defaults', 'default');
    $handler->display->display_options['access']['type'] = 'none';
    $handler->display->display_options['cache']['type'] = 'none';
    $handler->display->display_options['query']['type'] = 'views_query';
    $handler->display->display_options['exposed_form']['type'] = 'basic';
    $handler->display->display_options['pager']['type'] = 'full';
    $handler->display->display_options['style_plugin'] = 'default';
    return $view;
  }

  protected function retrieve_validated_view($form, $form_state, $unset = TRUE) {
    $key = hash('sha256', serialize($form_state['values']));
    $view (isset($this->validated_views[$key]) ? $this->validated_views[$key] : NULL);
    if ($unset) {
      unset($this->validated_views[$key]);
    }
    return $view;
  }

  protected function set_validated_view($form, $form_state, $view) {
    $key = hash('sha256', serialize($form_state['values']));
    $this->validated_views[$key] = $view;
  }

  /**
   * Instantiates a view and validates values.
   */
  function validate($form, &$form_state) {
    $view = $this->instantiate_view($from, $form_state);
    $errors = $view->validate();
    if (!$errors) {
      $this->set_validated_view($form, $form_state, $view);
    }
    return $errors;
  }

  /**
   * Create a View from values that have been already submitted to validate().
   *
   * @throws ViewsWizardException if the values have not been validated.
   */
 function create_view($form, &$form_state) {
   $view = $this->retrieve_validated_view($form, $form_state);
   if (empty($view)) {
     throw new ViewsWizardException(t('Attempted to create_view with values that have not been validated'));
   }
   return $view;
 }

}
