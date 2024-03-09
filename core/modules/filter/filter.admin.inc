<?php
/**
 * @file
 * Admin page callbacks for the Filter module.
 */

/**
 * Page callback: Form constructor for a form to list and reorder text formats.
 *
 * @ingroup forms
 * @see filter_menu()
 * @see filter_admin_overview_submit()
 */
function filter_admin_overview($form) {
  // Overview of all formats.
  $formats = filter_formats(NULL, TRUE);
  $fallback_format = filter_fallback_format();
  $editors = filter_get_editors();

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('The order of the text formats below determines the order in which they will appear in the <em>Editor</em> selector when creating content (depending on permissions for the text formats).'),
  );

  $form['#tree'] = TRUE;
  foreach ($formats as $id => $format) {
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => "admin/config/content/formats/$id",
      'weight' => 0,
    );
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => 'Text formats',
          'name' => 'filter.format.' . $id,
        ),
        'weight' => 5,
      );
    }

    // Add status so we can group by enabled/disabled. If disabled, add 100
    // to weight to push it down to the bottom of the list.
    $form['formats'][$id]['#status'] = $format->status;
    $form['formats'][$id]['#weight'] = $format->weight;
    if (!$format->status) {
      $form['formats'][$id]['#weight'] += 100;
    }

    // Check whether this is the fallback text format. This format is available
    // to all roles and cannot be disabled via the admin interface.
    $form['formats'][$id]['#is_fallback'] = ($id == $fallback_format);
    if ($form['formats'][$id]['#is_fallback']) {
      $label = theme('label_machine_name__filter', array(
        'label' => $format->name,
        'machine_name' => $id,
      ));
      $form['formats'][$id]['name'] = array('#markup' => $label);
      $roles_markup = t('Fallback format. Used if a role has access to no other formats.');
    }
    else {
      if ($form['formats'][$id]['#status']) {
        $links['disable'] = array(
          'title' => t('Disable'),
          'href' => 'admin/config/content/formats/' . $id . '/disable',
          'query' => array(
            'token' => backdrop_get_token('format-' . $format->format)
          ),
          'weight' => 10,
        );
      }
      else {
        $links['enable'] = array(
          'title' => t('Enable'),
          'href' => 'admin/config/content/formats/' . $id . '/enable',
          'query' => array(
            'token' => backdrop_get_token('format-' . $format->format)
          ),
          'weight' => -10,
        );
      }
      $label = theme('label_machine_name__filter', array(
        'label' => $format->name,
        'machine_name' => $id,
      ));
      $form['formats'][$id]['name'] = array('#markup' => $label);
      $roles = array();
      foreach (filter_get_roles_by_format($format) as $role_name) {
        $role = user_role_load($role_name);
        $roles[] = check_plain($role->label);
      }
      $roles_markup = $roles ? implode(', ', $roles) : t('No roles may use this format');
    }
    $form['formats'][$id]['editor'] = array(
      '#markup' => isset($editors[$format->editor]) ? check_plain($editors[$format->editor]['label']) : t('None'),
    );
    $form['formats'][$id]['roles'] = array(
      '#markup' => $roles_markup,
    );
    backdrop_sort($links, array('weight'));
    $form['formats'][$id]['operations'] = array(
      '#type' => 'operations',
      '#links' => $links,
    );
    $form['formats'][$id]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $format->name)),
      '#title_display' => 'invisible',
      '#default_value' => $format->weight,
    );
  }
  backdrop_sort($form['formats'], array('#weight'));

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save changes'));
  return $form;
}

/**
 * Form submission handler for filter_admin_overview().
 */
function filter_admin_overview_submit($form, &$form_state) {
  $formats = filter_formats(NULL, TRUE);
  foreach ($form_state['values']['formats'] as $id => $data) {
    if (is_array($data) && isset($data['weight']) && $data['weight'] != $formats[$id]->weight) {
      // Update the weight of this filter.
      $formats[$id]->weight = $data['weight'];
      filter_format_save($formats[$id]);
    }
  }
  filter_formats_reset();
  backdrop_set_message(t('The text format order has been saved.'));
}

/**
 * Page callback: Displays the text format add/edit form.
 *
 * @param object|null $format
 *   (optional) An object representing a format, with the following properties:
 *   - format: A machine-readable name representing the ID of the text format
 *     to save. If this corresponds to an existing text format, that format
 *     will be updated; otherwise, a new format will be created.
 *   - name: The title of the text format.
 *   - cache: (optional) An integer indicating whether the text format is
 *     cacheable (1) or not (0). Defaults to 1.
 *   - status: (optional) An integer indicating whether the text format is
 *     enabled (1) or not (0). Defaults to 1.
 *   - weight: (optional) The weight of the text format, which controls its
 *     placement in text format lists. If omitted, the weight is set to 0.
 *     Defaults to NULL.
 *
 * @return
 *   A form array.
 *
 * @see filter_menu()
 */
function filter_admin_format_page($format = NULL) {
  if (!isset($format->name)) {
    backdrop_set_title(t('Add text format'));
    $format = (object) array(
      'format' => NULL,
      'name' => '',
      'editor' => '',
      'editor_settings' => array(),
      'cache' => 1,
      'status' => 1,
      'is_new' => TRUE,
      'filters' => array(),
    );
  }
  return backdrop_get_form('filter_admin_format_form', $format);
}

/**
 * Form constructor for the text format add/edit form.
 *
 * @param $format
 *   A format object having the properties:
 *   - format: A machine-readable name representing the ID of the text format to
 *     save. If this corresponds to an existing text format, that format will be
 *     updated; otherwise, a new format will be created.
 *   - name: The title of the text format.
 *   - cache: An integer indicating whether the text format is cacheable (1) or
 *     not (0). Defaults to 1.
 *   - status: (optional) An integer indicating whether the text format is
 *     enabled (1) or not (0). Defaults to 1.
 *   - weight: (optional) The weight of the text format, which controls its
 *     placement in text format lists. If omitted, the weight is set to 0.
 *
 * @see filter_admin_format_form_validate()
 * @see filter_admin_format_form_submit()
 * @ingroup forms
 */
function filter_admin_format_form($form, &$form_state, $format) {
  // Use a format stored in tempstore if available.
  if ($stored_format = filter_get_format_tempstore($format->format)) {
    filter_admin_set_message($stored_format);
    $format = $stored_format;
  }
  $is_fallback = ($format->format == filter_fallback_format());
  $editors = filter_get_editors();
  if (isset($form_state['editor_info'])) {
    $editor_info = $form_state['editor_info'];

    // If the editor changed, update the format settings accordingly.
    $current_editor = $form_state['values']['editor'];
    if ($format->editor != $current_editor && isset($editors[$current_editor])) {
      $format->editor = $current_editor;
      $format->editor_settings = array();
    }
  }
  else {
    $editor_info = ($format->editor && isset($editors[$format->editor])) ? $editors[$format->editor] : NULL;
  }

  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => '',
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="filter-messages">',
    '#suffix' => '</div>',
  );

  $form_state['format'] = $format;
  $form_state['editor_info'] = $editor_info;

  $form['#validate'] = array();
  $form['#submit'] = array();
  $form['#tree'] = TRUE;
  $form['#attached']['library'][] = array('filter', 'filter.admin');
  $form['#attached']['library'][] = array('filter', 'filter.filtered_html.admin');
  $form['#attached']['library'][] = array('system', 'backdrop.dialog');

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $format->name,
    '#required' => TRUE,
    '#weight' => -20,
  );
  $form['format'] = array(
    '#type' => 'machine_name',
    '#required' => TRUE,
    '#default_value' => $format->format,
    '#maxlength' => 255,
    '#disabled' => !empty($format->format),
    '#machine_name' => array(
      'exists' => 'filter_format_exists',
    ),
    '#weight' => -19,
  );

  // Build the list of all available editors.
  $editor_options = array('' => t('None'));
  foreach ($editors as $editor_name => $editor) {
    $editor_options[$editor_name] = $editor['label'];
  }

  // Associate an editor with this format.
  if ($editor_info) {
    // Load the associated editor callbacks file, if any.
    if (!empty($editor_info['file'])) {
      $filepath = $editor_info['file'];
      $extension = substr($filepath, strrpos($filepath, '.') + 1);
      $filepath = substr($filepath, 0, strrpos($filepath, '.'));
      form_load_include($form_state, $extension, $editor_info['module'], $filepath);
    }
  }

  $form['editor'] = array(
    '#weight' => -9,
  );
  $form['editor']['editor'] = array(
    '#type' => 'select',
    '#title' => t('Text editor'),
    '#options' => $editor_options,
    '#empty_option' => t('None'),
    '#default_value' => $format->editor ? $format->editor : '',
    '#ajax' => array(
      'trigger_as' => array('name' => 'editor_configure'),
      'callback' => 'filter_admin_format_editor_ajax',
      'wrapper' => 'editor-settings-wrapper',
    ),
    '#parents' => array('editor'),
  );
  $form['editor']['configure'] = array(
    '#type' => 'submit',
    '#name' => 'editor_configure',
    '#value' => t('Configure editor'),
    '#limit_validation_errors' => array(array('editor')),
    '#submit' => array('filter_admin_format_editor_submit'),
    '#ajax' => array(
      'callback' => 'filter_admin_format_editor_ajax',
      'wrapper' => 'editor-settings-wrapper',
    ),
    '#attributes' => array('class' => array('js-hide')),
    '#parents' => array('editor_configure'),
  );

  // If there aren't any options (other than "None"), disable the select list.
  if (empty($editor_options)) {
    $form['editor']['editor']['#disabled'] = TRUE;
    $form['editor']['editor']['#description'] = t('This option is disabled because no modules that provide a text editor are currently enabled.');
  }

  $form['editor_settings'] = array(
    '#tree' => TRUE,
    '#weight' => -8,
    '#type' => 'container',
    '#id' => 'editor-settings-wrapper',
  );

  // Populate editor defaults.
  if (!empty($editor_info['default settings'])) {
    $format->editor_settings += $editor_info['default settings'];
  }

  // Add editor-specific validation and submit handlers.
  if (!empty($editor_info['settings callback'])) {
    $function = $editor_info['settings callback'];
    $form['editor_settings'] = array_merge($function($form, $form_state, $format), $form['editor_settings']);
    $form['editor_settings']['#parents'] = array('editor_settings');
  }

  // Add user role access selection.
  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles'),
    '#options' => array_map('check_plain', user_roles()),
    '#disabled' => $is_fallback,
  );
  if ($is_fallback) {
    $form['roles']['#description'] = t('All roles for this text format must be enabled and cannot be changed.');
  }
  if (!empty($format->format)) {
    // If editing an existing text format, pre-select its current permissions.
    $form['roles']['#default_value'] = filter_get_roles_by_format($format);
  }
  elseif ($admin_role = config_get('system.core', 'user_admin_role')) {
    // If adding a new text format and the site has an administrative role,
    // pre-select that role so as to grant administrators access to the new
    // text format permission by default.
    $form['roles']['#default_value'] = array($admin_role);
  }

  // Retrieve available filters and load all configured filters for existing
  // text formats.
  $all_filter_info = filter_get_filters();

  // Create an empty filter object for new/unconfigured filters.
  foreach ($all_filter_info as $name => $filter_info) {
    if (!isset($format->filters[$name])) {
      $format->filters[$name] = (object) array(
        'format' => $format->format,
        'module' => $filter_info['module'],
        'name' => $name,
        'status' => 0,
        'weight' => $filter_info['weight'],
        'settings' => $filter_info['default settings'],
      );
    }
  }

  // Filter order (tabledrag).
  $form['filters'] = array(
    '#type' => 'item',
    '#title' => t('Filter settings'),
    '#theme' => 'filter_admin_format_filter_order',
  );
  foreach ($all_filter_info as $name => $filter_info) {
    $filter_config = $format->filters[$name];
    $form['filters'][$name]['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => $filter_config->status,
      '#attributes' => array('class' => array('filter-status')),
    );
    if (isset($filter_info['settings callback'])) {
      $form['filters'][$name]['configure_button'] = array(
        '#type' => 'submit',
        '#name' => $name,
        '#validate' => array('filter_admin_format_filter_settings_form_redirect_validate'),
        '#submit' => array('filter_admin_format_filter_settings_form_redirect'),
        '#ajax' => array(
          'callback' => 'filter_admin_format_filter_settings_form_ajax',
          'effect' => 'fade',
        ),
        '#value' => t('Configure'),
      );
    }
    $form['filters'][$name]['filter'] = array(
      '#type' => 'item',
      '#title' => $filter_info['title'],
      '#description' => _filter_admin_format_get_tips($format, $name),
      '#wrapper_attributes' => array(
        'class' => array('filter-info'),
        'data-filter-name' => $name,
      ),
    );

    $form['filters'][$name]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $filter_info['title'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => $filter_config->weight,
      '#parents' => array('filters', $name, 'weight'),
    );
    $form['filters'][$name]['#weight'] = $filter_config->weight;
  }

  // This hidden field is used to store allowed HTML tags for the filter_html
  // filter. This is necessary to allow JS to respond to changes in editor
  // buttons, changing allowed tags in response.
  $filter_html = $format->filters['filter_html'];
  $form['allowed_html'] = array(
    '#type' => 'hidden',
    '#maxlength' => 1024,
    '#attributes' => array('id' => array('allowed-html')),
    '#default_value' => $filter_html->settings['allowed_html'],
  );

  $form['#validate'][] = 'filter_admin_format_form_validate';
  $form['#submit'][] = 'filter_admin_format_form_submit';

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#submit' => array('filter_admin_format_form_cancel'),
  );

  return $form;
}

/**
 * Submit handler for filter_admin_format_form() that cancels in-progress work.
 */
function filter_admin_format_form_cancel($form, &$form_state) {
  $format = $form_state['format'];

  if (isset($format->format)) {
    filter_clear_format_tempstore($format->format);
  }
  $form_state['redirect'] = 'admin/config/content/formats';
}

/**
 * Validation for filter "Configure" buttons in filter_admin_format_form().
 */
function filter_admin_format_filter_settings_form_redirect_validate($form, &$form_state) {
  // Throw a form error if there is no machine name yet, as we need this to set
  // up the URLs for settings forms.
  if (empty($form_state['values']['format'])) {
    form_set_error('format', t('Please enter a format name before configuring filter settings.'));
  }
}

/**
 * Submit handler for filter "Configure" buttons in filter_admin_format_form().
 */
function filter_admin_format_filter_settings_form_redirect($form, &$form_state) {
  $trigger = $form_state['triggering_element'];
  $name = $trigger['#name'];

  // Save any changes to the allowed HTML values made by the front-end.
  if (isset($form_state['format']->filters['filter_html'])) {
    $form_state['format']->filters['filter_html']->settings['allowed_html'] = $form_state['values']['allowed_html'];
  }

  // Populate a format name if creating a new format.
  if (empty($form_state['format']->format)) {
    $form_state['format']->format = $form_state['values']['format'];
  }
  $form_state['redirect'] = 'admin/config/content/formats/' . $form_state['format']->format . '/filter-settings/' . $name;
}

/**
 * Ajax handler: Opens the dialog to configure a text format filter.
 */
function filter_admin_format_filter_settings_form_ajax($form, &$form_state) {
  // If there are form errors, show them and scroll to the top of the page.
  if (form_get_errors()) {
    $commands[] = ajax_command_html('#filter-messages', theme('status_messages'));
    $commands[] = ajax_command_redirect('#filter-messages');
  }
  // If no form errors, open the configure dialog.
  else {
    $trigger = $form_state['triggering_element'];
    $name = $trigger['#name'];
    // Load previously added settings from tempstore, if any.
    if ($stored_format = filter_get_format_tempstore($form_state['format']->format)) {
      $format = $stored_format;
    }
    else {
      $format = $form_state['format'];
    }

    $configure_form = backdrop_get_form('filter_admin_format_filter_settings_form', $format, $name);
    $title = t('Configure filter');
    $options = array(
      'dialogClass' => 'filter-dialog',
      'width' => '90%',
    );

    $html = '';
    $html .= theme('status_messages');
    $html .= '<div data-filter-name="' . $name . '" class="filter-dialog-settings">';
    $html .= backdrop_render($configure_form);
    $html .= '</div>';

    $commands = array();
    $commands[] = ajax_command_html('#filter-messages', '');
    $commands[] = ajax_command_open_modal_dialog($title, $html, $options);
  }

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Returns the configuration form for an individual configurable filter.
 */
function filter_admin_format_filter_settings_form($form, &$form_state, $format, $filter_name) {
  form_load_include($form_state, 'inc', 'filter', 'filter.admin');
  $form_state['format'] = $format;
  $form_state['filter_name'] = $filter_name;

  // Use a format stored in tempstore if available.
  // Has to get loaded again here in case we did not come here via ajax.
  if ($stored_format = filter_get_format_tempstore($format->format)) {
    $form_state['format'] = $stored_format;
    $format = $stored_format;
  }
  $all_filter_info = filter_get_filters();
  $filter_info = $all_filter_info[$filter_name];
  $filter = $format->filters[$filter_name];

  if (isset($filter_info['settings callback'])) {
    $function = $filter_info['settings callback'];
    $settings_form = $function($form, $form_state, $filter, $format);
  }

  $settings_form['filter'] = array(
    '#type' => 'value',
    '#value' => $filter->name,
  );
  $settings_form['actions']['#type'] = 'actions';
  $settings_form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array(),
    '#submit' => array('filter_admin_format_filter_settings_update'),
    '#ajax' => array(
      'callback' => 'filter_admin_format_filter_settings_ajax',
      'progress' => array(
        'type' => 'throbber',
        'message' => NULL,
      ),
    ),
  );

  $settings_form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => '',
    '#attributes' => array(
      // Adding button classes puts the link in the button area of the dialog,
      // while the special "dialog-cancel" class closes the dialog.
      'class' => array('button', 'button-secondary', 'form-submit', 'dialog-cancel'),
    ),
  );

  return $settings_form;
}

/**
 * Form submit handler for filter_admin_format_filter_settings_form().
 */
function filter_admin_format_filter_settings_update($form, &$form_state) {
  form_state_values_clean($form_state);
  unset($form_state['values']['filter']);
  $filter_name = $form_state['filter_name'];
  $format = $form_state['format'];

  $format->filters[$filter_name]->settings = $form_state['values'];
  $format->updated = TRUE;
  filter_set_format_tempstore($format);
  filter_admin_set_message($format);

  $form_state['redirect'] = 'admin/config/content/formats/' . $format->format;
}

/**
 * Ajax handler for buttons in filter_admin_format_filter_settings_form().
 */
function filter_admin_format_filter_settings_ajax($form, &$form_state) {
  $filter_name = $form_state['filter_name'];
  $format = $form_state['format'];

  $commands = array();

  // Update the filter tips for the configured filter.
  $cloned_format = clone($format);
  $cloned_format->editor = FALSE;
  $tips = _filter_admin_format_get_tips($format, $filter_name);
  if (isset($tips)) {
    $commands[] = ajax_command_html('.filter-info[data-filter-name="' . $filter_name . '"] .description', $tips);
  }

  if (isset($form_state['values']['allowed_html'])) {
    // Update the tag list stored in the main form hidden element.
    $commands[] = ajax_command_invoke('#allowed-html', 'val', array($form_state['values']['allowed_html']));
    $commands[] = ajax_command_invoke('#allowed-html', 'trigger', array('change.filter-update'));
  }
  $commands[] = ajax_command_html('#filter-messages', theme('status_messages'));
  $commands[] = ajax_command_close_modal_dialog();
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Form validation handler for filter_admin_format_form().
 *
 * @see filter_admin_format_form_submit()
 */
function filter_admin_format_form_validate($form, &$form_state) {
  $format_format = trim($form_state['values']['format']);
  $format_name = trim($form_state['values']['name']);

  // Ensure that the values to be saved later are exactly the ones validated.
  form_set_value($form['format'], $format_format, $form_state);
  form_set_value($form['name'], $format_name, $form_state);

  $formats = filter_formats();
  foreach ($formats as $format) {
    if ($format->format !== $format_format && $format->name === $format_name) {
      form_set_error('name', t('Text format names must be unique. A format named %name already exists.', array('%name' => $format_name)));
    }
  }
}

/**
 * AJAX callback for filter_admin_format_form().
 */
function filter_admin_format_editor_ajax($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_replace('#editor-settings-wrapper', backdrop_render($form['editor_settings']));
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Form submission handler for filter_admin_format_form().
 *
 * Sets the editor_info property so that the form can be AJAX-updated.
 */
function filter_admin_format_editor_submit($form, &$form_state) {
  $editor_name = $form_state['values']['editor'];
  $editors = filter_get_editors();
  if ($editor_name && isset($editors[$editor_name])) {
    $form_state['editor_info'] = $editors[$editor_name];
  }
  else {
    $form_state['editor_info'] = array();
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form submission handler for filter_admin_format_form().
 *
 * @see filter_admin_format_form_validate()
 */
function filter_admin_format_form_submit($form, &$form_state) {
  // Remove unnecessary values.
  form_state_values_clean($form_state);
  unset($form_state['values']['filter_settings']);
  unset($form_state['values']['actions']);

  // Add the submitted form values to the text format, and save it.
  $format = $form_state['format'];
  // Save allowed HTML tags from the hidden field in the event JS modified them.
  $allowed_html = $form_state['values']['allowed_html'];
  unset($form_state['values']['allowed_html']);

  foreach ($form_state['values'] as $key => $value) {
    $format->$key = $value;
  }
  // Get values from filter settings forms back from tempstore.
  if ($stored_format = filter_get_format_tempstore($format->format)) {
    foreach ($stored_format->filters as $name => $filter) {
      $format->filters[$name]['settings'] = $filter->settings;
    }
  }
  // Set allowed html, parked in a variable previously.
  $format->filters['filter_html']['settings']['allowed_html'] = $allowed_html;

  // If not saving an editor, do not save any settings.
  if (!$format->editor) {
    $format->editor_settings = array();
  }

  $status = filter_format_save($format);

  // Save user permissions.
  if ($permission = filter_permission_name($format)) {
    foreach ($format->roles as $rid => $enabled) {
      user_role_change_permissions($rid, array($permission => $enabled));
    }
  }

  switch ($status) {
    case SAVED_NEW:
      backdrop_set_message(t('Added text format %format.', array('%format' => $format->name)));
      break;

    case SAVED_UPDATED:
      backdrop_set_message(t('Updated text format %format.', array('%format' => $format->name)));
      break;
  }

  filter_clear_format_tempstore($format->format);
  $form_state['redirect'] = 'admin/config/content/formats';
}

/**
 * Page callback: Enable a text format.
 */
function filter_admin_enable($format = NULL) {
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], 'format-' . $format->format)) {
    return MENU_ACCESS_DENIED;
  }
  if (isset($format)) {
    filter_format_enable($format);
    backdrop_set_message(t('Enabled text format %format.', array('%format' => $format->name)));
  }
  else {
    backdrop_set_message(t('No text format provided.'), 'error');
  }
  backdrop_goto('admin/config/content/formats');
}

/**
 * Form constructor for the text format deletion confirmation form.
 *
 * @param $format
 *   An object representing a text format.
 *
 * @see filter_menu()
 * @see filter_admin_disable_submit()
 * @ingroup forms
 */
function filter_admin_disable($form, &$form_state, $format) {
  $form_state['format'] = $format;

  return confirm_form($form,
    t('Are you sure you want to disable the text format %format?', array('%format' => $format->name)),
    'admin/config/content/formats',
    t('Any content stored with that format may not be rendered properly.'),
    t('Disable')
  );
}

/**
 * Form submission handler for filter_admin_disable().
 */
function filter_admin_disable_submit($form, &$form_state) {
  $format = $form_state['format'];

  filter_format_disable($format);
  backdrop_set_message(t('Disabled text format %format.', array('%format' => $format->name)));
  $form_state['redirect'] = 'admin/config/content/formats';
}

/**
 * Subform constructor to configure the text editor's image upload settings.
 *
 * Each text editor plugin that is configured to offer the ability to insert
 * images and uses EditorImageDialog for that, should use this form to update
 * the text editor's configuration so that EditorImageDialog knows whether it
 * should allow the user to upload images.
 *
 * @param $format
 *   The text format that is being edited.
 *
 * @return array
 *   The image upload settings form.
 *
 * @ingroup forms
 */
function filter_editor_image_upload_settings_form($format) {
  // Defaults.
  $settings = isset($format->editor_settings['image_upload']) ? $format->editor_settings['image_upload'] : array();
  $settings += array(
    'status' => FALSE,
    'scheme' => file_default_scheme(),
    'alt_required' => FALSE,
    'directory' => 'inline-images',
    'max_size' => '',
    'max_dimensions' => array('width' => '', 'height' => ''),
    'orientate' => FALSE,
  );
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable image uploads'),
    '#default_value' => $settings['status'],
    '#attributes' => array(
      'data-editor-image-upload' => 'status',
    ),
  );
  $show_if_image_uploads_enabled = array(
    'visible' => array(
      ':input[data-editor-image-upload="status"]' => array('checked' => TRUE),
    ),
  );

  // Any visible, writable wrapper can potentially be used for uploads,
  // including a remote file system that integrates with a CDN.
  $stream_wrappers = file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE);
  foreach ($stream_wrappers as $scheme => $info) {
    $options[$scheme] = $info['description'];
  }

  $form['orientate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable image re-orientation'),
    '#description' => t('If EXIF data indicates the need, the image will be rotated appropriately before being saved.'),
    '#default_value' => $settings['orientate'],
    '#attributes' => array(
      'data-editor-image-rotate' => 'orientate',
    ),
    '#states' => $show_if_image_uploads_enabled,
    '#access' => function_exists('exif_read_data'),
  );

  if (!empty($options)) {
    $form['scheme'] = array(
      '#type' => 'radios',
      '#title' => t('File storage'),
      '#default_value' => $settings['scheme'],
      '#options' => $options,
      '#states' => $show_if_image_uploads_enabled,
      '#access' => count($options) > 1,
    );
  }
  // Set data- attributes with human-readable names for all possible stream
  // wrappers, so that backdrop.ckeditor.backdropimage.admin's summary rendering
  // can use that.
  foreach ($stream_wrappers as $scheme => $info) {
    $form['scheme'][$scheme]['#attributes']['data-label'] = t('Storage: @name', array('@name' => $info['name']));
  }

  $form['alt_required'] = array(
    '#type' => 'checkbox',
    '#title' => t('<em>Alt</em> text required'),
    '#description' => t('Requiring alternative text is strongly encouraged to improve accessibility.'),
    '#default_value' => $settings['alt_required'],
    '#states' => $show_if_image_uploads_enabled,
  );

  $form['directory'] = array(
    '#type' => 'textfield',
    '#default_value' => $settings['directory'],
    '#title' => t('Upload directory'),
    '#description' => t("A directory relative to the files directory where uploaded images will be stored."),
    '#states' => $show_if_image_uploads_enabled,
  );

  $default_max_size = format_size(file_upload_max_size());
  $form['max_size'] = array(
    '#type' => 'textfield',
    '#default_value' => $settings['max_size'],
    '#title' => t('Maximum file size'),
    '#description' => t('If this is left empty, then the file size will be limited by the PHP maximum upload size of @size.', array('@size' => $default_max_size)),
    '#maxlength' => 20,
    '#size' => 10,
    '#placeholder' => $default_max_size,
    '#states' => $show_if_image_uploads_enabled,
  );

  $form['max_dimensions'] = array(
    '#type' => 'item',
    '#title' => t('Maximum dimensions'),
    '#field_prefix' => '<div class="container-inline clearfix">',
    '#field_suffix' => '</div>',
    '#description' => t('Images larger than these dimensions will be scaled down.'),
    '#states' => $show_if_image_uploads_enabled,
  );
  $form['max_dimensions']['width'] = array(
    '#title' => t('Width'),
    '#title_display' => 'invisible',
    '#type' => 'number',
    '#default_value' => $settings['max_dimensions']['width'],
    '#size' => 8,
    '#maxlength' => 8,
    '#min' => 1,
    '#max' => 99999,
    '#placeholder' => 'width',
    '#field_suffix' => ' x ',
    '#states' => $show_if_image_uploads_enabled,
  );
  $form['max_dimensions']['height'] = array(
    '#title' => t('Height'),
    '#title_display' => 'invisible',
    '#type' => 'number',
    '#default_value' => $settings['max_dimensions']['height'],
    '#size' => 8,
    '#maxlength' => 8,
    '#min' => 1,
    '#max' => 99999,
    '#placeholder' => 'height',
    '#field_suffix' => 'pixels',
    '#states' => $show_if_image_uploads_enabled,
  );

  return $form;
}


/**
 * Subform constructor to configure the text editor's file upload settings.
 *
 * Each text editor plugin that is configured to offer the ability to insert
 * images and uses EditorImageDialog for that, should use this form to update
 * the text editor's configuration so that EditorImageDialog can also be used
 * to upload files.
 *
 * @param $format
 *   The text format that is being edited.
 *
 * @return array
 *   The file upload settings form.
 *
 * @ingroup forms
 */
function filter_editor_file_upload_settings_form($format) {
  // Defaults.
  $settings = isset($format->editor_settings['file_upload']) ? $format->editor_settings['file_upload'] : array();
  $settings += array(
    'status' => FALSE,
    'scheme' => file_default_scheme(),
    'directory' => 'inline-files',
    'max_size' => '',
    'file_extensions' => 'txt pdf',
  );

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable file uploads'),
    '#default_value' => $settings['status'],
    '#attributes' => array(
      'data-editor-file-upload' => 'status',
    ),
  );
  $show_if_file_uploads_enabled = array(
    'visible' => array(
      ':input[data-editor-file-upload="status"]' => array('checked' => TRUE),
    ),
  );

  // Any visible, writable wrapper can potentially be used for uploads,
  // including a remote file system that integrates with a CDN.
  $stream_wrappers = file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE);
  foreach ($stream_wrappers as $scheme => $info) {
    $options[$scheme] = $info['description'];
  }
  if (!empty($options)) {
    $form['scheme'] = array(
      '#type' => 'radios',
      '#title' => t('File storage'),
      '#default_value' => $settings['scheme'],
      '#options' => $options,
      '#states' => $show_if_file_uploads_enabled,
      '#access' => count($options) > 1,
    );
  }
  // Set data- attributes with human-readable names for all possible stream
  // wrappers, so that backdrop.ckeditor.backdropimage.admin's summary rendering
  // can use that.
  foreach ($stream_wrappers as $scheme => $info) {
    $form['scheme'][$scheme]['#attributes']['data-label'] = t('Storage: @name', array('@name' => $info['name']));
  }

  $form['directory'] = array(
    '#type' => 'textfield',
    '#default_value' => $settings['directory'],
    '#title' => t('Upload directory'),
    '#description' => t("A directory relative to the files directory where uploaded files will be stored."),
    '#states' => $show_if_file_uploads_enabled,
  );

  $default_max_size = format_size(file_upload_max_size());
  $form['max_size'] = array(
    '#type' => 'textfield',
    '#default_value' => $settings['max_size'],
    '#title' => t('Maximum file size'),
    '#description' => t('If this is left empty, then the file size will be limited by the PHP maximum upload size of @size.', array('@size' => $default_max_size)),
    '#maxlength' => 20,
    '#size' => 10,
    '#placeholder' => $default_max_size,
    '#states' => $show_if_file_uploads_enabled,
  );

  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed file extensions'),
    '#default_value' => $settings['file_extensions'],
    '#description' => t('Separate extensions with a space or comma and do not include the leading dot.'),
    '#element_validate' => array('_file_generic_settings_extensions'),
    '#weight' => 1,
    '#maxlength' => 256,
    // By making this field required, we prevent a potential security issue
    // that would allow files of any type to be uploaded.
    '#required' => TRUE,
    '#states' => $show_if_file_uploads_enabled,
  );

  return $form;
}

/**
 * Sets a page message for changed filter tempstore values.
 */
function filter_admin_set_message($format) {
  if (!empty($format->updated)) {
    $message = t('This form has unsaved changes. Click "Save configuration" to make changes permanent or "Cancel" to discard changes.');
    backdrop_set_message($message, 'warning', FALSE);
  }
}

/**
 * Get filter tips for an individual filter within a text format.
 *
 * This will return tips as though any editor were disabled.
 *
 * @param stdClass $format
 *   The format from which to pull filter tips.
 * @param string $filter_name
 *   The machine name of the filter from which to get tips.
 */
function _filter_admin_format_get_tips($format, $filter_name) {
  // Temporarily disable editor setting.
  $editor_settings = $format->editor;
  $format->editor = FALSE;

  $filter_info = filter_get_filters();

  if (isset($filter_info[$filter_name]['tips callback'])) {
    $tips = $filter_info[$filter_name]['tips callback']($format->filters[$filter_name], $format, FALSE);
  }

  // Restore editor settings.
  $format->editor = $editor_settings;

  return isset($tips) ? $tips : NULL;
}
