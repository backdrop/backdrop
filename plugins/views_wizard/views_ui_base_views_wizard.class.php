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
  protected $entity_type;
  protected $entity_info = array();
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

    if (isset($plugin['filters'])) {
      foreach ($plugin['filters'] as $name => $info) {
        $default['id'] = $name;
        $plugin['filters'][$name] = $info + $default;
      }
    }

    $this->plugin = $plugin;

    $entities = entity_get_info();
    foreach ($entities as $entity_type => $entity_info) {
      if ($this->base_table == $entity_info['base table']) {
        $this->entity_info = $entity_info;
        $this->entity_type = $entity_type;
      }
    }
  }

  function build_form($form, &$form_state) {
    $style_options = views_fetch_plugin_names('style', 'normal', array($this->base_table));
    $feed_row_options = views_fetch_plugin_names('row', 'feed', array($this->base_table));
    $path_prefix = url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q=');

    // Add filters and sorts which apply to the view as a whole.
    $this->build_filters($form, $form_state);
    $this->build_sorts($form, $form_state);

    $form['displays']['page'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    $form['displays']['page']['create'] = array(
      '#title' => t('Create a page'),
      '#type' => 'checkbox',
      '#attributes' => array('class' => array('strong')),
      '#default_value' => TRUE,
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
    $form['displays']['page']['options']['style'] = array(
      '#type' => 'fieldset',
      '#attributes' => array('class' => array('container-inline')),
    );
    $form['displays']['page']['options']['style']['style_plugin'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => $style_options,
      '#default_value' => 'default',
      '#ajax' => array(
        'callback' => 'views_ui_add_form_update_style_page',
        'wrapper' => 'edit-page-style-plugin',
      ),
    );
    $this->build_form_style($form, $form_state, 'page');
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
    $form['displays']['page']['options']['link_properties'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="page[link]"]' => array('checked' => TRUE),
        ),
      ),
    );
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
    );
    $form['displays']['page']['options']['link_properties']['title'] = array(
      '#title' => t('Link text'),
      '#type' => 'textfield',
    );
    // Only offer a feed if we have at least one available feed row style.
    if ($feed_row_options) {
      $form['displays']['page']['options']['feed'] = array(
        '#title' => t('Include an RSS feed'),
        '#type' => 'checkbox',
      );
      $form['displays']['page']['options']['feed_properties'] = array(
        '#type' => 'container',
        '#states' => array(
          'visible' => array(
            ':input[name="page[feed]"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['displays']['page']['options']['feed_properties']['path'] = array(
        '#title' => t('Feed path'),
        '#type' => 'textfield',
        '#field_prefix' => $path_prefix,
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
    $form['displays']['block']['options']['style_plugin'] = array(
      '#title' => t('Display format'),
      '#help_topic' => 'style',
      '#type' => 'select',
      '#options' => $style_options,
      '#default_value' => 'default',
      '#ajax' => array(
        'callback' => 'views_ui_add_form_update_style_block',
        'wrapper' => 'edit-block-style-plugin',
      ),
    );
    $this->build_form_style($form, $form_state, 'block');
    $form['displays']['block']['options']['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#default_value' => '5',
      '#size' => 5,
    );

    return $form;
  }

  /**
   * Build the part of the form that builds the display format options.
   */
  protected function build_form_style(&$form, &$form_state, $type) {
    $style_form =& $form['displays'][$type]['options']['style'];
    $style = isset($form_state['values'][$type]) ? $form_state['values'][$type]['style']['style_plugin'] : $style_form['style_plugin']['#default_value'];
    $style_plugin = views_get_plugin('style', $style);
    $options = $this->row_style_options($type);
    $style_form['row_style'] = array(
      '#type' => 'select',
      '#title' => t('of'),
      '#options' => $options,
      '#ajax' => array(
        'wrapper' => "edit-style-$type-options-style-row-style-options",
        'callback' => 'views_ui_add_form_update_row_' . $type,
      ),
    );
    $style_form['row_style_options'] = array(
      '#type' => 'container',
      '#id' => "edit-style-$type-options-style-row-style-options",
    );
  }

  /**
   * Add possible row style options.
   *
   * Per default use fields with base field.
   */
  protected function row_style_options($type) {
    $data = views_fetch_data($this->base_table);
    return array(
      'fields' => t('Fields'),
    );
  }

  /**
   * Build the part of the form that allows the user to select the view's filters.
   *
   * By default, this adds "of type" and "tagged with" filters (when they are
   * available).
   */
  protected function build_filters(&$form, &$form_state) {
    // Find all the fields we are allowed to filter by.
    $fields = views_fetch_fields($this->base_table, 'filter');

    $entity_info = $this->entity_info;
    // If the current base table support bundles and has more than one (like user).
    if (isset($entity_info['bundle keys']) && isset($entity_info['bundles'])) {
      // Get all bundles and their human readable names.
      $options = array('all' => t('All'));
      foreach ($entity_info['bundles'] as $type => $bundle) {
        $options[$type] = $bundle['label'];
      }
      $form['displays']['show']['type'] = array(
        '#type' => 'select',
        '#title' => t('of type'),
        '#options' => $options,
        '#default_value' => 'all',
        '#ajax' => array(
          'callback' => 'views_ui_add_form_update',
          'wrapper' => 'edit-view-displays-wrapper',
        ),
      );
      // Add this to the "Update options" button's form validation so the
      // submitted type will always be available in $form_state['values'].
      // This allows us to use $form_state['values']['show']['type'] to
      // dynamically build the rest of the form.
      $form['displays']['show']['update_wizard_key']['#limit_validation_errors'][] = array('show', 'type');
    }

    // Check if we are allowed to filter by taxonomy, and if so, add the
    // "tagged with" filter to the view.
    //
    // We construct this filter using taxonomy_index.tid (which limits the
    // filtering to a specific vocabulary) rather than taxonomy_term_data.name
    // (which matches terms in any vocabulary). This is because it is a more
    // commonly-used filter that works better with the autocomplete UI, and
    // also to avoid confusion with other vocabularies on the site that may
    // have terms with the same name but are not used for free tagging.
    //
    // The downside is that if there *is* more than one vocabulary on the site
    // that is used for free tagging, the wizard will only be able to make the
    // "tagged with" filter apply to one of them (see below for the method it
    // uses to choose).
    if (isset($fields['taxonomy_index.tid'])) {
      // Check if this view will be displaying fieldable entities.
      if (!empty($entity_info['fieldable'])) {
        // Find all "tag-like" taxonomy fields associated with the view's
        // entities. If a particular entity type (i.e., bundle) has been
        // selected above, then we only search for taxonomy fields associated
        // with that bundle. Otherwise, we use all bundles.
        $bundles = array_keys($entity_info['bundles']);
        $selected_bundle = NULL;
        if (isset($form_state['values']['show']['type'])) {
          $selected_bundle = $form_state['values']['show']['type'];
        }
        elseif (isset($form['displays']['show']['type']['#default_value'])) {
          $selected_bundle = $form['displays']['show']['type']['#default_value'];
        }
        if (isset($selected_bundle) && in_array($selected_bundle, $bundles)) {
          $bundles = array($selected_bundle);
        }
        $tag_fields = array();
        foreach ($bundles as $bundle) {
          foreach (field_info_instances($this->entity_type, $bundle) as $instance) {
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
          // Add the autocomplete textfield to the wizard.
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

  /**
   * Build the part of the form that allows the user to select the view's sort order.
   *
   * By default, this adds a "sorted by [date]" filter (when it is available).
   */
  protected function build_sorts(&$form, &$form_state) {
    // Check if we are allowed to sort by creation date.
    $sorts = array();
    if (!empty($this->plugin['created_column'])) {
      $sorts = array(
        $this->plugin['created_column'] . ':DESC' => t('Newest first'),
        $this->plugin['created_column'] . ':ASC' => t('Oldest first'),
      );
      if (isset($this->plugin['available_sorts'])) {
        $sorts += $this->plugin['available_sorts'];
      }
    }
    if (!empty($sorts)) {
      $form['displays']['show']['sort'] = array(
        '#type' => 'select',
        '#title' => t('sorted by'),
        '#options' => $sorts,
        '#default_value' => isset($this->plugin['created_column']) ? $this->plugin['created_column'] . ':DESC' : NULL,
      );
    }
  }

  protected function instantiate_view($form, &$form_state) {
    $view = views_new_view();
    $view->name = $form_state['values']['name'];
    $view->human_name = $form_state['values']['human_name'];
    $view->description = $form_state['values']['description'];
    $view->tag = 'default';
    $view->core = VERSION;
    $view->base_table = $this->base_table;

    // Display: Master
    $default_display = $view->new_display('default', 'Master', 'default');
    $options = $this->default_display_options($form, $form_state);
    if (!isset($options['filters'])) {
      $options['filters'] = array();
    }
    $options['filters'] += $this->default_display_filters($form, $form_state);
    if (!isset($options['sorts'])) {
      $options['sorts'] = array();
    }
    $options['sorts'] += $this->default_display_sorts($form, $form_state);
    foreach ($options as $option => $value) {
      $default_display->set_option($option, $value);
    }

    // Display: Page
    if (!empty($form_state['values']['page']['create'])) {
      $display = $view->new_display('page', 'Page', 'page');
      $options = $this->page_display_options($form, $form_state);
      // The page display is usually the main one (from the user's point of
      // view). Its options should therefore become the overall view defaults,
      // so that new displays which are added later automatically inherit them.
      $this->set_default_options($options, $display, $default_display);
      // Display: Feed (attached to the page)
      if (!empty($form_state['values']['page']['feed'])) {
        $display = $view->new_display('feed', 'Feed', 'feed');
        $options = $this->page_feed_display_options($form, $form_state);
        $this->set_override_options($options, $display, $default_display);
      }
    }

    // Display: Block
    if (!empty($form_state['values']['block']['create'])) {
      $display = $view->new_display('block', 'Block', 'block');
      $options = $this->block_display_options($form, $form_state);
      // When there is no page, the block display options should become the
      // overall view defaults.
      if (empty($form_state['values']['page']['create'])) {
        $this->set_default_options($options, $display, $default_display);
      }
      else {
        $this->set_override_options($options, $display, $default_display);
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

    // Add any filters provided by the plugin.
    if (isset($this->plugin['filters'])) {
      foreach ($this->plugin['filters'] as $name => $info) {
        $filters[$name] = $info;
      }
    }

    // Add any filters specified by the user when filling out the wizard.
    $filters = array_merge($filters, $this->default_display_filters_user($form, $form_state));

    return $filters;
  }

  protected function default_display_filters_user($form, $form_state) {
    $filters = array();

    if (!empty($form_state['values']['show']['type']) && $form_state['values']['show']['type'] != 'all') {
      $bundle_key = $this->entity_info['bundle keys']['bundle'];
      $filters[$bundle_key] = array(
        'id' => $bundle_key,
        'table' => $this->base_table,
        'field' => $bundle_key,
        'value' => drupal_map_assoc(array($form_state['values']['show']['type'])),
      );
    }
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
        // Sort the terms so the filter will be displayed as it normally would
        // on the edit screen.
        sort($filters['tid']['value']);
      }
    }

    return $filters;
  }

  protected function default_display_sorts($form, $form_state) {
    $sorts = array();

    // Add any sorts provided by the plugin.
    if (isset($this->plugin['sorts'])) {
      foreach ($this->plugin['sorts'] as $name => $info) {
        $sorts[$name] = $info;
      }
    }

    // Add any sorts specified by the user when filling out the wizard.
    $sorts = array_merge($sorts, $this->default_display_sorts_user($form, $form_state));

    return $sorts;
  }

  protected function default_display_sorts_user($form, $form_state) {
    $sorts = array();

    if (!empty($form_state['values']['show']['sort'])) {
      list($column, $sort) = explode(':', $form_state['values']['show']['sort']);
      // Column either be a column-name or the table-columnn-ame.
      $column = explode('-', $column);
      if (count($column) > 1) {
        $table = $column[0];
        $column = $column[1];
      }
      else {
        $table = $this->base_table;
        $column = $column[0];
      }

      $sorts[$column] = array(
        'id' => $column,
        'table' => $table,
        'field' => $column,
        'order' => $sort,
      );
    }

    return $sorts;
  }

  protected function page_display_options($form, $form_state) {
    $display_options = array();
    $page = $form_state['values']['page'];
    $display_options['title'] = $page['title'];
    $display_options['path'] = $page['path'];
    $display_options['style_plugin'] = $page['style']['style_plugin'];
    $display_options['pager']['type'] = 'full';
    $display_options['pager']['options']['items_per_page'] = $page['items_per_page'];
    if (!empty($page['link'])) {
      $display_options['menu']['type'] = 'normal';
      $display_options['menu']['title'] = $page['link_properties']['title'];
      $display_options['menu']['name'] = $page['link_properties']['menu_name'];
    }
    return $display_options;
  }

  protected function block_display_options($form, $form_state) {
    $display_options = array();
    $block = $form_state['values']['block'];
    $display_options['title'] = $block['title'];
    $display_options['style_plugin'] = $block['style']['style_plugin'];
    $display_options['pager']['type'] = 'full';
    $display_options['pager']['options']['items_per_page'] = $block['items_per_page'];
    return $display_options;
  }

  protected function page_feed_display_options($form, $form_state) {
    $display_options = array();
    $display_options['pager']['type'] = 'some';
    $display_options['style_plugin'] = 'rss';
    $display_options['row_plugin'] = $form_state['values']['page']['feed_properties']['row_plugin'];
    $display_options['path'] = $form_state['values']['page']['feed_properties']['path'];
    $display_options['title'] = $form_state['values']['page']['title'];
    $display_options['displays'] = array(
      'default' => 'default',
      'page' => 'page',
    );
    return $display_options;
  }

  /**
   * Sets options for a display and makes them the default options if possible.
   *
   * This function can be used to set options for a display when it is desired
   * that the options also become the defaults for the view whenever possible.
   * This should be done for the "primary" display created in the view wizard,
   * so that new displays which the user adds later will be similar to this
   * one.
   *
   * @param $options
   *   An array whose keys are the name of each option and whose values are the
   *   desired values to set.
   * @param $display
   *   The display which the options will be applied to. The default display
   *   will actually be assigned the options (and this display will inherit
   *   them) when possible.
   * @param $default_display
   *   The default display, which will store the options when possible.
   */
  protected function set_default_options($options, $display, $default_display) {
    foreach ($options as $option => $value) {
      // If the default display supports this option, set the value there.
      // Otherwise, set it on the provided display.
      $default_value = $default_display->get_option($option);
      if (isset($default_value)) {
        $default_display->set_option($option, $value);
      }
      else {
        $display->set_option($option, $value);
      }
    }
  }

  /**
   * Sets options for a display, inheriting from the defaults when possible.
   *
   * This function can be used to set options for a display when it is desired
   * that the options inherit from the default display whenever possible. This
   * avoids setting too many options as overrides, which will be harder for the
   * user to modify later. For example, if $this->set_default_options() was
   * previously called on a page display and then this function is called on a
   * block display, and if the user entered the same title for both displays in
   * the views wizard, then the view will wind up with the title stored as the
   * default (with the page and block both inheriting from it).
   *
   * @param $options
   *   An array whose keys are the name of each option and whose values are the
   *   desired values.
   * @param $display
   *   The display which the options will apply to. It will get the options by
   *   inheritance from the default display when possible.
   * @param $default_display
   *   The default display, from which the options will be inherited when
   *   possible.
   */
  protected function set_override_options($options, $display, $default_display) {
    foreach ($options as $option => $value) {
      // Only override the default value if it is different from the value that
      // was provided.
      $default_value = $default_display->get_option($option);
      if (!isset($default_value)) {
        $display->set_option($option, $value);
      }
      elseif ($default_value !== $value) {
        $display->override_option($option, $value);
      }
    }
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
