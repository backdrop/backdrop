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

    $this->build_filters($form, $form_state);

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

  /**
   * Build the part of the form that allows the user to select the view's filters.
   *
   * By default, this adds two filters (when they are available), "tagged with"
   * and "sorted by [date]".
   */
  protected function build_filters(&$form, &$form_state) {
    // Find all the fields we are allowed to filter by.
    $fields = views_fetch_fields($this->base_table, 'filter');

    // Check if we are allowed to filter by taxonomy. We will construct our
    // filters using taxonomy_index.tid (which limits the filtering to a
    // specific vocabulary) rather than taxonomy_term_data.name (which matches
    // terms in any vocabulary) because it is a more commonly-used filter that
    // works better with the autocomplete UI, and also to avoid confusion with
    // other vocabularies on the site that may have terms with the same name
    // but are not used for free tagging. The downside is that if there *is*
    // more than one vocabulary on the site that is used for free tagging, the
    // wizard will only be able to make the "tagged with" filter apply to one
    // of them (see below).
    if (isset($fields['taxonomy_index.tid'])) {
      // Check if this view will be displaying fieldable entities.
      $entities = entity_get_info();
      $displays_entities = FALSE;
      foreach ($entities as $entity_type => $entity_info) {
        if ($this->base_table == $entity_info['base table']) {
          $displays_entities = TRUE;
          // $entity_type and $entity_info will now store information about the
          // type of entity this view can display.
          break;
        }
      }
      if ($displays_entities && $entity_info['fieldable']) {
        // Find all "tag-like" taxonomy fields associated with the view's
        // entities. If the plugin has already added filters that will restrict
        // the selected entities to certain bundles, then we only search for
        // taxonomy fields associated with those bundles. Otherwise, we use all
        // bundles (for example, if we are filtering by "All content").
        $bundles = isset($this->plugin['bundles']) ? array_intersect($this->plugin['bundles'], array_keys($entity_info['bundles'])) : array_keys($entity_info['bundles']);
        $tag_fields = array();
        foreach ($bundles as $bundle) {
          foreach (field_info_instances($entity_type, $bundle) as $instance) {
            // We define "tag-like" taxonomy fields as ones that use the
            // "Autocomplete term widget (tagging)" widget.
            if ($instance['widget']['type'] == 'taxonomy_autocomplete') {
              $tag_fields[] = $instance['field_name'];
            }
          }
        }
        $tag_fields = array_unique($tag_fields);
        if (!empty($tag_fields)) {
          // If there is more than one "tag-like" taxonomy field available to
          // the view, we can only make our filter apply to one of them (as
          // described above). We choose 'field_tags' if it is available, since
          // that is created by the Standard install profile in core and also
          // commonly used by contrib modules; thus, it is most likely to be
          // associated with the "main" free-tagging vocabulary on the site.
          if (in_array('field_tags', $tag_fields)) {
            $tag_field_name = 'field_tags';
          }
          else {
            $tag_field_name = reset($tag_fields);
          }
          // Add the "tagged with" autocomplete textfield.
          $form['displays']['show']['tagged_with'] = array(
            '#type' => 'textfield',
            '#title' => t('tagged with'),
            '#autocomplete_path' => 'taxonomy/autocomplete/' . $tag_field_name,
            '#size' => 30,
            '#maxlength' => 1024,
            '#field_name' => $tag_field_name,
            '#element_validate' => array('views_ui_taxonomy_autocomplete_validate'),
          );
        }
      }
    }
  }

  protected function instantiate_view($form, &$form_state) {
    $view = views_new_view();
    $view->name = $form_state['values']['name'];
    $view->human_name = $form_state['values']['human_name'];
    $view->tag = 'default';
    $view->core = VERSION;
    $view->base_table = $this->base_table;

    // Display: Defaults
    $handler = $view->new_display('default', 'Defaults', 'default');
    $handler->display->display_options = $this->default_display_options($form, $form_state);
    if (!isset($handler->display->display_options['filters'])) {
      $handler->display->display_options['filters'] = array();
    }
    $handler->display->display_options['filters'] += $this->default_display_filters($form, $form_state);

    // Display: Page
    if (!empty($form_state['values']['page']['create'])) {
      $handler = $view->new_display('page', 'Page', 'page');
      $handler->display->display_options = $this->page_display_options($form, $form_state);
      if (!empty($form_state['values']['page']['feed'])) {
        $handler = $view->new_display('feed', 'Feed', 'feed_page');
        $handler->display->display_options = $this->page_feed_display_options($form, $form_state);
      }
    }

    // Display: Block
    if (!empty($form_state['values']['block']['create'])) {
      $handler = $view->new_display('block', 'Block', 'block');
      $handler->display->display_options = $this->block_display_options($form, $form_state);
      if (!empty($form_state['values']['block']['feed'])) {
        $handler = $view->new_display('feed', 'Block feed', 'feed_block');
        $handler->display->display_options = $this->block_feed_display_options($form, $form_state);
      }
    }
    return $view;
  }

  /**
   * Most subclasses will need to override this method to provide some fields
   * or a different row plugin.
   */
  protected function default_display_options($form, $form_state) {
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

  protected function default_display_filters($form, $form_state) {
    $filters = array();

    // Add the filters provided by the plugin.
    foreach ($this->plugin['filters'] as $name => $info) {
      $filters[$name] = $info;
    }

    // Add any filters specified by the user when filling out the wizard.
    if (!empty($form_state['values']['show']['tagged_with']['tids'])) {
      $filters['tid'] = array(
        'id' => 'tid',
        'table' => 'taxonomy_index',
        'field' => 'tid',
        'value' => $form_state['values']['show']['tagged_with']['tids'],
        'vocabulary' => $form_state['values']['show']['tagged_with']['vocabulary'],
      );
      // If the user entered more than one valid term in the autocomplete
      // field, they probably intended both of them to be applied.
      if (count($form_state['values']['show']['tagged_with']['tids']) > 1) {
        $filters['tid']['operator'] = 'and';
      }
    }

    return $filters;
  }

  protected function page_display_options($form, $form_state) {
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

  protected function page_feed_display_options($form, $form_state) {
    $display_options = array();
    return $display_options;
  }

  protected function block_display_options($form, $form_state) {
    $display_options = array();
    return $display_options;
  }

  protected function block_feed_display_options($form, $form_state) {
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
