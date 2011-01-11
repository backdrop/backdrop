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
    'id' => NULL,
    'expose' => array('operator' => FALSE),
    'group' => 0,
  );

  function __construct($plugin) {
    $this->base_table = $plugin['base_table'];
    $default = $this->filter_defaults;

    foreach ($plugin['filters'] as $name => $info) {
      $default['id'] = $name;
      $plugin['filters'][$name] = $info + $default;
    }
    $this->plugin = $plugin;
  }

  function build_form($form, &$form_state) {
    $style_options = views_fetch_plugin_names('style', 'normal', array($this->base_table));
    $feed_row_options = views_fetch_plugin_names('row', 'feed', array($this->base_table));
    $path_prefix = url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q=');

    $form['displays']['page'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    // Temporary markup to monitor effect of form updates.
    $form['displays']['page']['base_table'] = array(
      '#markup' => '<div style="float: right">Base table: ' . $this->base_table . '<br /> Wizard plugin: '. $this->plugin['name'] .'</div>',
    );
    $form['displays']['page']['create'] = array(
      '#title' => t('Create a page'),
      '#type' => 'checkbox',
      '#attributes' => array('class' => array('strong')),
    );

    // All options for the page display are included in this container so they
    // can be hidden en masse when the "Create a page" checkbox is unchecked.
    $form['displays']['page']['options'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="page[create]"]' => array('checked' => TRUE),
        ),
      ),
      '#parents' => array('page'),
    );

    $form['displays']['page']['options']['title'] = array(
      '#title' => t('Page title'),
      '#type' => 'textfield',
    );
    $form['displays']['page']['options']['path'] = array(
      '#title' => t('Path'),
      '#type' => 'textfield',
      '#field_prefix' => $path_prefix,
    );
    $form['displays']['page']['options']['display_format']['style'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => $style_options,
      '#default_value' => 'default',
    );
    $form['displays']['page']['options']['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#default_value' => '10',
      '#size' => 5,
    );
    $form['displays']['page']['options']['link'] = array(
      '#title' => t('Create a menu link'),
      '#type' => 'checkbox',
    );
    $form['displays']['page']['options']['link_properties'] = array();
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
    $form['displays']['page']['options']['link_properties']['menu_name'] = array(
      '#title' => t('Menu'),
      '#type' => 'select',
      '#options' => $menu_options,
      '#states' => array(
        'visible' => array(
          ':input[name="page[link]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['displays']['page']['options']['link_properties']['title'] = array(
      '#title' => t('Link text'),
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="page[link]"]' => array('checked' => TRUE),
        ),
      ),
    );
    // Only offer a feed if we have at least one available feed row style.
    if ($feed_row_options) {
      $form['displays']['page']['options']['feed'] = array(
        '#title' => t('Include an RSS feed'),
        '#type' => 'checkbox',
      );
      $form['displays']['page']['options']['feed_properties']['path'] = array(
        '#title' => t('Feed path'),
        '#type' => 'textfield',
        '#field_prefix' => $path_prefix,
        '#states' => array(
          'visible' => array(
            ':input[name="page[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
      // This will almost never be visible.
      $form['displays']['page']['options']['feed_properties']['row_plugin'] = array(
        '#title' => t('Feed row style'),
        '#type' => 'select',
        '#options' => $feed_row_options,
        '#default_value' => key($feed_row_options),
        '#access' => (count($feed_row_options) > 1),
        '#states' => array(
          'visible' => array(
            ':input[name="page[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
    }

    $form['displays']['block'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    $form['displays']['block']['create'] = array(
      '#title' => t('Create a block'),
      '#type' => 'checkbox',
      '#attributes' => array('class' => array('strong')),
    );

    // All options for the block display are included in this container so they
    // can be hidden en masse when the "Create a block" checkbox is unchecked.
    $form['displays']['block']['options'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="block[create]"]' => array('checked' => TRUE),
        ),
      ),
      '#parents' => array('block'),
    );

    $form['displays']['block']['options']['title'] = array(
      '#title' => t('Block title'),
      '#type' => 'textfield',
    );
    // This may change by AJAX as we change the base table of the selected wizard.
    $form['displays']['block']['options']['display_format']['style'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => views_fetch_plugin_names('style', 'normal', array($this->base_table)),
      '#default_value' => 'default',
    );
    $form['displays']['block']['options']['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#default_value' => '5',
      '#size' => 5,
    );
    // Only offer a feed if we have at least one available feed row style.
    if ($feed_row_options) {
      $form['displays']['block']['options']['feed'] = array(
        '#title' => t('Include an RSS feed'),
        '#type' => 'checkbox',
      );
      $form['displays']['block']['options']['feed_properties']['path'] = array(
        '#title' => t('Feed path'),
        '#type' => 'textfield',
        '#field_prefix' => $path_prefix,
        '#states' => array(
          'visible' => array(
            ':input[name="block[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
      // This will almost never be visible.
      $form['displays']['block']['options']['feed_properties']['row_plugin'] = array(
        '#title' => t('Feed row style'),
        '#type' => 'select',
        '#options' => $feed_row_options,
        '#default_value' => key($feed_row_options),
        '#access' => (count($feed_row_options) > 1),
        '#states' => array(
          'visible' => array(
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
    $view->human_name = $form_state['values']['human_name'];
    $view->tag = 'default';
    $view->core = VERSION;
    $view->base_table = $this->base_table;

    // Display: Defaults
    $handler = $view->new_display('default', 'Defaults', 'default');
    $handler->display->display_options = $this->default_display_options($from, $form_state);
    if (!isset($handler->display->display_options['filters'])) {
      $handler->display->display_options['filters'] = array();
    }
    $handler->display->display_options['filters'] += $this->default_display_filters($from, $form_state);

    // Display: Page
    if (!empty($form_state['values']['page']['create'])) {
      $handler = $view->new_display('page', 'Page', 'page');
      $handler->display->display_options = $this->page_display_options($from, $form_state);
      if (!empty($form_state['values']['page']['feed'])) {
        $handler = $view->new_display('feed', 'Feed', 'feed_page');
        $handler->display->display_options = $this->page_feed_display_options($from, $form_state);
      }
    }

    // Display: Block
    if (!empty($form_state['values']['block']['create'])) {
      $handler = $view->new_display('block', 'Block', 'block');
      $handler->display->display_options = $this->block_display_options($from, $form_state);
      if (!empty($form_state['values']['block']['feed'])) {
        $handler = $view->new_display('feed', 'Block feed', 'feed_block');
        $handler->display->display_options = $this->block_feed_display_options($from, $form_state);
      }
    }
    return $view;
  }

  /**
   * Most subclasses will need to override this method to provide some fields
   * or a different row plugin.
   */
  protected function default_display_options($from, $form_state) {
    $display_options = array();
    $display_options['access']['type'] = 'none';
    $display_options['cache']['type'] = 'none';
    $display_options['query']['type'] = 'views_query';
    $display_options['exposed_form']['type'] = 'basic';
    $display_options['pager']['type'] = 'full';
    $display_options['style_plugin'] = 'default';
    $display_options['row_plugin'] = 'fields';
    return $display_options;
  }

  protected function default_display_filters($from, $form_state) {
    $filters = array();
    foreach ($this->plugin['filters'] as $name => $info) {
      $filters[$name] = $info;
    }
    return $filters;
  }

  protected function page_display_options($from, $form_state) {
    $display_options = array();
    $page = $form_state['values']['page'];
    $display_options['path'] = $page['path'];
    $display_options['title'] = $page['title'];
    if (!empty($page['link'])) {
      $display_options['menu']['type'] = 'normal';
      $display_options['menu']['title'] = $page['link_properties']['title'];
      $display_options['menu']['name'] = $page['link_properties']['menu_name'];
    }
    return $display_options;
  }

  protected function page_feed_display_options($from, $form_state) {
    $display_options = array();
    return $display_options;
  }

  protected function block_display_options($from, $form_state) {
    $display_options = array();
    return $display_options;
  }

  protected function block_feed_display_options($from, $form_state) {
    $display_options = array();
    return $display_options;
  }

  protected function retrieve_validated_view($form, $form_state, $unset = TRUE) {
    $key = hash('sha256', serialize($form_state['values']));
    $view = (isset($this->validated_views[$key]) ? $this->validated_views[$key] : NULL);
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
    $view = $this->instantiate_view($form, $form_state);
    $errors = $view->validate();
    if (!is_array($errors) || empty($errors)) {
      $this->set_validated_view($form, $form_state, $view);
      return array();
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
