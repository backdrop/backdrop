<?php

/**
 * @file
 * Administrative page callbacks for the Filter module.
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
  $formats = filter_formats();
  $fallback_format = filter_fallback_format();

  $form['#tree'] = TRUE;
  foreach ($formats as $id => $format) {
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => "admin/config/content/formats/$id",
    );
    // Check whether this is the fallback text format. This format is available
    // to all roles and cannot be disabled via the admin interface.
    $form['formats'][$id]['#is_fallback'] = ($id == $fallback_format);
    if ($form['formats'][$id]['#is_fallback']) {
      $form['formats'][$id]['name'] = array('#markup' => backdrop_placeholder($format->name));
      $roles_markup = backdrop_placeholder(t('All roles may use this format'));
    }
    else {
      $form['formats'][$id]['name'] = array('#markup' => check_plain($format->name));
      $roles = array();
      foreach (filter_get_roles_by_format($format) as $role_name) {
        $role = user_role_load($role_name);
        $roles[] = check_plain($role->label);
      }
      $roles_markup = $roles ? implode(', ', $roles) : t('No roles may use this format');
      $links['disable'] = array(
        'title' => t('Disable'),
        'href' => "admin/config/content/formats/$id/disable",
      );
    }
    $form['formats'][$id]['roles'] = array('#markup' => $roles_markup);
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
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save changes'));
  return $form;
}

/**
 * Form submission handler for filter_admin_overview().
 */
function filter_admin_overview_submit($form, &$form_state) {
  $formats = filter_formats();
  foreach ($form_state['values']['formats'] as $id => $data) {
    if (is_array($data) && isset($data['weight']) && $data['weight'] != $formats[$id]->weight) {
      // Update the weight of this filter.
      $formats[$id]->weight = $data['weight'];
      filter_format_save($formats[$id]);
    }
  }
  filter_formats_reset();
  backdrop_set_message(t('The text format ordering has been saved.'));
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
  $is_fallback = ($format->format == filter_fallback_format());

  $form_state['format'] = $format;
  $form['#tree'] = TRUE;
  $form['#attached']['js'][] = backdrop_get_path('module', 'filter') . '/js/filter.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'filter') . '/css/filter.css';

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $format->name,
    '#required' => TRUE,
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
  );

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
  $filters = $format->filters;

  // Prepare filters for form sections.
  foreach ($all_filter_info as $name => $filter_info) {
    // Create an empty filter object for new/unconfigured filters.
    if (!isset($filters[$name])) {
      $filters[$name] = (object) array(
        'format' => $format->format,
        'module' => $filter_info['module'],
        'name' => $name,
        'status' => 0,
        'weight' => $filter_info['weight'],
        'settings' => isset($filter_info['default settings']) ? $filter_info['default settings'] : array(),
      );
    }
  }
  $form['#filters'] = $filters;

  // Filter status.
  $form['filters']['status'] = array(
    '#type' => 'item',
    '#title' => t('Enabled filters'),
    '#prefix' => '<div id="filters-status-wrapper">',
    '#suffix' => '</div>',
  );
  foreach ($all_filter_info as $name => $filter_info) {
    $form['filters']['status'][$name] = array(
      '#type' => 'checkbox',
      '#title' => $filter_info['title'],
      '#default_value' => $filters[$name]->status,
      '#parents' => array('filters', $name, 'status'),
      '#description' => $filter_info['description'],
      '#weight' => $filter_info['weight'],
    );
  }

  // Filter order (tabledrag).
  $form['filters']['order'] = array(
    '#type' => 'item',
    '#title' => t('Filter processing order'),
    '#theme' => 'filter_admin_format_filter_order',
  );
  foreach ($all_filter_info as $name => $filter_info) {
    $form['filters']['order'][$name]['filter'] = array(
      '#markup' => check_plain($filter_info['title']),
    );
    $form['filters']['order'][$name]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $filter_info['title'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => $filters[$name]->weight,
      '#parents' => array('filters', $name, 'weight'),
    );
    $form['filters']['order'][$name]['#weight'] = $filters[$name]->weight;
  }

  // Filter settings.
  $form['filter_settings_title'] = array(
    '#type' => 'item',
    '#title' => t('Filter settings'),
  );
  $form['filter_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  foreach ($all_filter_info as $name => $filter_info) {
    if (isset($filter_info['settings callback'])) {
      $function = $filter_info['settings callback'];
      $settings_form = $function($form, $form_state, $filters[$name], $format);
      if (!empty($settings_form)) {
        $form['filters']['settings'][$name] = array(
          '#type' => 'fieldset',
          '#title' => $filter_info['title'],
          '#parents' => array('filters', $name, 'settings'),
          '#weight' => $filter_info['weight'],
          '#group' => 'filter_settings',
        );
        $form['filters']['settings'][$name] += $settings_form;
      }
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
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
  foreach ($form_state['values'] as $key => $value) {
    $format->$key = $value;
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

  $form_state['redirect'] = 'admin/config/content/formats';
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
    t('Disabled text formats are completely removed from the administrative interface, and any content stored with that format will not be displayed. This action cannot be undone.'),
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
