<?php
/**
 * @file
 * Admin page callbacks for the Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */

/**
 * Output a list of layouts that are managed.
 */
function layout_list_page() {
  $layouts = layout_load_all();
  $layout_template_info = layout_get_layout_template_info();

  // Group layouts by path.
  $layout_paths = array();
  $path_groups = array();
  $admin_path_groups = array();
  foreach ($layouts as $layout) {
    $path = $layout->getPath();
    // Admin layouts are to be added to the bottom of the list.
    if (path_is_admin($path)) {
      $admin_path_groups[$path][$layout->name] = $layout;
    }
    else {
      $path_groups[$path][$layout->name] = $layout;
    }
    // Build a list of all layout paths to check callbacks for.
    if ($path && !in_array($path, $layout_paths)) {
      $layout_paths[] = $path;
    }
  }

  // Put admin layouts at the bottom of the list.
  foreach ($admin_path_groups as $path => $name) {
    foreach ($name as $layout) {
      $path_groups[$path][$layout->name] = $layout;
    }
  }

  // Check layout paths for callbacks.
  if (!empty($layout_paths)) {
    $page_callbacks = db_query('
      SELECT path, page_callback
      FROM {menu_router}
      WHERE path IN (:paths)
      ', array(':paths' => $layout_paths))
      ->fetchAllKeyed();
  }
  else {
    $page_callbacks = array();
  }

  // Assemble the rows of the table.
  $stand_alone_rows = array();
  $override_rows = array();
  $default_rows = array();

  foreach ($path_groups as $path => $group) {
    // Print a row for rearranging a group of layouts.
    $operations = array(
      '#type' => 'dropbutton',
      '#links' => _layout_get_group_operations($path, $group),
    );
    $row = array();
    // Generate the heading row for non-default layout path groups.
    if ($path !== '') {
      if (strpos($path, '%') === FALSE) {
        $path_link = t('Path: ') . l($path, $path);
      }
      else {
        $path_link = t('System path: ') . check_plain($path);
      }

      $class = array();
      if (!$page_callbacks[$path]) {
        $class[] = 'missing-path';
        $path_link .= ' <span class="path-parenthetical">(' . t('missing callback') . ')</span>';
      }
      $row[] = array('data' => $path_link, 'class' => $class);
      // Empty columns are intentional so that they may be hidden with
      // priority-low class on the $header array.
      $row[] = '';
      $row[] = '';
      $row[] = '';

      if (count($operations['#links']) !== 0) {
        $row[] = backdrop_render($operations);
      }
      else {
        $row[] = '';
      }
    }
    // Add the heading row to the non-default layout path groups.
    if (isset($page_callbacks[$path]) && $page_callbacks[$path] == 'layout_page_callback') {
      $stand_alone_rows[] = array(
        'data' => $row,
        'class' => array('layout-group'),
      );
    }
    elseif ($path !== '') {
      $override_rows[] = array('data' => $row, 'class' => array('layout-group'));
    }

    foreach ($group as $layout) {
      $operations = array(
        '#type' => 'dropbutton',
        '#links' => _layout_get_operations($layout),
      );
      if ($layout->storage === LAYOUT_STORAGE_OVERRIDE) {
        $storage = t('Overridden');
      }
      elseif ($layout->storage === LAYOUT_STORAGE_DEFAULT) {
        $storage = t('Default (module-provided)');
      }
      else {
        $storage = t('Custom');
      }

      // Create a link to the settings page.
      $info = $layout_template_info[$layout->layout_template];
      $template = l($info['title'], 'admin/structure/layouts/manage/' . $layout->name . '/configure');

      $row = array();
      $row[] = theme('layout_info', array('layout' => $layout));
      $row[] = $template;
      $row[] = theme('layout_condition_info', array('layout' => $layout));
      $row[] = $storage;
      $row[] = backdrop_render($operations);
      $class = array('layout-row');
      if ($layout->disabled) {
        $class[] = 'disabled';
      }

      if (isset($page_callbacks[$path]) && $page_callbacks[$path] == 'layout_page_callback') {
        $stand_alone_rows[] = array('data' => $row, 'class' => $class);
      }
      elseif ($path === '') {
        $default_rows[] = array('data' => $row, 'class' => $class);
      }
      else {
        $override_rows[] = array('data' => $row, 'class' => $class);
      }
    }
  }

  $header = array(
    array('data' => t('Layout'), 'class' => array('layout-title')),
    array('data' => t('Template'), 'class' => array('layout-template', 'priority-low')),
    array('data' => t('Visibility conditions'), 'class' => array('layout-conditions', 'priority-low')),
    array('data' => t('Storage state'), 'class' => array('layout-status', 'priority-low')),
    array('data' => t('Operations'), 'class' => array('layout-operations')),
  );

  $help = '<p>' . t('There are three kinds of <em>Layouts</em>:') . '</p>';
  $help_items = array();
  $help_items[] = t('<strong>Layout pages</strong> are layouts that create new stand-alone pages on the site.  Without the layouts, these pages would not exist.');
  $help_items[] = t('<strong>Layout overrides</strong> are layouts that override existing pages on the site. Without these layout overrides, these pages would use other layouts. Which one is used will depend on the path and visibility conditions. If no other layouts match, one of the default layouts will be used.');
  $help_items[] = t('<strong>Default layouts</strong> are used for paths that have no other matching layouts.');
  $help .= theme('item_list', array('items' => $help_items));
  $help .= '<p>' . t('As a page is rendered, a <em>Layout</em> is selected as follows.') . '</p>';
  $help_items = array();
  $help_items[] = array(
    'data' => t('The <em>Path</em> for the page is compared to those in the alphabetical list here, and <em>Visibility conditions</em> are evaluated.'),
    'children' => array(
      t('Each path may have one or more layouts available. These are listed beneath the path.'),
      array(
        'data' => t('The top available layout for each path is is evaluated for matching visibility conditions first, followed by the second, etc.'),
        'children' => array(t('Available layouts can be reordered to adjust priority.')),
      ),
      t('The first layout to match all conditions will be selected.'),
    ),
  );
  $help_items[] = t('If no match is found, or if visibility conditions are not met, the page will use one of the <em>Default layouts</em> shown at the bottom of this page.');
  $help .= theme('item_list', array('items' => $help_items));
  $help .= '<p>' . t('Note: Paths used by the layout system are <em>menu router paths</em>, not normal paths or !link. For the default About page, <code>node/%</code> is the menu router path, <code>node/2</code> is the normal path, and <code>about</code> is the URL alias.', array('!link' => l(t('URL aliases'), 'admin/config/urls/path'))) . '</p>';
  $help .= '<p class="align-right">' . t('For more information and details, see the <a href="https://docs.backdropcms.org/documentation/layouts-and-templates" target="_blank">online documentation</a>.') . '</p>';

  $page = array();
  $page['help'] = array(
    '#type' => 'details',
    '#summary' => t('How layouts work'),
    '#details' => $help,
    '#attributes' => array(
      'class' => array('description'),
    ),
    '#weight' => -1,
  );
  $page['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';
  $page['stand_alone'] = array(
    '#type' => 'help',
    '#markup' => '<h2>' . t('Layout pages') . '</h2>' . t('These layouts will create new stand-alone pages on the site.'),
    '#weight' => 0,
  );
  $page['stand_alone_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $stand_alone_rows,
    '#attributes' => array('class' => array('layout-list')),
    '#empty' => t('No layout pages have been created yet.'),
    '#weight' => 1,
  );
  $page['override'] = array(
    '#type' => 'help',
    '#markup' => '<h2>' . t('Layout overrides') . '</h2>' . t('These layouts will override existing pages on the site.'),
    '#weight' => 2,
  );
  $page['override_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $override_rows,
    '#attributes' => array('class' => array('layout-list')),
    '#empty' => t('No layout overrides have been created yet.'),
    '#weight' => 3,
  );
  $page['default'] = array(
    '#type' => 'help',
    '#markup' => '<h2>' . t('Default layouts') . '</h2>' . t('Used as a fallback for all paths without a matching layout.'),
    '#weight' => 4,
  );
  $page['default_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $default_rows,
    '#attributes' => array('class' => array('layout-list')),
    '#weight' => 5,
  );
  return $page;
}

/**
 * Given a layout, return a list of operations that can be performed on it.
 */
function _layout_get_operations(Layout $layout) {
  $links = array();
  if ($layout->disabled && !$layout->isDefault()) {
    $links['enable'] = array(
      'title' => t('Enable layout'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/enable',
      'query' => array('token' => backdrop_get_token('layout-' . $layout->name)),
    );
  }
  $links['edit'] = array(
    'title' => t('Manage blocks'),
    'href' => 'admin/structure/layouts/manage/' . $layout->name,
  );
  $links['settings'] = array(
    'title' => t('Configure layout'),
    'href' => 'admin/structure/layouts/manage/' . $layout->name . '/configure',
  );
  if (!$layout->isDefault()) {
    $links['clone'] = array(
      'title' => t('Clone layout'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/clone',
    );
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export layout'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => 'Layouts',
          'name' => 'layout.layout.' . $layout->name,
        ),
      );
    }
    if (!$layout->disabled) {
      $links['disable'] = array(
        'title' => t('Disable layout'),
        'href' => 'admin/structure/layouts/manage/' . $layout->name . '/disable',
        'query' => array('token' => backdrop_get_token('layout-' . $layout->name)),
      );
    }
    if ($layout->storage === LAYOUT_STORAGE_NORMAL) {
      $links['delete'] = array(
        'title' => t('Delete layout'),
        'href' => 'admin/structure/layouts/manage/' . $layout->name . '/delete',
      );
    }
  }

  if ($layout->storage === LAYOUT_STORAGE_OVERRIDE) {
    $links['revert'] = array(
      'title' => t('Revert layout'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/delete',
    );
  }

  return $links;
}

/**
 * Given a path, return a list of operations for a group of layouts.
 */
function _layout_get_group_operations($path, array $group) {
  $links = array();

  // The default layouts have no operations.
  if ($path === '') {
    return $links;
  }

  $last_layout = end($group);
  reset($group);
  if ($last_layout->menu_item) {
    $links['menu_settings'] = array(
      'title' => t('Menu settings'),
      'href' => 'admin/structure/layouts/menu/' . $last_layout->menu_item->name,
    );
  }
  if (count($group) > 1) {
    $links['reorder'] = array(
      'title' => t('Reorder'),
      'href' => 'admin/structure/layouts/reorder',
      'query' => array('layouts' => array_keys($group)),
    );
  }
  return $links;
}

/**
 * Menu callback; Display the form for adding a new layout.
 */
function layout_add_page() {
  if (isset($_SESSION['layout_new_name'])) {
    $layout = layout_tempstore_load($_SESSION['layout_new_name']);
  }
  if (empty($layout)) {
    $config = array(
      'is_new' => TRUE,
    );
    $layout = new Layout($config);
  }
  backdrop_set_title(t('Add layout'));
  return backdrop_get_form('layout_settings_form', $layout);
}

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_settings_form($form, &$form_state, Layout $layout) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  form_load_include($form_state, 'inc', 'layout', 'layout.context.admin');

  $form_state['layout'] = &$layout;
  $form['#tree'] = TRUE;

  $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

  $messages = array();
  if ($layout->locked) {
    $messages['warning'] = array(
      layout_locked_message($layout, 'layout'),
    );
  }
  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => $messages,
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $form['title'] = array(
    '#title' => t('Layout name'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $layout->title,
    '#required' => TRUE,
    '#access' => !$layout->isDefault(),
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('title'),
      'exists' => 'layout_load',
    ),
    '#maxlength' => 128,
    '#default_value' => $layout->name,
    '#required' => TRUE,
    '#access' => !$layout->isDefault(),
  );
  $form['layout_template'] = array(
    '#title' => t('Layout template'),
    '#type' => 'radios',
    '#default_value' => $layout->layout_template,
    '#options' => array(),
    '#wrapper_attributes' => array('class' => array('clearfix', 'layout-options')),
    '#required' => TRUE,
  );

  // Get the list of layout template options. The list needs to be rebuilt (see
  // https://github.com/backdrop/backdrop-issues/issues/984)
  $all_template_info = layout_get_layout_template_info(NULL, TRUE);

  $excluded = config_get('layout.settings', 'excluded_templates');
  foreach ($all_template_info as $template_name => $template_info) {
    if (!in_array($template_name, $excluded)) {
      $form['layout_template']['#options'][$template_name] = theme('layout_template_option', array('template_info' => $template_info));
    }
  }

  // Path entry field with dynamic information about the entered path and
  // toggleable placeholder examples.
  $path = isset($form_state['values']['path']) ? $form_state['values']['path'] : $layout->getPath();
  $is_new = isset($layout->is_new) && $layout->is_new;
  $has_placeholder = strpos((string) $path, '%') !== FALSE;
  if (empty($path)) {
    $path_matches_text = t('Please enter a path.');
  }
  else {
    if (_layout_menu_router_path_exists($path)) {
      if ($has_placeholder) {
        if ($is_new) {
          $path_matches_text = t('This layout will override the default layout for existing page(s) whose paths match this pattern.');
        }
        else {
          $path_matches_text = t('This layout overrides the default layout for existing page(s) whose paths match this pattern.');
        }
      }
      else {
        if ($is_new) {
          $path_matches_text = t('This layout will override the default layout for an existing page whose path matches this pattern.');
        }
        else {
          $path_matches_text = t('This layout overrides the default layout for an existing page whose path matches this pattern.');
        }
      }
    }
    else {
      if ($has_placeholder) {
        if ($is_new) {
          $path_matches_text = t('This path will create pages whose paths match this pattern.');
        }
        else {
          $path_matches_text = t('This path provides pages whose paths match this pattern.');
        }
      }
      else {
        if ($is_new) {
          $path_matches_text = t('This path will create a page at this path.');
        }
        else {
          $path_matches_text = t('This path provides a page at this path.');
        }
      }
    }
  }
  $form['path_matches_text'] = array(
    '#type' => 'value',
    '#value' => $path_matches_text,
  );
  $path_matches = array(
    '#type' => 'container',
    '#access' => !$layout->isDefault(),
    '#attributes' => array(
      'id' => array('layout-path-matches'),
    ),
    'path_matches_text' => array('#markup' => $path_matches_text),
  );
  $path_matches_description = backdrop_render($path_matches);
  $toggle = ' <a class="layout-placeholder-examples-toggle" href="#">' . t('Show examples') . '<span class="arrow"></span></a>';
  $description_items = array(
    t('Content: <code>node/%</code>'),
    t('User accounts: <code>user/%</code>'),
    t('Taxonomy terms: <code>taxonomy/term/%</code>'),
    t('Comments: <code>comment/%</code>'),
  );
  $description_list = '<div class="layout-placeholder-examples">' . t('Some common examples of placeholders include:') . theme('item_list', array('items' => $description_items)) . '</div>';
  $form['path'] = array(
    '#title' => t('Path'),
    '#field_prefix' => url('', array('absolute' => TRUE)),
    '#type' => 'textfield',
    '#default_value' => $layout->getPath(),
    '#required' => TRUE,
    '#access' => !$layout->isDefault(),
    '#description' => $path_matches_description . t('Use the percent symbol ("%") to indicate a placeholder in the path.') . $toggle . $description_list,
  );

  $form['path_update'] = array(
    '#type' => 'submit',
    '#value' => t('Check path'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_path_rebuild',
    ),
    '#ajax' => array(
      'callback' => 'layout_settings_form_path_ajax',
      'disable' => FALSE,
      'progress' => 'none',
    ),
    '#attributes' => array(
      'data-layout-path-update' => 'true',
      'class' => array('js-hide'),
    ),
    '#access' => !$layout->isDefault(),
  );

  // Get all contexts except those provided by relationships.
  $contexts = $layout->getContexts(LayoutContext::USAGE_TYPE_ALL ^ LayoutContext::USAGE_TYPE_RELATIONSHIP);
  $built_in_contexts = isset($contexts['overrides_path']) ? 2 : 1;
  $context_description = t('Contexts provide additional information to blocks displayed within the layout. Most placeholders in the path will automatically create context(s).');
  $form['context_wrapper'] = array(
    '#title' => t('Contexts'),
    '#type' => 'item',
    '#id' => 'context-wrapper',
  );
  $form['context_wrapper']['#access'] = !$layout->isDefault();
  $form['context_wrapper']['#prefix'] = '<div id="layout-contexts">';
  $form['context_wrapper']['#suffix'] = '</div>';
  $form['context_wrapper']['context'] = array(
    '#type' => 'container',
    '#parents' => array('context'),
  );
  $form['context_wrapper']['context']['description'] = array(
    '#markup' => '',
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
  );
  $form['context_wrapper']['context']['links'] = array(
    '#theme' => 'layout_action_links',
  );
  $form['context_wrapper']['context']['links']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add context'),
    '#attributes' => array('class' => array('layout-link-button', 'layout-access-add')),
    '#validate' => array(),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_context_add',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );

  // Get contexts from relationships
  $relevant_relationships = layout_relationships_get_relevant_info($contexts);
  if (!empty($relevant_relationships)) {
    $form['context_wrapper']['context']['links']['add_relationship'] = array(
      '#type' => 'submit',
      '#name' => 'relationship_add_button',
      '#value' => t('Add relationship'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-add')),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_update_layout',
        'layout_settings_form_context_add',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
    );
    $context_description .= ' ' . t('Relationships create contexts from data provided by existing contexts.');
  }
  $form['context_wrapper']['context']['description']['#markup'] = $context_description;

  $all_context_info = _layout_get_all_info('layout_context');
  $context_options = array();
  foreach ($all_context_info as $plugin_name => $context_info) {
    if (empty($context_info['hidden'])) {
      $context_options[$plugin_name] = $context_info['title'];
    }
  }

  $form['context_wrapper']['context']['required'] = array(
    '#theme' => 'layout_settings_context_table',
    '#layout_path' => $layout->getPath(),
    '#access' => count($contexts) > $built_in_contexts,
  );

  foreach ($contexts as $context_key => $layout_context) {
    if ($layout_context->usageType !== LayoutContext::USAGE_TYPE_SYSTEM) {
      // Contexts that are locked to a particular position (such as node/%).
      if ($layout_context->position && $layout_context->required) {
        $form['context_wrapper']['context']['required'][$context_key]['summary'] = array(
          '#markup' => $layout_context->getAdminSummary($layout->getPath()),
        );
        if ($layout_context->locked) {
          $form['context_wrapper']['context']['required'][$context_key]['plugin'] = array(
            '#markup' => check_plain($all_context_info[$layout_context->plugin]['title']),
          );
        }
        else{
          $form['context_wrapper']['context']['required'][$context_key]['plugin'] = array(
            '#type' => 'select',
            '#options' => $context_options,
            '#default_value' => $layout_context->plugin,
          );
        }
      }
      // Custom contexts and required contexts that do not need position (such
      // as admin/dashboard).
      else {
        $form['context_wrapper']['context']['required'][$context_key]['summary'] = array(
          '#markup' => $layout_context->getAdminSummary($layout->getPath()),
        );
        $form['context_wrapper']['context']['required'][$context_key]['plugin'] = array(
          '#markup' => check_plain($all_context_info[$layout_context->plugin]['title']),
        );
      }
      if (!$layout_context->required) {
        $form['context_wrapper']['context']['required'][$context_key]['operations'] = array(
          '#type' => 'container',
          '#attributes' => array('class' => array('layout-operations')),
        );
        $form['context_wrapper']['context']['required'][$context_key]['operations']['remove'] = array(
          '#type' => 'submit',
          '#value' => t('Remove'),
          '#attributes' => array('class' => array('layout-link-button', 'layout-context-remove')),
          '#validate' => array(),
          '#submit' => array(
            'layout_settings_form_context_remove',
            'layout_settings_form_update_layout',
          ),
          '#ajax' => array(
            'callback' => 'layout_ajax_form_update',
          ),
          '#name' => 'conditions_' . $context_key . '_remove',
        );
        $form['context_wrapper']['context']['required'][$context_key]['operations']['configure'] = array(
          '#type' => 'submit',
          '#value' => t('Configure'),
          '#attributes' => array('class' => array('layout-link-button', 'layout-context-configure')),
          '#validate' => array(
            'layout_settings_form_validate',
          ),
          '#submit' => array(
            'layout_settings_form_context_edit',
          ),
          '#ajax' => array(
            'callback' => 'layout_ajax_form_open_dialog',
          ),
          '#name' => 'conditions_' . $context_key . '_configure',
        );
      }
    }
  }

  $all_relationship_info = _layout_get_all_info('layout_relationship');
  foreach ($layout->relationships as $relationship_key => $relationship) {
    $form['context_wrapper']['context']['required'][$relationship_key]['summary'] = array(
      '#markup' => $relationship->getAdminSummary(),
    );
    $form['context_wrapper']['context']['required'][$relationship_key]['plugin'] = array(
      '#markup' => isset($all_relationship_info[$relationship->plugin]['title']) ?
        check_plain($all_relationship_info[$relationship->plugin]['title']) :
        t('Broken'),
    );
    $form['context_wrapper']['context']['required'][$relationship_key]['operations'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('layout-operations')),
    );
    $form['context_wrapper']['context']['required'][$relationship_key]['operations']['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-context-remove')),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_context_relationship_remove',
        'layout_settings_form_update_layout',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_update',
      ),
      '#name' => 'conditions_' . $relationship_key . '_remove',
    );
    $form['context_wrapper']['context']['required'][$relationship_key]['operations']['configure'] = array(
      '#type' => 'submit',
      '#value' => t('Configure'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-context-configure')),
      '#validate' => array(
        'layout_settings_form_validate',
      ),
      '#submit' => array(
        'layout_settings_form_context_relationship_edit',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
      '#name' => 'conditions_' . $relationship_key . '_configure',
    );
  }

  // Display a notice if overriding a system path that normally contains
  // placeholders e.g. node/1 instead of node/%. This is usually (but not always)
  // a misconfiguration of the layout.
  $layout_path = $layout->getPath();
  $router_item = menu_get_item($layout_path);
  if ($router_item && $layout_path) {
    $router_item_path = (string) $router_item['path'];
    if ($router_item_path !== $layout_path && substr_count($router_item_path, '/') === substr_count($layout_path, '/') && layout_context_required_by_path($router_item_path)) {
      $message = t('The path entered will create a new page and disable the current "@old_path". If you intended to add a layout for "@old_path", use the path "@new_path" and add a URL path condition for "@old_path".', array('@new_path' => $router_item['path'], '@old_path' => $layout->getPath()));
      if (backdrop_is_ajax()) {
        backdrop_set_message($message, 'warning');
      }
      else {
        $form['messages']['#messages']['warning'][] = $message;
      }
    }
  }

  $form['conditions'] = array(
    '#title' => t('Visibility conditions'),
    '#type' => 'item',
    '#id' => 'layout-access',
    '#access' => !$layout->isDefault(),
  );
  $form['conditions']['description'] = array(
    '#markup' => t('Visibility conditions allow this layout to selectively apply to different situations, such as different types of content at the same path.'),
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
  );
  $form['conditions']['links'] = array(
    '#theme' => 'layout_action_links',
  );
  $form['conditions']['links']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add visibility condition'),
    '#attributes' => array('class' => array('layout-link-button', 'layout-access-add')),
    '#validate' => array(),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_condition_add',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );

  $form['conditions']['active'] = array(
    '#type' => 'container',
    '#theme' => 'layout_conditions',
    '#attributes' => array('class' => array('layout-access-list')),
  );
  foreach ($layout->conditions as $access_key => $layout_access) {
    $form['conditions']['active'][$access_key] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('layout-condition')),
      '#id' => NULL,
    );
    $form['conditions']['active'][$access_key]['label'] = array(
      '#markup' => $layout_access->summary(),
    );
    $form['conditions']['active'][$access_key]['operations'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('layout-operations')),
    );
    $form['conditions']['active'][$access_key]['operations']['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-remove')),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_condition_remove',
        'layout_settings_form_update_layout',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_update',
      ),
      '#name' => 'conditions_' . $access_key . '_remove',
    );
    $form['conditions']['active'][$access_key]['operations']['configure'] = array(
      '#type' => 'submit',
      '#value' => t('Configure'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-configure')),
      '#validate' => array(
        'layout_settings_form_validate',
      ),
      '#submit' => array(
        'layout_settings_form_condition_edit',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
      '#name' => 'conditions_' . $access_key . '_configure',
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($layout->is_new) ? t('Save layout') : t('Create layout'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_save_submit',
    ),
  );
  if (isset($_SESSION['layout_new_name']) || isset($layout->locked)) {
    $form['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#limit_validation_errors' => array(array('actions', 'reset')),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_reset',
      ),
    );
  }

  return $form;
}

/**
 * Helper function to determine whether a layout path is an existing menu router
 * path.
 */
function _layout_menu_router_path_exists($path) {
  $path_matches = db_query('
    SELECT path, page_callback
    FROM {menu_router}
    WHERE path = :path
    ', array(':path' => $path))
    ->fetchAll();
  return !empty($path_matches);
}

/**
 * Validates layout_settings_form(), ensuring a valid path.
 */
function layout_settings_form_validate(&$form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];

  // Remove trailing and preceding slashes.
  $path = $form_state['values']['path'] = trim($form_state['values']['path'], '/');

  if (strpos($path, '%') === 0) {
    form_error($form['path'], t('The first part of a path may not be a placeholder.'));
  }

  // Ensure the path is not already an alias to something else.
  if (strpos($path, '%') === FALSE) {
    $alias = db_query('SELECT * FROM {url_alias} WHERE alias = :path', array(':path' => $path))->fetchObject();
    if ($alias) {
      $error = t('That path is currently assigned to be an alias for "@alias".', array('@alias' => $alias->source));
      if (user_access('administer url aliases')) {
        $error .= ' ' . t('<a href="!url">Delete the alias first</a>, then save this layout.', array('!url' => url('admin/config/urls/path/delete/' . $alias->pid, array('query' => array('destination' => $_GET['q'])))));
      }
      form_error($form['path'], $error);
    }
  }

  // Ensure path is properly formed.
  $parts = explode('/', $path);
  foreach ($parts as $part) {
    $wildcard_position = strpos($part, '%');
    if ($wildcard_position !== FALSE && ($wildcard_position > 0 || strlen($part) > 1)) {
      form_error($form['path'], t('Placeholders must be the only character in their part of the path i.e. "@part" should be "%".', array('@part' => $part)));
    }
  }

  // Ensure that all conditions have context requirements met.
  $context_plugins = array(
    'user', // The user plugin is always available for the current user.
  );
  $contexts = layout_context_required_by_path($path);
  foreach ($contexts as $context) {
    $context_plugins[] = $context->plugin;
  }
  foreach ($layout->conditions as $key => $condition) {
    $required_contexts = $condition->getRequiredContexts();
    if (array_diff($required_contexts, $context_plugins)) {
      form_error($form['conditions']['active'][$key], t('The condition \'!condition\' does not have the required contexts at this path. Remove the condition to save the layout.', array('!condition' => $condition->summary())));
    }
  }
}

/**
 * Submit handler for layout_settings_form().
 *
 * Updates available contexts and information about path matching.
 */
function layout_settings_form_path_rebuild($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX handler for layout_settings_form().
 *
 * Returns updated contexts and information about path matching.
 */
function layout_settings_form_path_ajax($form, &$form_state) {
  $commands[] = ajax_command_remove('.l-messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
  $commands[] = ajax_command_html('#layout-path-matches', $form['path_matches_text']['#value']);
  $commands[] = ajax_command_html('#layout-contexts', backdrop_render($form['context_wrapper']));
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Submit handler for layout_settings_form() that lets the user add a condition.
 */
function layout_settings_form_condition_add($form, &$form_state) {
  // Remove destination flags when adding conditions, otherwise the user will
  // be taken to the destination instead of adding a condition. This only
  // affects non-JS behavior.
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/condition/add';
}

/**
 * Submit handler for layout_settings_form() that removes a condition.
 */
function layout_settings_form_condition_remove($form, &$form_state) {
  $removed_condition = $form_state['clicked_button']['#array_parents'][2];
  unset($form_state['layout']->conditions[$removed_condition]);
  layout_set_layout_tempstore($form_state['layout']);

  $form_state['ajax_update'] = array('conditions');
}

/**
 * Submit handler for layout_settings_form() that edits an condition.
 */
function layout_settings_form_condition_edit($form, &$form_state) {
  $edit_condition = $form_state['clicked_button']['#array_parents'][2];
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $form_state['layout']->name . '/condition/edit/layout/' . $edit_condition;
}

/**
 * Submit handler for layout_settings_form() that saves in-progress values.
 */
function layout_settings_form_update_layout($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];

  // Keep track of the original name.
  if (isset($form_state['values']['name'])) {
    if ($form_state['values']['name'] !== $layout->name && !isset($layout->original_name)) {
      $layout->original_name = $layout->name;
    }
  }

  // Update main properties.
  if (isset($form_state['values']['title'])) {
    $layout->title = $form_state['values']['title'];
  }
  if (isset($form_state['values']['name'])) {
    $layout->name = $form_state['values']['name'];
  }
  if (isset($form_state['values']['layout_template'])) {
    $old_template = $layout->layout_template;
    $new_template = $form_state['values']['layout_template'];
    if ($old_template != $new_template) {
      $region_mapping = layout_get_legacy_region_mapping($old_template, $new_template);
      // If there is no theme cache entry for this template, rebuild theme cache
      // to ensure that the new template is loaded.
      $theme_entries = theme_get_registry(FALSE);
      if (empty($theme_entries['layout__' . $new_template])) {
        backdrop_theme_rebuild();
      }
      $layout->setLayoutTemplate($new_template, $region_mapping);
    }
    if (!empty($layout->orphaned_blocks)) {
      $orphaned_blocks_titles = array();
      $layout_template_info = layout_get_layout_template_info($form_state['values']['layout_template']);
      foreach ($layout->orphaned_blocks as $uuid) {
        $orphaned_blocks_titles[] = $layout->content[$uuid]->getAdminTitle();
      }
      $message = t('The following blocks have been reassigned to the %new_region region. The regions they were placed in previously do not exist in the new layout template. You can move them to more appropriate regions:', array(
        '%new_region' => $layout_template_info['regions'][$layout->refuge_region]));
      $message .= theme('item_list', array('items' => $orphaned_blocks_titles));
      backdrop_set_message($message, 'warning');
    }
  }
  if (isset($form_state['values']['path'])) {
    $layout->setPath($form_state['values']['path']);
  }

  // Update contexts.
  // @todo: Provide methods/API for setting menu item arguments.
  if ($layout->menu_item && isset($form_state['values']['context'])) {
    $plugin_counts = array();
    $arguments = array();

    foreach ($layout->getContexts() as $position => $layout_context) {
      if (isset($form_state['values']['context']['required'][$position]['plugin'])) {
        $argument_config = $form_state['values']['context']['required'][$position];
        $plugin = $argument_config['plugin'];
      }
      else {
        $plugin = $layout_context->plugin;
      }

      // Keep track of all utilized plugins, both required and locked.
      $plugin_counts[$plugin] = isset($plugin_counts[$plugin]) ? $plugin_counts[$plugin] + 1 : 1;

      // Locked contexts are forced to be at this path, so they do not need to
      // be saved in the menu item's argument configuration.
      if ($layout_context->locked) {
        continue;
      }

      // Give an automatically generated name such as "node" or "string2".
      $name = $plugin . (($plugin_counts[$plugin] === 1) ? '' : $plugin_counts[$plugin]);

      // Persist machine names if they already exists for this argument.
      if (isset($layout->menu_item->arguments[$position])) {
        $existing_argument = $layout->menu_item->arguments[$position];
        if ($existing_argument['plugin'] === $plugin) {
          $name = $existing_argument['name'];
        }
      }

      $arguments[$position] = array(
        'plugin' => $plugin,
        'name' => $name,
        'position' => $position,
      );
    }
    $layout->menu_item->arguments = $arguments;
  }

  if (!empty($layout->is_new)) {
    $_SESSION['layout_new_name'] = $layout->name;
  }
  else {
    if ($form_state['values']['name'] !== $layout->name) {
      $layout->original_name = $form_state['values']['name'];
    }
  }
  layout_set_layout_tempstore($layout);
}

/**
 * Returns an array of mappings from old region names to new.
 */
function layout_get_legacy_region_mapping($old, $new) {
  $region_mapping = array();
  $legacy = array('one_column', 'two_column', 'two_column_flipped', 'three_three_four_column');
  $bootstrap = array('boxton', 'geary', 'harris', 'moscone', 'moscone_flipped', 'rolph', 'simmons', 'sutro', 'taylor', 'taylor_flipped');

  if (in_array($old, $legacy) && in_array($new, $bootstrap)) {
    $region_mapping = array(
      'sidebar_first'       => 'sidebar',
      'sidebar_second'      => 'sidebar2',
      'triptych_first'      => 'third1',
      'triptych_middle'     => 'third2',
      'triptych_last'       => 'third3',
      'footer_firstcolumn'  => 'quarter1',
      'footer_secondcolumn' => 'quarter2',
      'footer_thirdcolumn'  => 'quarter3',
      'footer_fourthcolumn' => 'quarter4',
    );
  }

  if (in_array($old, $bootstrap) && in_array($new, $legacy)) {
    $region_mapping = array(
      'sidebar'  => 'sidebar_first',
      'sidebar2' => 'sidebar_second',
      'third1'   => 'triptych_first',
      'third2'   => 'triptych_middle',
      'third3'   => 'triptych_last',
      'quarter1' => 'footer_firstcolumn',
      'quarter2' => 'footer_secondcolumn',
      'quarter3' => 'footer_thirdcolumn',
      'quarter4' => 'footer_fourthcolumn',
    );
  }

  return $region_mapping;
}

/**
 * Submit handler for layout_settings_form() that resets in-progress changes.
 */
function layout_settings_form_reset($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  if (isset($_SESSION['layout_new_name'])) {
    unset($_SESSION['layout_new_name']);
  }
  layout_clear_layout_tempstore($layout->name);
  backdrop_set_message(t('Layout changes discarded.'));
  $form_state['redirect'] = 'admin/structure/layouts';
  $form_state['no_redirect'] = FALSE;
}

/**
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_settings_form_save_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  if (isset($_SESSION['layout_new_name'])) {
    unset($_SESSION['layout_new_name']);
  }

  // If this is a new layout, populate blocks from default layout.
  if (!empty($layout->is_new)) {
    $default_layout_name = path_is_admin($layout->getPath()) ? 'admin_default' : 'default';
    $default_layout = layout_load($default_layout_name);
    // Get the template being used by this layout.
    $template_info = layout_get_layout_template_info($layout->layout_template);
    foreach ($default_layout->positions as $region => $region_block_list) {
      if (array_key_exists($region, $template_info['regions'])) {
        foreach ($region_block_list as $uuid) {
          if (isset($default_layout->content[$uuid])) {
            $block = $default_layout->content[$uuid];
            $new_block = $block->getClone();
            // Only add the block from the default if required contexts are
            // present for each block.
            if ($layout->hasContexts($new_block->getRequiredContexts())) {
              $layout->positions[$region][] = $new_block->uuid;
              $layout->content[$new_block->uuid] = $new_block;
            }
          }
        }
      }
    }
  }

  $layout->save();

  layout_clear_layout_tempstore($layout->name);
  if (!empty($layout->is_new)) {
    backdrop_set_message(t('Layout created. Blocks may now be added to this layout.'));
  }
  else {
    backdrop_set_message(t('Layout saved.'));
  }
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name;
}

/**
 * Form callback; Main form for editing a layout's content.
 *
 * No validation function is necessary, as all 'validation' is handled
 * either in the lead-up to form rendering (through the selection of
 * specified content types) or by the validation functions specific to
 * the ajax modals & content types.
 *
 * @ingroup forms
 */
function layout_content_form($form, &$form_state, Layout $layout, $renderer_name = NULL) {
  $path = isset($layout->menu_item->path) ? '(' . $layout->menu_item->path . ')' : NULL;
  backdrop_set_title(t('!layout !path', array('!layout' => $layout->title, '!path' => $path)));
  if (!isset($form_state['layout'])) {
    $form_state['layout'] = $layout;
  }
  if ($renderer_name) {
    $renderer = layout_create_renderer($renderer_name, $layout);
  }
  else {
    $renderer = layout_create_renderer('editor', $layout);
  }

  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = $renderer;
  }

  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];

  // Although layout is already set in $form_state, we need this to make it
  // available to the theme layer.
  $form['#layout'] = $layout;

  $form['#attached'] = array(
    'library' => array(
      array('system', 'backdrop.ajax'),
      array('system', 'backdrop.announce'),
    ),
  );

  $form['messages'] = array(
    '#markup' => $layout->locked ? '<div class="messages warning">' . layout_locked_message($layout, 'layout') . '</div>' : '',
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  // Add a link to preview the page.
  $link_path = $layout->getPath();
  // A null path returns the site root. See https://github.com/backdrop/backdrop-issues/issues/4383
  if (!is_null($link_path) && backdrop_valid_path($link_path)) {
    $form['top']['#theme_wrappers'] = array('container');
    $form['top']['#attributes']['class'] = array('edit-layout-top', 'clearfix');

    $form['top']['button'] = array(
      '#type' => 'dropbutton',
    );
    $form['top']['button']['#links'] = array(
      array(
        'title' => t('View page'),
        'href' => $link_path,
      )
    );
  }

  $form['content'] = array('#tree' => TRUE);
  $form['content']['block'] = array('#tree' => TRUE);

  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  foreach ($renderer->layout_info['regions'] as $region_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($layout->positions[$region_id])) {
      $layout->positions[$region_id] = array();
    }

    $form['content']['positions'][$region_id] = array(
      // Use 'hidden' instead of 'value' so the JS can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', $layout->positions[$region_id]),
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save layout'),
    '#id' => 'layout-edit-save',
    '#submit' => array('layout_content_form_submit'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
    '#limit_validation_errors' => array(array('actions', 'reset')),
    '#submit' => array('layout_settings_form_reset'),
  );

  return $form;
}

/**
 * Validate the layout settings form.
 */
function layout_content_form_validate($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];

  // Ensure that the main page content block is only positioned once and that
  // default layouts include the main page content.
  $main_blocks_found = 0;
  foreach ($layout->content as $block) {
    if ($block->module === 'system' && $block->delta === 'main') {
      $main_blocks_found++;
    }
  }
  if ($main_blocks_found > 1) {
    form_error($form['content']['positions'], t('The "@block" block may only be added once to this layout.', array('@block' => t('Main page content'))));
  }
  elseif ($main_blocks_found === 0 && ($layout->name === 'default' || $layout->name === 'admin_default')) {
    form_error($form['content']['positions'], t('The "@block" block must be added to this layout.', array('@block' => t('Main page content'))));
  }

  // Ensure that no block is positioned more than once. This is merely a sanity
  // check, it should not occur within the normal interface.
  $all_uuids = array();
  foreach ($layout->positions as $uuids) {
    foreach ($uuids as $uuid) {
      if (in_array($uuid, $all_uuids)) {
        form_error($form['content']['positions'], t('The "@block" block has incorrectly been positioned multiple times. This may indicate an incorrectly saved layout configuration.', array('@block' => $layout->content[$uuid]->getAdminTitle())));
      }
      $all_uuids[] = $uuid;
    }
  }

  // Let the layout do any specific validations.
  $layout->formValidate($form, $form_state);
}

/**
 * Handle form submission of the display content editor.
 *
 * This reads the location of the various blocks from the form, which will
 * have been modified via JavaScript, rearranges them and then saves
 * the display.
 */
function layout_content_form_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = &$form_state['layout'];

  // Update the block positions based on the hidden position elements.
  if (!empty($form_state['values']['content']['positions'])) {
    $new_content = array();
    $new_positions = array();

    foreach ($form_state['values']['content']['positions'] as $region_id => $blocks) {
      $new_positions[$region_id] = array();
      if ($blocks) {
        $uuids = array_filter(explode(',', $blocks));
        foreach ($uuids as $uuid) {
          if (isset($layout->content[$uuid])) {
            $new_content[$uuid] = $layout->content[$uuid];
            $new_positions[$region_id][] = $uuid;
          }
        }
      }
    }
    $layout->content = $new_content;
    $layout->positions = $new_positions;
  }

  // Save title settings.
  if (isset($form_state['values']['title_display'])) {
    $layout->settings['title'] = $form_state['values']['title'];
    $layout->settings['title_display'] = $form_state['values']['title_display'];
  }

  // Let layouts save their own settings if needed.
  $layout->formSubmit($form, $form_state);

  // Save the entire layout to configuration.
  $layout->save();

  // Delete the tempstore copy of the layout.
  layout_clear_layout_tempstore($layout->name);

  backdrop_set_message(t('Layout content saved.'));
}

/**
 * Menu callback; Display a list of blocks to be added to a layout.
 */
function layout_block_add_page(Layout $layout, $renderer_name, $region_name, $block_key = NULL) {
  /* @var Layout $layout */
  $form_state['layout'] = $layout;
  $template_info = layout_get_layout_template_info($layout->layout_template);
  if (!array_key_exists($region_name, $template_info['regions'])) {
    return MENU_NOT_FOUND;
  }

  // If a block has been selected, return the block configuration form.
  if (isset($block_key)) {
    list($module, $delta) = explode(':', $block_key);
    $block_info = layout_get_block_info($module, $delta);
    if (!$block_info) {
      return MENU_NOT_FOUND;
    }

    $block = layout_create_handler('block', $block_key);
    return backdrop_get_form('layout_block_configure_form', $layout, $block, $renderer_name, $region_name);
  }

  $blocks = layout_get_block_info();

  // Assemble all blocks into a single list and sort.
  $block_list = array();
  foreach ($blocks as $module => $module_blocks) {
    if (empty($module_blocks)) {
      continue;
    }
    foreach ($module_blocks as $delta => $block_info) {
      /* @var Block $parent_block */
      $parent_block = layout_create_handler('block', $module . ':' . $delta);
      $children = $parent_block->getChildren();
      if (is_array($children)) {
        foreach ($children as $child_id => $child_info) {
          // Skip blocks that don't have context requirements met.
          $required_contexts = isset($child_info['required contexts']) ? $child_info['required contexts'] : array();
          if ($layout->hasContexts($required_contexts)) {
            $block_list[$module . ':' . $delta . ':' . $child_id] = $child_info;
          }
        }
      }
      else {
        $required_contexts = isset($block_info['required contexts']) ? $block_info['required contexts'] : array();
        // Skip blocks that don't have context requirements met.
        if ($layout->hasContexts($required_contexts)) {
          $block_list[$module . ':' . $delta] = $block_info;
        }
      }
    }
  }
  backdrop_sort($block_list, array('info' => SORT_STRING));

  $output['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Search'),
    '#attributes' => array(
      'id' => array('layout-block-list-search'),
    ),
    // Hide the search if no JavaScript is available.
    '#wrapper_attributes' => array(
      'class' => array('js-show'),
    ),
  );
  $output['block_list'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-block-list'),
    ),
  );

  $output['block_list']['block_list_inner'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('layout-block-list-wrapper'),
    ),
  );

  foreach ($block_list as $key => $block_info) {
    $output['block_list']['block_list_inner'][$key] = array(
      '#type' => 'item',
      '#markup' => l(
        $block_info['info'],
        'admin/structure/layouts/manage/' . $layout->name . '/add-block/' . $renderer_name . '/' . $region_name . '/' . $key,
        array(
          'attributes' => array(
            'class' => array('use-ajax block-item'),
            'data-dialog' => TRUE,
            'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
          )
        )
      ),
      '#description' => isset($block_info['description']) ? filter_xss($block_info['description']) : NULL,
      '#wrapper_attributes' => array('class' => array('layout-block-add-row'))
    );
  }

  return $output;
}

/**
 * Form callback; Configure layout title type.
 *
 * @ingroup forms
 */
function layout_title_settings_form($form, &$form_state, Layout $layout) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $form_state['layout'] = $layout;
  $form_state['menu_item'] = NULL;

  $form['title_display']['#tree'] = FALSE;
  $form['title_display']['title_display'] = array(
    '#type' => 'radios',
    '#default_value' => $layout->settings['title_display'],
    '#options' => array(
      LAYOUT_TITLE_DEFAULT => t('Default page title'),
      LAYOUT_TITLE_CUSTOM => t('Custom page title'),
      LAYOUT_TITLE_BLOCK => t('Copy from block'),
      LAYOUT_TITLE_NONE => t('No title'),
    ),
  );

  $form['title_display']['title_display'][LAYOUT_TITLE_DEFAULT]['#description'] = t('The page title will pull from a default source. For example: on the user account page, a username may be used.');
  $form['title_display']['title_display'][LAYOUT_TITLE_CUSTOM]['#description'] = t('Provide a custom page title for this layout.');
  $form['title_display']['title_display'][LAYOUT_TITLE_BLOCK]['#description'] = t('Copy the page title from a block for this layout.');
  $form['title_display']['title_display'][LAYOUT_TITLE_NONE]['#description'] = t('Hide the page title for this layout.');

  $form['title_display']['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $layout->settings['title'],
    '#title' => t('Custom page title'),
    '#description' => t('If left blank, a default title may be used.'),
    '#states' => array(
      'visible' => array(
        ':input[name="title_display"]' => array('value' => LAYOUT_TITLE_CUSTOM),
      ),
    ),
    '#maxlength' => 255,
  );

  $block_options = array();
  foreach ($layout->content as $uuid => $block) {
    $title = strip_tags($block->getAdminTitle());
    if (!empty($title) && $block->delta !== 'custom_block' && !is_a($block, 'PageComponents')) {
      $block_options[$uuid] = $title;
    }
  }
  asort($block_options);

  $form['title_display']['title_block'] = array(
    '#type' => 'select',
    '#default_value' => $layout->settings['title_block'],
    '#title' => t('Block to copy'),
    '#description' => t('The page title will be taken from this block\'s title.'),
    '#states' => array(
      'visible' => array(
        ':input[name="title_display"]' => array('value' => LAYOUT_TITLE_BLOCK),
      ),
    ),
    '#options' => $block_options,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#attributes' => array('class' => array('layout-title-button')),
    '#submit' => array(
      'layout_title_settings_form_submit',
    ),
    '#save_in_progress' => TRUE,
    '#ajax' => array(
      'callback' => 'layout_ajax_form_save_title_dialog',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_title_settings_form() to change title settings.
 */
function layout_title_settings_form_submit($form, &$form_state) {
  $layout = $form_state['layout'];

  // Save title settings.
  if (isset($form_state['values']['title_display'])) {
    $layout->settings['title'] = $form_state['values']['title'];
    $layout->settings['title_block'] = $form_state['values']['title_block'];
    $layout->settings['title_display'] = $form_state['values']['title_display'];
  }

  // Set the part of the form to be rebuilt via ajax.
  $form_state['ajax_update'] = array('title_display');

  $layout->in_progress['section'] = 'title';
  layout_set_layout_tempstore($layout);
}

/**
 * Menu callback; Redirect to the layout configuration form.
 *
 * This menu callback is utilized by contextual links.
 */
function layout_block_configure_redirect($layout, $block_uuid) {
  $destination = backdrop_get_destination();
  if (isset($_GET['destination'])) {
    unset($_GET['destination']);
  }
  backdrop_goto('admin/structure/layouts/manage/' . $layout->name, array('query' => $destination));
}

/**
 * Form callback; Add or edit a block within a layout.
 *
 * @ingroup forms
 */
function layout_block_configure_form($form, &$form_state, Layout $layout, Block $block, $renderer_name, $region_name = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = layout_create_renderer($renderer_name, $layout);
  }
  if (!isset($region_name)) {
    $region_name = $layout->getBlockPosition($block->uuid);
  }

  if (empty($block->uuid)) {
    $form['#title'] = t('Add block "!title"', array('!title' => $block->getAdminTitle()));
  }
  else {
    $form['#title'] = t('Configure block "!title"', array('!title' => $block->getAdminTitle()));
  }

  $template_info = layout_get_layout_template_info($layout->layout_template);
  $form['region'] = array(
    '#type' => 'select',
    '#title' => t('Region'),
    '#default_value' => $region_name,
    '#options' => $template_info['regions'],
    '#access' => backdrop_is_html(), // Only show this on non-JS requests.
  );

  // We overload the layout object with an "in_progress" key to hold information
  // about the block currently being edited, this eases the multi-dialog
  // workflow for a sequence of dialogs affecting existing or new blocks.
  if (!isset($layout->in_progress['block']) || $layout->in_progress['block']->uuid !== $block->uuid) {
    $layout->in_progress = array(
      'block' => $block,
      'region_name' => $region_name,
      'renderer_name' => $renderer_name,
      'section' => NULL,
    );
  }
  // If resuming an in-progress edit, pull out the region name.
  else {
    $form['region']['#default_value'] = $layout->in_progress['region_name'];
  }

  $form['actions']['#weight'] = 999;
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($block->uuid) ? t('Add block') : t('Update block'),
  );

  $form['#tree'] = TRUE;
  $block->form($form, $form_state);

  /* Style settings: */
  $form['style'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style settings'),
    '#collapsed' => $layout->in_progress['section'] !== 'style',
    '#collapsible' => TRUE,
    '#weight' => 10,
  );
  $styles = _layout_get_all_info('layout_style');
  $style_options = array();
  foreach ($styles as $style_name => $style_info) {
    $style_options[$style_name] = $style_info['title'];
  }
  $current_style = $block->settings['style'];
  if (isset($form_state['values']['style'])) {
    $current_style = $form_state['values']['style'];
  }
  $form['style']['style'] = array(
    '#type' => 'select',
    '#title' => t('Style'),
    '#options' => $style_options,
    '#default_value' => $current_style,
    '#ajax' => array(
      'wrapper' => 'block_style_settings',
      'callback' => 'layout_block_configure_ajax_style'
    ),
    '#parents' => array('style'),
  );
  $form['style']['style_settings'] = array(
    '#type' => 'container',
    '#id' => 'block_style_settings',
    '#parents' => array('style_settings'),
  );

  // The style may change in a form rebuild, so we can't use $block->style
  // directly, instead crate a new style handler and use that form.
  $block->style = layout_create_handler('layout_style', $current_style, $block->style->toArray());
  $block->style->form($form['style']['style_settings'], $form_state);
  /* End style settings. */

  /* Visibility conditions: */
  $form['conditions'] = array(
    '#title' => t('Visibility conditions'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => $layout->in_progress['section'] !== 'conditions',
    '#description' => t('Visibility conditions allow this block to be shown only in certain situations, such as for certain roles or types of content.'),
    '#id' => 'block-access',
    '#weight' => 20,
  );
  $form['conditions']['links'] = array(
    '#theme' => 'layout_action_links',
  );
  $form['conditions']['links']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add visibility condition'),
    '#attributes' => array('class' => array('layout-link-button', 'layout-access-add')),
    '#submit' => array(
      'layout_block_configure_form_submit',
      'layout_block_configure_form_condition_add',
    ),
    '#save_in_progress' => TRUE, // See layout_block_configure_form_submit().
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );
  $form['conditions']['active'] = array(
    '#type' => 'container',
    '#theme' => 'layout_conditions',
    '#attributes' => array('class' => array('layout-access-list')),
  );
  foreach ($block->conditions as $access_key => $block_access) {
    $form['conditions']['active'][$access_key] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('layout-condition')),
      '#id' => NULL,
    );
    $form['conditions']['active'][$access_key]['label'] = array(
      '#markup' => $block_access->summary(),
    );
    $form['conditions']['active'][$access_key]['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-remove')),
      '#submit' => array(
        'layout_block_configure_form_condition_remove',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_update',
      ),
      '#name' => 'conditions' . $access_key . 'remove',
    );
    $form['conditions']['active'][$access_key]['configure'] = array(
      '#type' => 'submit',
      '#value' => t('Configure'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-configure')),
      '#submit' => array(
        'layout_block_configure_form_condition_edit',
      ),
      '#save_in_progress' => TRUE, // See layout_block_configure_form_submit().
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
      '#name' => 'conditions' . $access_key . 'remove',
    );
  }
  /* End visibility conditions. */

  // Note that we specifically use backdrop_is_ajax() in addition to
  // backdrop_is_dialog() to ensure the #ajax callback is specified when the
  // style dropdown rebuilds the form on changes.
  if (backdrop_is_dialog() || backdrop_is_ajax()) {
    $form['actions']['submit']['#ajax'] = array(
      'callback' => 'layout_block_configure_ajax_update',
    );
  }

  return $form;
}

/**
 * AJAX callback to update the style settings within the block configure form.
 */
function layout_block_configure_ajax_style($form, $form_state) {
  return $form['style']['style_settings'];
}

/**
 * Submit handler for layout_block_configure_form() to add a condition.
 */
function layout_block_configure_form_condition_add($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];

  $layout->in_progress['section'] = 'conditions';
  layout_set_layout_tempstore($layout);
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/condition/add/' . $block->uuid;
}

/**
 * Submit handler for layout_block_configure_form() to edit a condition.
 */
function layout_block_configure_form_condition_edit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];

  $edit_condition = $form_state['clicked_button']['#array_parents'][2];
  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/condition/edit/' . $block->uuid . '/' . $edit_condition;
}

/**
 * Submit handler for layout_block_configure_form() to remove a condition.
 */
function layout_block_configure_form_condition_remove($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];

  $removed_condition = $form_state['clicked_button']['#array_parents'][2];
  unset($block->conditions[$removed_condition]);
  $layout->in_progress['section'] = 'conditions';
  layout_set_layout_tempstore($layout);

  $form_state['ajax_update'] = array('conditions');
}

/**
 * Form validate handler for layout_block_configure_form().
 */
function layout_block_configure_form_validate($form, &$form_state) {
  /* @var Block $block */
  $block = $form_state['block'];

  // Validate the block settings.
  $block->formValidate($form, $form_state);

  // Replace the current style handler with a new one, in the event it changed.
  $style = layout_create_handler('layout_style', $form_state['values']['style']);
  $style->formValidate($form, $form_state);
  $form_state['style'] = $style;
}

/**
 * Form submit handler for layout_block_configure_form().
 */
function layout_block_configure_form_submit($form, &$form_state) {
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var LayoutStyle $style */
  $style = $form_state['style'];
  /* @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];
  $block_plugin = $block->plugin;

  // Save the block settings.
  $block->formSubmit($form, $form_state);

  // Allow the block to change it's type by updating the plugin.
  if ($block_plugin !== $block->plugin) {
    $old_block = clone $block;
    $block = layout_create_handler('block', $old_block->plugin);
    $block->settings = $old_block->settings;
    $block->uuid = $old_block->uuid;
    $block->conditions = $old_block->conditions;
    $block->style = $old_block->style;
    $block->contexts = $old_block->contexts;
    $block->is_new = TRUE;
    $form_state['block'] = $block;
  }

  // Save the style settings.
  $style->formSubmit($form['style']['style_settings'], $form_state);
  $block->style = $style;

  // Generate a UUID for the block.
  if (empty($block->uuid)) {
    $uuid = new Uuid();
    $block->uuid = $uuid->generate();
  }

  // Temporarily store any unfinished blocks (those that do not yet have a
  // position but were just added) in the layout so it can be retrieved. See
  // layout_tempstore_block_load().
  if (!empty($form_state['triggering_element']['#save_in_progress'])) {
    $layout->in_progress['region_name'] = $form_state['values']['region'];
    $layout->in_progress['block'] = $block;
    $layout->in_progress['renderer'] = $renderer;
  }
  // Insert new blocks into the layout.
  else {
    $layout->content[$block->uuid] = $block;
    if ($form_state['values']['region']) {
      $layout->setBlockPosition($block->uuid, $form_state['values']['region']);
    }
    // Clean up the in_progress value now that we're fully done.
    $layout->in_progress = NULL;
  }

  $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name;
  layout_set_layout_tempstore($layout);
}

/**
 * AJAX callback to update the underlying layout form after updating a block.
 */
function layout_block_configure_ajax_update($form, $form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutRendererStandard $renderer */
  $renderer = $form_state['renderer'];
  $added_to_region = $block->is_new ? (isset($layout->in_progress['region_name']) ? $layout->in_progress['region_name'] : $form_state['values']['region']) : FALSE;

  // Display error messages in the form if any.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
  }
  // If a new block, add to the bottom of the region.
  elseif ($added_to_region) {
    $commands[] = ajax_command_close_modal_dialog();
    $commands[] = ajax_command_remove('#layout-editor-block-' . $block->uuid);
    $commands[] = ajax_command_append('#layout-editor-region-' . $added_to_region . ' .layout-editor-region-content', $renderer->renderBlock($block));
  }
  // If editing a block, replace the block with its placeholder.
  else {
    $commands[] = ajax_command_close_modal_dialog();
    $commands[] = ajax_command_replace('#layout-editor-block-' . $block->uuid, $renderer->renderBlock($block));
  }

  // Remove the page title pseudo region if we're adding a Title block or a
  // Combo block.
  if ($block->module == 'system' && $block->delta == 'page_components' && ($block->childDelta == 'title' || $block->childDelta =='title_combo')) {
    $commands[] = ajax_command_replace('#layout-editor-title', '<div id="layout-title-region-empty"></div>');
  }

  // Update the messages area.
  if ($message = layout_locked_message($layout, 'layout')) {
    backdrop_set_message($message, 'warning');
  }
  $commands[] = ajax_command_remove('#messages');
  $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Menu callback; Remove a block from a layout.
 */
function layout_block_remove_page(Layout $layout, Block $block, $renderer_name) {
  if (backdrop_get_token('layout-' . $layout->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  $commands = array();
  $layout->removeBlock($block->uuid);
  layout_set_layout_tempstore($layout);

  if (backdrop_is_ajax()) {
    $commands[] = ajax_command_remove('#layout-editor-block-' . $block->uuid);
    // If neither a Title block nor a Combo block remains in the layout, add
    // the page title pseudo-region.
    if ($block->module == 'system' && $block->delta == 'page_components' && ($block->childDelta == 'title' || $block->childDelta =='title_combo')) {
      $render_title_region = TRUE;
      $all_layout_blocks = $layout->content;
      foreach ($all_layout_blocks as $existing_block) {
        if ($existing_block->module == 'system' && $existing_block->delta == 'page_components' && ($existing_block->childDelta == 'title' || $existing_block->childDelta =='title_combo') && ($existing_block->uuid != $block->uuid)) {
          $render_title_region = FALSE;
          break;
        }
      }
      if ($render_title_region) {
        $renderer = layout_create_renderer($renderer_name, $layout);
        $commands[] = ajax_command_replace('#layout-title-region-empty', $renderer->renderTitleRegion('title'));
      }
    }
    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Block "@title" removed.', array('@title' => $block->getAdminTitle())));
    backdrop_goto('admin/structure/layouts/manage/' . $layout->name);
  }
}

/**
 * Menu callback; Set a block as disabled.
 */
function layout_block_disable_page(Layout $layout, Block $block, $renderer_name) {
  if (backdrop_get_token('layout-' . $layout->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  $uuid = $block->uuid;
  $layout->content[$uuid]->status = $block->status ? BLOCK_DISABLED : BLOCK_ENABLED;
  layout_set_layout_tempstore($layout);

  $renderer = layout_create_renderer($renderer_name, $layout);
  $replace_block = $renderer->renderBlock($block);

  if (backdrop_is_ajax()) {
    if ($message = layout_locked_message($layout, 'layout')) {
      backdrop_set_message($message, 'warning');
      $commands[] = ajax_command_remove('#messages');
      $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
    }

    $commands[] = ajax_command_replace('#layout-editor-block-' . $block->uuid, $replace_block);
    return array(
      '#type' => 'ajax',
      '#commands' => $commands,
    );
  }
  else {
    backdrop_set_message(t('Block "@title" disabled.', array('@title' => $block->getAdminTitle())));
    backdrop_goto('admin/structure/layouts/manage/' . $layout->name);
  }

}
/**
 * Form callback; Displays form for adding new conditions to a layout or block.
 *
 * @param Layout $layout
 *   The layout to which a condition is being added.
 * @param Block|null $block
 *   If applying a condition to a particular block, the block instance.
 * @param LayoutMenuItem|null $menu_item
 *   If applying a condition to an menu item, the menu item object.
 * @param int|null $condition_id
 *   The integer ID of the condition being configured. If adding a new
 *   condition, no value will be passed in. Note that this value may be a
 *   FALSE-like value, such as 0, indicating the first condition is being
 *   configured, which is different than a NULL value indicating a new
 *   condition is being added.
 *
 * @ingroup forms
 */
function layout_condition_add_form($form, &$form_state, Layout $layout = NULL, Block $block = NULL, LayoutMenuItem $menu_item = NULL, $condition_id = NULL) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');
  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  $form_state['menu_item'] = $menu_item;
  $form_state['condition_id'] = $condition_id;

  // Condition position may be a zero-value. Use a clear variable for checking
  // the add vs. configure existing status.
  $is_new_condition = TRUE;
  if (isset($condition_id)) {
    $is_new_condition = FALSE;
  }
  // Specify the default value of the current condition if configuring an
  // existing condition.
  if (!isset($form_state['values']['condition'])) {
    $form_state['values']['condition'] = $condition_id;
  }

  $handler = NULL;
  if (isset($menu_item)) {
    $form['#redirect'] = 'admin/structure/layouts/menu/' . $menu_item->name . '/access';
    if ($is_new_condition) {
      $form['#title'] = t('Add visibility condition');
    }
    else {
      $form['#title'] = t('Configure visibility condition');
    }
    $help = t('Restrict access to all layouts at this path.');
    $handler = isset($menu_item->conditions[$condition_id]) ? $menu_item->conditions[$condition_id] : NULL;
  }
  elseif (isset($block)) {
    $form['#redirect'] = 'admin/structure/layouts/manage/' . $layout->name;
    if ($is_new_condition) {
      $form['#title'] = t('Add visibility condition for "!title" block', array('!title' => $block->getAdminTitle()));
    }
    else {
      $form['#title'] = t('Configure visibility condition for "!title" block', array('!title' => $block->getAdminTitle()));
    }
    $help = t('Limit the situations in which this block will be shown.');
    $handler = isset($block->conditions[$condition_id]) ? $block->conditions[$condition_id] : NULL;
  }
  else {
    $form['#redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/configure';
    if ($is_new_condition) {
      $form['#title'] = t('Add visibility condition for "@title" layout', array('@title' => $layout->title));
    }
    else {
      $form['#title'] = t('Configure visibility condition for "@title" layout', array('@title' => $layout->title));
    }
    $help = t('Limit the situations in which this layout will apply.');
    $handler = isset($layout->conditions[$condition_id]) ? $layout->conditions[$condition_id] : NULL;
  }
  $form['help'] = array(
    '#type' => 'help',
    '#markup' => $help,
  );
  $all_access_info = _layout_get_all_info('layout_access');
  backdrop_sort($all_access_info, array('title'));
  $form['condition'] = array(
    '#type' => 'select',
    '#title' => t('Visibility condition'),
    '#options' => array(),
    '#required' => TRUE,
    '#default_value' => isset($handler->plugin) ? $handler->plugin : NULL,
    '#parents' => array('condition'),
    '#ajax' => array(
      'wrapper' => 'condition_settings',
      'callback' => 'layout_condition_ajax_style',
    ),
  );
  foreach ($all_access_info as $access_info) {
    $required_contexts = !empty($access_info['required contexts']) ? $access_info['required contexts'] : array();
    $layout_has_context = $layout && $layout->hasContexts($required_contexts);
    $menu_item_has_context = $menu_item && $menu_item->hasContexts($required_contexts);
    if (empty($required_contexts) || $layout_has_context || $menu_item_has_context) {
      $form['condition']['#options'][$access_info['name']] = $access_info['title'];
    }
  }

  $form['condition_settings'] = array(
    '#type' => 'container',
    '#id' => 'condition_settings',
    '#parents' => array('condition_settings'),
  );
  $form['condition_settings']['content'] = layout_condition_return_form($form['condition_settings'], $form_state);
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['load_condition_nojs'] = array(
    '#type' => 'submit',
    '#value' => t('Load condition'),
    '#submit' => array(
      'layout_condition_add_load_condition_nojs',
    ),
    '#attributes' => array(
      'class' => array('js-hide'),
    ),
  );
  $form['actions']['add'] = array(
    '#type' => 'submit',
    '#value' => $is_new_condition ? t('Add visibility condition') : t('Save visibility condition'),
    '#submit' => array(
      'layout_condition_add_form_submit',
    ),
    '#validate' => array(
      'layout_condition_add_form_validate',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_save_dialog',
    ),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#limit_validation_errors' => array(),
    '#submit' => array(
      'layout_condition_add_form_cancel',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_condition_cancel_dialog',
    ),
  );
  return $form;
}

/**
 * Submit handler for the cancel button on layout_condition_add_form().
 */
function layout_condition_add_form_cancel($form, &$form_state) {
  if (isset($form_state['block'])) {
    /* @var Block $block */
    $block = $form_state['block'];
    /* @var Layout $layout */
    $layout = $form_state['layout'];
    $renderer_name = isset($layout->in_progress['renderer_name']) ? $layout->in_progress['renderer_name'] : 'editor';
    $renderer = layout_create_renderer($renderer_name, $layout);
    $form_state['redirect'] = $renderer->getUrl('configure-block', $block->uuid);
  }
}

/**
 * AJAX callback to update the condition settings.
 */
function layout_condition_ajax_style($form, $form_state) {
  return $form['condition_settings'];
}

/**
 * Submit handler to select a condition. Hidden when JavaScript is enabled.
 */
function layout_condition_add_load_condition_nojs($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX handler that cancels the condition edit dialog.
 */
function layout_ajax_condition_cancel_dialog($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  // Reload the block configure dialog if editing block conditions.
  if (!empty($form_state['redirect'])) {
    $dialog_ajax = layout_ajax_form_open_dialog($form, $form_state);
    $commands = array_merge($commands, $dialog_ajax['#commands']);
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Helper function to return a partial condition settings form.
 *
 * This can be used to add a condition to either a layout, block, or menu item,
 * based on the values in the $form_state parameter.
 */
function layout_condition_return_form($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutMenuItem $menu_item */
  $menu_item = $form_state['menu_item'];
  $condition_id = $form_state['condition_id'];

  // If no condition has yet been selected, there is no sub-form to display.
  if (!isset($form_state['values']['condition'])) {
    return array();
  }
  else {
    $handler_id = $form_state['values']['condition'];
  }

  if (isset($condition_id)) {
    if ($menu_item) {
      $handler = &$menu_item->conditions[$condition_id];
    }
    elseif ($block) {
      $handler = &$block->conditions[$condition_id];
    }
    else {
      $handler = &$layout->conditions[$condition_id];
    }

    // Update the condition in the same place if the type of condition has
    // changed entirely (note the $handler is assigned by reference above).
    if (!is_numeric($handler_id) && $condition_id != $handler_id) {
      $handler = layout_create_access($handler_id);
    }
  }
  else {
    $handler = layout_create_access($handler_id);
    $handler->is_new = TRUE;
  }

  $form['#title'] = layout_condition_edit_title($layout, $block, $menu_item, $handler_id);
  $form_state['layout'] = $layout;
  $form_state['block'] = $block;
  $form_state['menu_item'] = $menu_item;
  $form_state['handler'] = $handler;
  $handler->form($form, $form_state);

  return $form;
}

/**
 * Validation handler for layout_condition_add_form().
 */
function layout_condition_add_form_validate($form, &$form_state) {
  if (isset($form_state['handler'])) {
    /* @var LayoutAccess $handler */
    $handler = $form_state['handler'];
    $handler->formValidate($form, $form_state);
  }
}

/**
 * Submit handler for layout_condition_add_form().
 *
 * Saves a condition into the layout, block, or menu item.
 */
function layout_condition_add_form_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var Block $block */
  $block = $form_state['block'];
  /* @var LayoutMenuItem $menu_item */
  $menu_item = $form_state['menu_item'];

  if (!isset($form_state['handler'])) {
    return;
  }
  /* @var LayoutAccess $handler */
  $handler = $form_state['handler'];
  $handler->formSubmit($form, $form_state);
  if (!empty($handler->is_new)) {
    $handler->is_new = FALSE;
    if ($menu_item) {
      $menu_item->conditions[] = $handler;
    }
    elseif ($block) {
      $block->conditions[] = $handler;
    }
    else {
      $layout->conditions[] = $handler;
    }
  }
  if ($menu_item) {
    layout_set_layout_tempstore($menu_item, 'menu_item');
  }
  else {
    layout_set_layout_tempstore($layout);
  }

  // Update the portion of the page for menu item access.
  if ($menu_item) {
    $form_state['ajax_rebuild_form'] = 'layout_menu_item_form';
    $form_state['ajax_rebuild_args'] = array($menu_item);
    $form_state['ajax_update'] = array('conditions');
    $form_state['redirect'] = 'admin/structure/layouts/menu/' . $menu_item->name;
  }
  // Blocks return to the dialog/page for editing that block.
  elseif ($block) {
    $renderer_name = isset($layout->in_progress['renderer_name']) ? $layout->in_progress['renderer_name'] : 'editor';
    $renderer = layout_create_renderer($renderer_name, $layout);
    $form_state['redirect'] = $renderer->getUrl('configure-block', $block->uuid);
  }
  // Set the portion of the page to be updated for layouts.
  else {
    $form_state['ajax_rebuild_form'] = 'layout_settings_form';
    $form_state['ajax_rebuild_args'] = array($layout);
    $form_state['ajax_update'] = array('conditions');
    $form_state['redirect'] = 'admin/structure/layouts/manage/' . $layout->name . '/configure';
  }
}

/**
 * Title callback; Provide the page title for configuring a layout condition.
 */
function layout_condition_edit_title(Layout $layout = NULL, Block $block = NULL, LayoutMenuItem $menu_item = NULL, $handler_id) {
  if (isset($menu_item->conditions[$handler_id])) {
    $plugin_name = $menu_item->conditions[$handler_id]->plugin;
  }
  elseif (isset($block->conditions[$handler_id])) {
    $plugin_name = $block->conditions[$handler_id]->plugin;
  }
  elseif (isset($layout->conditions[$handler_id])) {
    $plugin_name = $layout->conditions[$handler_id]->plugin;
  }
  else {
    $plugin_name = $handler_id;
  }
  $access_info = layout_get_access_info($plugin_name);
  if (isset($access_info['title'])) {
    if ($menu_item) {
      return t('Configure condition "@condition" for "@title"', array('@condition' => $access_info['title'], '@title' => $menu_item->path));
    }
    elseif ($block) {
      return t('Configure condition "@condition" for "@title"', array('@condition' => $access_info['title'], '@title' => $block->getAdminTitle()));
    }
    else {
      return t('Configure condition "@condition"', array('@condition' => $access_info['title']));
    }
  }
}

/**
 * Menu callback; Enable or disable a layout configuration.
 */
function layout_toggle_enabled(Layout $layout, $status) {
  if (backdrop_get_token('layout-' . $layout->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  if ($status) {
    $layout->enable();
    backdrop_set_message(t('Layout %title has been enabled.', array('%title' => $layout->title)));
  }
  else {
    $path = $layout->getPath();
    $removal_removes_page = layout_removal_removes_page($layout);
    $layout->disable();
    backdrop_set_message(t('Layout %title has been disabled.', array('%title' => $layout->title)));
    if ($removal_removes_page) {
      layout_warn_if_site_info_path($path, 'disable');
    }
  }
  backdrop_goto('admin/structure/layouts');
}

/**
 * Menu callback; Enable or disable a layout template.
 */
function layout_template_toggle_enabled($template_name, $status) {
  if (backdrop_get_token('layout-template-' . $template_name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }
  $template_info = layout_get_layout_template_info($template_name);
  if (!$template_info) {
    return MENU_NOT_FOUND;
  }

  $config = config('layout.settings');
  $data = $config->get();
  $errors = FALSE;

  if (!$status) {
    if (!in_array($template_name, $data['excluded_templates'])) {
      $data['excluded_templates'][] = $template_name;
    }
    // Check which layout templates are in use.
    foreach (layout_load_all() as $layout) {
      $info = layout_get_layout_template_info($layout->layout_template);
      if (in_array($layout->layout_template, $data['excluded_templates'])) {
        $errors = TRUE;
        break;
      }
    }
    if ($errors) {
      backdrop_set_message(t('The "@layout" layout template is currently in use and may not be disabled.', array('@layout' => $info['title'])), 'error');
    }
    else {
      backdrop_set_message(t('Layout template "@title" disabled.', array('@title' => $template_info['title'])));
    }
  }
  else {
    $data['excluded_templates'] = array_diff($data['excluded_templates'], array($template_name));
    backdrop_set_message(t('Layout template "@title" enabled.', array('@title' => $template_info['title'])));
  }

  if (!$errors) {
    $config->setData($data);
    $config->save();
  }

  backdrop_goto('admin/structure/layouts/settings');
}

/**
 * Reorder the order of layouts at the same path.
 *
 * @ingroup forms
 */
function layout_reorder_form($form, &$form_state) {
  if (!isset($form_state['layouts'])) {
    $layout_list = isset($_GET['layouts']) ? $_GET['layouts'] : array();
    $layouts = array();
    if (is_array($layout_list)) {
      $layouts = layout_load_multiple($layout_list);
    }
    $form_state['layouts'] = $layouts;
  }

  $layouts = $form_state['layouts'];
  $form['#tree'] = TRUE;

  // Sanity check.
  if (empty($layouts)) {
    backdrop_set_message(t('No layouts specified to reorder.'));
    return $form;
  }

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('Layouts at the same path are selected based on the first layout that matches its conditions. Reorder the layouts on this page to affect the layout that will be used first.'),
  );

  $form['layouts'] = array(
    '#theme' => 'layout_reorder_layouts',
  );

  $layout_count = count($layouts);
  foreach ($layouts as $layout) {
    $form['layouts'][$layout->name]['title'] = array(
      '#markup' => check_plain($layout->title),
    );
    $form['layouts'][$layout->name]['weight'] = array(
      '#type' => 'weight',
      '#delta' => $layout_count,
      '#default_value' => $layout->weight,
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save order'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/layouts',
    '#options' => array(
      'attributes' => array('class' => array('form-cancel')),
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_reorder_form().
 */
function layout_reorder_form_submit(&$form, &$form_state) {
  foreach ($form_state['layouts'] as $name => $layout) {
    $layout->weight = (int) $form_state['values']['layouts'][$name]['weight'];
    $layout->save();
  }
  backdrop_set_message(t('Layout order saved.'));
  $form_state['redirect'] = 'admin/structure/layouts';
}

/**
 * Menu callback; Break a lock on a layout or menu item.
 *
 * @param $item
 *   Either a Layout or LayoutMenuItem object, which is currently being edited.
 * @param string $type
 *   The type of Layout item, either "layout" or "menu_item".
 *
 * @see layout_locked_message()
 */
function layout_break_lock_page($item, $type = 'layout') {
  if (backdrop_get_token($type . '-' . $item->name) !== $_GET['token']) {
    return MENU_ACCESS_DENIED;
  }

  layout_clear_layout_tempstore($item->name, $type);
  backdrop_set_message(t('The lock has been cleared and all changes discarded. You may now make changes to this page.'));
  backdrop_goto();
}

/**
 * Prepare a message and link based on if a layout item is updated or locked.
 *
 * @see layout_break_lock_page()
 */
function layout_locked_message($item, $type = 'layout') {
  global $user;

  // The path varies based on the item type.
  $path_type = 'manage';
  $button_title = t('Save layout');
  if ($type === 'menu_item') {
    $path_type = 'menu';
    $button_title = t('Save menu settings');
  }

  $message = '';
  if (!empty($item->locked) && $user->uid !== $item->locked['uid']) {
    $account = user_load($item->locked['uid']);
    $lock_user = theme('username', array('account' => $account));
    $lock_age = format_interval(REQUEST_TIME - $item->locked['updated']);
    $lock_break_url = url('admin/structure/layouts/' . $path_type . '/' . $item->name . '/break-lock', array('query' => array('token' => backdrop_get_token($type . '-' . $item->name)) + backdrop_get_destination()));
    $message = t('Settings on this page are being edited by user !user, and is therefore locked from editing by others. This lock is !age old. Click here to <a href="!break">break this lock</a>.',  array('!user' => $lock_user, '!age' => $lock_age, '!break' => $lock_break_url));
  }
  elseif (!empty($item->locked['updated'])) {
    $message = t('This form has unsaved changes. Click "@button" to make changes permanent or "Cancel" to discard changes.', array('@button' => $button_title));
  }

  return $message;
}

/**
 * Form callback; Copy an existing layout into a new layout.
 *
 * @ingroup forms
 */
function layout_clone_form($form, &$form_state, $layout) {
  $layout_clone = $layout->getClone();

  $settings_form = layout_settings_form($form, $form_state, $layout_clone);
  $form['title'] = $settings_form['title'];
  $form['name'] = $settings_form['name'];

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clone layout'),
  );

  return $form;
}

/**
 * Submit handler for layout_clone_form().
 */
function layout_clone_form_submit($form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  $layout->title = $form_state['values']['title'];
  $layout->name = $form_state['values']['name'];
  if (!empty($layout->menu_item)) {
    $layout->menu_item->menu['title'] = $form_state['values']['name'];
    $layout->menu_item->name = $form_state['values']['name'];
  }
  $layout->save();

  // Redirect to the full settings form.
  $form_state['redirect'] = array(
    'admin/structure/layouts/manage/' . $layout->name . '/configure',
    array('query' => array('destination' => 'admin/structure/layouts/manage/' . $layout->name)),
  );
  backdrop_set_message(t('Cloned layout @title created. You may now configure the cloned layout.', array('@title' => $layout->title)));
}

/**
 * Form callback. Delete or revert a layout.
 *
 * @ingroup forms
 */
function layout_delete_form($form, &$form_state, $layout) {
  $form_state['layout'] = $layout;

  $delete_layout = ($layout->storage === LAYOUT_STORAGE_NORMAL);
  if ($delete_layout) {
    $question = t('Delete layout @title?', array('@title' => $layout->title));

    if (layout_removal_removes_page($layout)) {
      $path = $layout->getPath();
      $site_config = config('system.core');
      if ($path == $site_config->get('site_frontpage')) {
        $text = t('This layout creates a page at %path that is used as the home page of the site.', array('%path' => $path)) . ' ' . t(' If you delete or disable this layout, that page will be removed.') . ' ' . _layout_site_info_page_message('edit-front-page', t('home page'));
      }
      elseif ($path == $site_config->get('site_403')) {
        $text = t('This layout creates a page at %path that is used as the default 403 (access denied) page of the site.', array('%path' => $path)) . ' ' . t(' If you delete or disable this layout, that page will be removed.') . ' ' . _layout_site_info_page_message('edit-error-page', t('403 page'));
      }
      elseif ($path == $site_config->get('site_404')) {
        $url = url('admin/config/system/site-information') . '#edit-error-page';
        $text = t('This layout creates a page at %path that is used as the default 404 (not found) page of the site.', array('%path' => $path)) . ' ' . t(' If you delete or disable this layout, that page will be removed.') . ' ' . _layout_site_info_page_message('edit-error-page', t('404 page'));
      }
      else {
        $text = t('Are you sure you want to delete this layout?') . ' ' . t('The page at %path will be completely removed.') . ' ' . t('This action cannot be undone. You may prefer to disable the layout instead.', array('%path' => $path));
      }
    }
    else {
      // We're deleting the layout, but there will still be a page at its path.
      $text = t('Are you sure you want to delete this layout?') . ' ' . t('This action cannot be undone. You may prefer to disable the layout instead.');
    }
    $button_text = t('Delete layout');
  }
  else {
    $question = t('Revert layout @title?', array('@title' => $layout->title));
    $text = t('Reverting the layout will delete any changes, reverting it to the original default layout provided by the %module module.', array('%module' => $layout->module));
    $button_text = t('Revert layout');
  }

  $form = confirm_form($form, $question, 'admin/structure/layouts', $text, $button_text);

  if ($delete_layout) {
    // Add a disable layout button to the standard confirmation form.
    $cancel = array_pop($form['actions']);
    $form['actions']['disable'] = array(
      '#type' => 'submit',
      '#value' => t('Disable layout'),
    );
    $form['actions']['cancel'] = $cancel;
  }

  return $form;
}

/**
 * Submit handler for layout_delete_form(). Delete, revert, or disable a layout.
 */
function layout_delete_form_submit(&$form, &$form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  $title = $layout->title;
  $path = $layout->getPath();
  $removal_removes_page = layout_removal_removes_page($layout);

  switch ($form_state['clicked_button']['#value']) {
    case t('Delete layout'):
      $layout->delete();
      backdrop_set_message(t('Layout %title has been deleted.', array('%title' => $title)));
      if ($removal_removes_page) {
        layout_warn_if_site_info_path($path, 'delete');
      }
      break;

    case t('Disable layout'):
      $layout->disable();
      backdrop_set_message(t('Layout %title has been disabled.', array('%title' => $title)));
      if ($removal_removes_page) {
        layout_warn_if_site_info_path($path, 'disable');
      }
      break;

    case t('Revert layout'):
      $layout->revert();
      backdrop_set_message(t('Layout %title has been reverted.', array('%title' => $title)));
      break;
  }

  $form_state['redirect'] = 'admin/structure/layouts';
}

/**
* Determines whether the deletion or disabling of this layout would remove the
* page at its path.
*
* @return Boolean.
*   TRUE if the layout creates a page that is the last enabled layout with the
*   same path. FALSE if not.
*/
function layout_removal_removes_page($layout) {
  $path = $layout->getPath();
  $form['layout_path'] = $path;
  $router_item = menu_get_item($path);
  if ($router_item != FALSE) {
    $is_layout_page = ($router_item['page_callback'] == 'layout_page_callback');
    if ($is_layout_page) {
      if (!isset($router_item['map'])) {
        // This needs to exist but isn't always set by menu_get_item().
        $router_item['map'] = array();
      }
      return count(layout_load_multiple_by_router_item($router_item)) == 1;
    }
  }
  return FALSE;
}

/**
 * Issues warnings if for a layout whose removal removes a page, its path
 * matches one of the paths on the site information page.
 *
 * @param string $path
 *   The path of the layout (that was just deleted or disabled).
 * @param string $op
 *   One of 'delete' or 'disable'.
 */
function layout_warn_if_site_info_path($path, $op) {
  $site_config = config('system.core');
  switch ($op) {
    case 'delete':
      if ($path == $site_config->get('site_frontpage')) {
        backdrop_set_message(t('You have deleted the page at %path that is used as the home page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-front-page', t('home page')), 'warning');
      }
      if ($path == $site_config->get('site_403')) {
        backdrop_set_message(t('You have deleted the page at %path that is used as the default 403 (access denied) page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-error-page', t('403 page')), 'warning');
      }
      if ($path == $site_config->get('site_404')) {
        backdrop_set_message(t('You have deleted the page at %path that is used as the default 404 (not found) page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-error-page', t('404 page')), 'warning');
      }
      break;

    case 'disable':
      if ($path == $site_config->get('site_frontpage')) {
        backdrop_set_message(t('You have disabled the page at %path that is used as the home page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-front-page', t('home page')), 'warning');
      }
      if ($path == $site_config->get('site_403')) {
        backdrop_set_message(t('You have disabled the page at %path that is used as the default 403 (access denied) page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-error-page', t('403 page')), 'warning');
      }
      if ($path == $site_config->get('site_404')) {
          backdrop_set_message(t('You have disabled the the page at %path that is used as the default 404 (not found) page of the site.', array('%path' => $path)) . ' ' . _layout_site_info_page_message('edit-error-page', t('404 page')), 'warning');
      }
      break;
  }
}

/**
 * Returns a "you should also change" message and link to the appropriate part
 * of the site information page.
 *
 * @param string $id
 *   The CSS ID of the relevant section of the site information page.
 * @param string $setting_type
 *   The translated name of the relevant setting type on the site information
 *   page.
 *
 * @return string
 */
function _layout_site_info_page_message($id, $setting_type) {
  $url = url('admin/config/system/site-information') . '#' . $id;
  return t('You should also change the @setting_type setting under <a href="!url">site information</a>.', array(
    '@setting_type' => $setting_type,
    '!url' => $url,
  ));
}

/**
 * Form callback; Configure a layout menu item.
 *
 * @ingroup forms
 */
function layout_menu_item_form($form, &$form_state, LayoutMenuItem $menu_item) {
  form_load_include($form_state, 'inc', 'layout', 'layout.admin');

  $form_state['menu_item'] = $menu_item;

  $form['#tree'] = TRUE;
  $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';
  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';

  $messages = array();
  if ($menu_item->locked) {
    $messages['warning'] = array(
      layout_locked_message($menu_item, 'menu_item'),
    );
  }
  $form['messages'] = array(
    '#theme' => 'status_messages',
    '#messages' => $messages,
    '#weight' => -100,
    // Prefix/suffix used to identify in AJAX requests.
    '#prefix' => '<div id="layout-messages">',
    '#suffix' => '</div>',
  );

  $menu_settings = $menu_item->menu;

  $form['menu']['type'] = array(
    '#title' => t('Type'),
    '#type' => 'radios',
    '#options' => array(
      'none' => t('No menu entry'),
      'normal' => t('Normal menu entry'),
      'tab' => t('Menu tab'),
      'default tab' => t('Default menu tab'),
      'action' => t('Local action'),
    ),
    '#default_value' => $menu_settings['type'],
  );

  $form['menu']['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $menu_settings['title'],
    '#description' => t('If set to normal or tab, enter the text to use for the menu item.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="menu[type]"]' => array('value' => 'none'),
      ),
    ),
  );

  // Only display the menu selector if menu module is enabled.
  if (module_exists('menu')) {
    $form['menu']['name'] = array(
      '#title' => t('Menu'),
      '#type' => 'select',
      '#options' => menu_get_menus(),
      '#default_value' => $menu_settings['name'],
      '#description' => t('Insert item into an available menu.'),
      '#states' => array(
        'visible' => array(
          ':input[name="menu[type]"]' => array('value' => 'normal'),
        ),
      ),
    );
  }
  else {
    $form['menu']['name'] = array(
      '#type' => 'value',
      '#value' => $menu_settings['name'],
    );
    $form['menu']['markup'] = array(
      '#value' => t('Menu selection requires the activation of menu module.'),
    );
  }
  $form['menu']['weight'] = array(
    '#title' => t('Weight'),
    '#type' => 'number',
    '#default_value' => isset($menu_settings['weight']) ? $menu_settings['weight'] : 0,
    '#description' => t('The lower the weight the higher/further left it will appear.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="menu[type]"]' => array('value' => 'none'),
      ),
    ),
  );

  $form['menu']['parent'] = array(
    '#type' => 'fieldset',
    '#title' => t('Parent menu item'),
    '#states' => array(
      'visible' => array(
        ':input[name="menu[type]"]' => array('value' => 'default tab'),
      ),
    ),
  );
  $form['menu']['parent']['type'] = array(
    '#title' => t('Parent menu item'),
    '#title_display' => 'none',
    '#type' => 'radios',
    '#options' => array('none' => t('No menu entry'), 'normal' => t('Normal menu item'), 'tab' => t('Menu tab')),
    '#default_value' => $menu_settings['parent']['type'],
    '#description' => t('When providing a menu item as a default tab, the parent menu item of that tab must be specified. Sometimes the parent will already exist, but other times you will need to have one created. The path of a parent item will always be the same path with the last part left off. i.e, if the path to this layout is <em>foo/bar/baz</em>, the parent path would be <em>foo/bar</em>.'),
  );
  $form['menu']['parent']['title'] = array(
    '#title' => t('Parent item title'),
    '#type' => 'textfield',
    '#default_value' => $menu_settings['parent']['title'],
    '#description' => t('If creating a parent menu item, enter the title of the item.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="menu[parent][type]"]' => array('value' => 'none'),
      ),
    ),
  );
  // Only display the menu selector if menu module is enabled.
  if (module_exists('menu')) {
    $form['menu']['parent']['name'] = array(
      '#title' => t('Parent item menu'),
      '#type' => 'select',
      '#options' => menu_get_menus(),
      '#default_value' => $menu_settings['parent']['name'],
      '#description' => t('Insert item into an available menu.'),
      '#states' => array(
        'visible' => array(
          ':input[name="menu[parent][type]"]' => array('value' => 'normal'),
        ),
      ),
    );
  }
  else {
    $form['menu']['parent']['name'] = array(
      '#type' => 'value',
      '#value' => $menu_settings['parent']['name'],
    );
  }
  $form['menu']['parent']['weight'] = array(
    '#title' => t('Item weight'),
    '#type' => 'textfield',
    '#default_value' => $menu_settings['parent']['weight'],
    '#size' => 5,
    '#description' => t('Enter the weight of the menu item or tab. The lower the number, the earlier in the list it will be.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="menu[parent][type]"]' => array('value' => 'none'),
      ),
    ),
  );

  $form['conditions'] = array(
    '#title' => t('Access conditions'),
    '#type' => 'item',
    '#description' => t('Conditions set at the menu level will prevent access to the entire page and all layouts configured in it.'),
    '#id' => 'layout-access',
  );
  $form['conditions']['active'] = array(
    '#type' => 'container',
    '#theme' => 'layout_conditions',
    '#attributes' => array('class' => array('layout-access-list')),
  );
  foreach ($menu_item->conditions as $access_key => $menu_access) {
    $form['conditions']['active'][$access_key] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('layout-condition')),
      '#id' => NULL,
    );
    $form['conditions']['active'][$access_key]['label'] = array(
      '#markup' => $menu_access->summary(),
    );
    $form['conditions']['active'][$access_key]['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-remove')),
      '#submit' => array(
        'layout_menu_item_form_condition_remove',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_update',
      ),
      '#name' => 'conditions[' . $access_key . '][remove]',
    );
    $form['conditions']['active'][$access_key]['configure'] = array(
      '#type' => 'submit',
      '#value' => t('Configure'),
      '#attributes' => array('class' => array('layout-link-button', 'layout-access-configure')),
      '#submit' => array(
        'layout_menu_item_form_condition_edit',
      ),
      '#ajax' => array(
        'callback' => 'layout_ajax_form_open_dialog',
      ),
      '#name' => 'conditions[' . $access_key . '][configure]',
    );
  }
  $form['conditions']['links'] = array(
    '#theme' => 'layout_action_links',
  );
  $form['conditions']['links']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add visibility condition'),
    '#attributes' => array('class' => array('layout-link-button', 'layout-access-add')),
    '#submit' => array(
      'layout_menu_item_form_condition_add'
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_open_dialog',
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save menu settings'),
  );

  if (isset($menu_item->locked)) {
    $form['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#limit_validation_errors' => array(array('actions', 'reset')),
      '#validate' => array(),
      '#submit' => array(
        'layout_menu_item_form_reset',
      ),
    );
  }

  return $form;
}

/**
 * Form callback; Configure a layout menu item.
 */
function layout_menu_item_form_submit($form, &$form_state) {
  /* @var LayoutMenuItem $menu_item */
  $menu_item = $form_state['menu_item'];

  // Update the menu settings. Conditions are added already through tempstore.
  $menu_item->menu = $form_state['values']['menu'];
  $menu_item->save();

  layout_clear_layout_tempstore($menu_item->name, 'menu_item');
  backdrop_set_message(t('Menu settings saved for @path.', array('@path' => $menu_item->path)));
  $form_state['redirect'] = 'admin/structure/layouts';
}

/**
 * AJAX handler for layout_settings_form() the returns updated contexts.
 */
function layout_menu_item_form_path_ajax($form, &$form_state) {
  return $form['context_wrapper'];
}

/**
 * Submit handler for layout_settings_form() that lets the user add a condition.
 */
function layout_menu_item_form_condition_add($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/layouts/menu/' . $form_state['menu_item']->name . '/condition/add';
}

/**
 * Submit handler for layout_settings_form() that removes a condition.
 */
function layout_menu_item_form_condition_remove($form, &$form_state) {
  $removed_condition = $form_state['clicked_button']['#array_parents'][2];
  unset($form_state['menu_item']->conditions[$removed_condition]);
  layout_set_layout_tempstore($form_state['menu_item'], 'menu_item');

  $form_state['ajax_update'] = array('conditions');
}

/**
 * Submit handler for layout_settings_form() that edits an condition.
 */
function layout_menu_item_form_condition_edit($form, &$form_state) {
  $edit_condition = $form_state['clicked_button']['#array_parents'][2];
  $form_state['redirect'] = 'admin/structure/layouts/menu/' . $form_state['menu_item']->name . '/condition/edit/' . $edit_condition;
}

/**
 * Submit handler for layout_menu_item_form() that resets in-progress changes.
 */
function layout_menu_item_form_reset($form, &$form_state) {
  $menu_item = $form_state['menu_item'];
  layout_clear_layout_tempstore($menu_item->name, 'menu_item');
  backdrop_set_message(t('Menu item changes discarded.'));
  $form_state['redirect'] = 'admin/structure/layouts';
}

/**
 * Partial form to generate a select list for selecting context.
 *
 * @param LayoutContext[] $available_contexts
 *   An array of contexts from a layout, as retrieved by Layout::getContexts().
 * @param array $info
 *   An array of block or condition info, as provided by
 *   layout_get_access_info() or layout_get_block_info(). The list of required
 *   contexts is pulled from here (if any).
 * @param array $current_context_settings
 *   An array of current context settings, used for populating the defaults.
 *
 * @return array
 *   The partial form element.
 */
function layout_contexts_form_element(array $available_contexts, array $current_context_settings, array $info) {
  $element = array(
    '#tree' => TRUE,
    '#weight' => -20,
  );
  if (isset($info['required contexts'])) {
    foreach ($info['required contexts'] as $context_key => $context_type) {
      if (isset($info['required contexts labels'][$context_key])) {
        $label = $info['required contexts labels'][$context_key];
      }
      else {
        $context_info = layout_get_context_info($context_type);
        $label = $context_info['title'];
      }
      $options = array();
      foreach ($available_contexts as $layout_context_key => $context) {
        if ($context->plugin === $context_type) {
          $options[$layout_context_key] = $context->label();
        }
      }
      if (count($options) > 1) {
        $element[$context_key] = array(
          '#type' => 'select',
          '#title' => check_plain($label),
          '#default_value' => isset($current_context_settings[$context_key]) ? $current_context_settings[$context_key] : NULL,
          '#options' => $options,
        );
      }
    }
  }
  return $element;
}

/**
 * AJAX handler that opens a form in a dialog.
 *
 * This AJAX handler is used both on the layout settings form and the layout
 * content form.
 *
 * @see layout_settings_form()
 * @see layout_block_configure_form()
 */
function layout_ajax_form_open_dialog($form, &$form_state) {
  // Ensure that no errors have occurred before opening the dialog.
  if (form_get_errors()) {
    $commands[] = ajax_command_remove('#messages');
    $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
  }
  else {
    // Get the dialog path as set in the redirect.
    $dialog_path = $form_state['redirect'];
    $page_callback_result = NULL;
    if ($router_item = menu_get_item($dialog_path)) {
      if ($router_item['access']) {
        if ($router_item['include_file']) {
          require_once BACKDROP_ROOT . '/' . $router_item['include_file'];
        }
        $page_callback_result = call_user_func_array($router_item['page_callback'], $router_item['page_arguments']);
      }
    }
    $html = is_string($page_callback_result) ? $page_callback_result : backdrop_render($page_callback_result);
    if (isset($page_callback_result['#title'])) {
      $title = $page_callback_result['#title'];
    }
    else {
      $title = $router_item['title'];
    }
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * AJAX handler that saves a form in a dialog and updates the underlying form.
 *
 * This is meant for generic use. It depends on a few variables in $form_state:
 *   - $form_state['ajax_rebuild_form']: The function name that builds the
 *     underlying form that will be updated.
 *   - $form_state['ajax_update']: An array of keys representing the portion
 *     of the form that should be updated.
 *   - $form_state['redirect']: If specified, the redirect location will be
 *     opened as a dialog, acting as a tool to open a sequence of forms in
 *     dialogs.
 *
 * @see layout_ajax_form_update()
 */
function layout_ajax_form_save_dialog($form, $form_state) {
  /* @var Layout $layout */
  $layout = $form_state['layout'];
  /* @var LayoutMenuItem $menu_item */
  $menu_item = isset($form_state['menu_item']) ? $form_state['menu_item']  : '';

  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  // If errors occurred in the dialog, show messages and re-display.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
  }
  else {
    // If there is a part of the page that needs to be updated, update it.
    if (!empty($form_state['ajax_update'])) {
      $update_ajax = layout_ajax_form_update($form, $form_state);
      $commands = array_merge($commands, $update_ajax['#commands']);
    }

    // If there is a second action that needs to be executed, open the next page.
    elseif (!empty($form_state['redirect'])) {
      $dialog_ajax = layout_ajax_form_open_dialog($form, $form_state);
      $commands = array_merge($commands, $dialog_ajax['#commands']);
    }

    // Clear out previous messages.
    if ($message = layout_locked_message($layout, 'layout')) {
      backdrop_set_message($message, 'warning');
    }
    elseif ($message = layout_locked_message($menu_item, 'menu_item')) {
      backdrop_set_message($message, 'warning');
    }
    $commands[] = ajax_command_remove('#messages');
    $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

function layout_ajax_form_save_title_dialog($form, $form_state) {
  $layout = $form_state['layout'];

  $commands = array();
  $commands[] = ajax_command_close_modal_dialog();

  // If errors occurred in the dialog, show messages and re-display.
  if (form_get_errors()) {
    $html = '';
    $html .= theme('status_messages');
    $html .= backdrop_render($form);
    $title = isset($form['#title']) ? $form['#title'] : backdrop_get_title();
    $commands[] = ajax_command_open_modal_dialog($title, $html, array('dialogClass' => 'layout-dialog'));
  }
  else {
    // Rebuild the title area description.
    $description = '<span class="description">' . layout_get_title_description($layout) . '</span>';
    $commands[] = ajax_command_replace("#layout-editor-title .description", $description);

    // Clear out previous messages.
    if ($message = layout_locked_message($layout, 'layout')) {
      backdrop_set_message($message, 'warning');
    }
    elseif ($message = layout_locked_message($menu_item, 'menu_item')) {
      backdrop_set_message($message, 'warning');
    }
    $commands[] = ajax_command_remove('#messages');
    $commands[] = ajax_command_html('#layout-messages', theme('status_messages'));
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * AJAX handler that updates a portion of the form.
 *
 * This is meant for generic use. It depends on a few variables in $form_state:
 *   - $form_state['ajax_rebuild_form']: The function name that builds the
 *     form that will be updated.
 *   - $form_state['ajax_update']: An array of keys representing the portion
 *     of the form that should be updated.
 */
function layout_ajax_form_update($form, $form_state) {
  if (!isset($form_state['ajax_update'])) {
    return NULL;
  }

  // If updating an entirely different form, build the form and pull out the new
  // pieces.
  if (isset($form_state['ajax_rebuild_form'])) {
    $form_state += array(
      'ajax_rebuild_args' => array(),
    );
    $args = array_merge(array($form_state['ajax_rebuild_form']), $form_state['ajax_rebuild_args']);
    $replace_form = call_user_func_array('backdrop_get_form', $args);
  }
  // Or if just rebuilding the current form, rebuild it directly.
  else {
    $replace_form = backdrop_rebuild_form($form['#form_id'], $form_state, $form);
  }

  // Remove any ID incrementation that could happen on the form ID.
  $form_id = preg_replace('/--\d$/', '', $replace_form['#id']);
  $build_id = backdrop_render($replace_form['form_build_id']);

  // Narrow down the form to just the part that will be replaced.
  $replace_portion = $form_state['ajax_update'];
  foreach ($replace_portion as $parent) {
    if (isset($replace_form[$parent])) {
      $replace_form = $replace_form[$parent];
    }
    else {
      $replace_form = FALSE;
      break;
    }
  }

  $commands = array();
  if ($replace_form) {
    // Update the portion of the form.
    $html = backdrop_render($replace_form);
    $commands[] = ajax_command_replace('#' . $replace_form['#id'], $html);
    // And update the build ID so these new values are included when saving.
    $commands[] = ajax_command_replace('#' . $form_id . ' input[name="form_build_id"]', $build_id);
  }
  else {
    // @todo: Provide an error message if the target is not found.
  }

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Administrative form for layout settings.
 */
function layout_settings_page($form, &$form_state) {
  $header = array(
    'layout_template_info' => array('data' => t('Template')),
    'type' => array('data' => t('Type'), 'sort' => 'desc', 'class' => array(RESPONSIVE_PRIORITY_LOW)),
    'operations' => t('Operations'),
  );

  backdrop_set_title(t('Layout templates'));
  $excluded = config_get('layout.settings', 'excluded_templates');

  $rows = array();
  $options = array();
  $default = array();
  $form_state['hidden_templates'] = array();
  $all_template_info = layout_get_layout_template_info(NULL, TRUE);

  foreach ($all_template_info as $template_name => $template_info) {
    // Build the default values list.
    $is_default = FALSE;
    if (!in_array($template_name, $excluded)) {
      $is_default = $default[$template_name] = TRUE;
    }

    // Build the list of possible options, either enabled or not hidden.
    if ($is_default || !$template_info['hidden']) {
      $rows[$template_name] = array(
        'layout_template_info' => theme('layout_template_info', array('template_info' => $template_info)),
        'type' => !empty($template_info['flexible']) ? t('Flexible') : t('Standard'),
      );
      $rows[$template_name]['operations']['data'] = array(
        '#type' => 'dropbutton',
        '#links' => _layout_settings_get_group_operations($template_info, $is_default),
       );
    }
    // For disabled and hidden templates, save the fact they are disabled.
    elseif ($template_info['hidden']) {
      $form_state['hidden_templates'][] = $template_name;
    }
  }

  $form['#attached']['css'][] = backdrop_get_path('module', 'layout') . '/css/layout.admin.css';
  $form['templates'] = array(
    '#type' => 'tableselect',
    '#js_select' => FALSE,
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('There are no layout templates.'),
    '#default_value' => $default,
    '#attributes' => array('class' => array('layout-list')),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validation for layout settings page.
 *
 * Ensures that active layouts are not disabled while in use.
 */
function layout_settings_page_validate(&$form, &$form_state) {
  $options = array_keys($form_state['values']['templates']);
  $included = array_values($form_state['values']['templates']);
  $excluded = array_unique(array_diff($options, $included));
  $form_state['excluded_templates'] = array_values(array_merge($excluded, $form_state['hidden_templates']));

  // Ensure all layouts are not disabled.
  if (empty($form_state['values']['templates'])) {
    form_set_error('templates', t('At least one layout template must be enabled.'));
  }

  // Check which layout templates are in use.
  foreach (layout_load_all() as $layout) {
    $info = layout_get_layout_template_info($layout->layout_template);
    if (in_array($layout->layout_template, $excluded)) {
      form_set_error('templates][' . $layout->layout_template, t('The "@layout" layout template is currently in use and may not be disabled.', array('@layout' => $info['title'])));
    }
  }
}

/**
 * Submit handler for layout advanced settings page.
 */
function layout_settings_page_submit($form, &$form_state) {
  config_set('layout.settings', 'excluded_templates', $form_state['excluded_templates']);
  backdrop_set_message(t('Your configuration has been saved.'));
}

/**
 * Given a path, return a list of operations for a group of layouts.
 */
function _layout_settings_get_group_operations($template_info, $is_default) {
  $links = array();

  if (!empty($template_info['flexible'])) {
    if (user_access('administer flexible templates')) {
      $links['configure'] = array(
        'title' => t('Configure regions'),
        'href' => 'admin/structure/layouts/settings/flexible-template/' . $template_info['name'] . '/configure',
      );
      $links['edit'] = array(
        'title' => t('Configure name'),
        'href' => 'admin/structure/layouts/settings/flexible-template/' . $template_info['name'] . '/edit',
      );
    }
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => t('Flexible layout templates'),
          'name' => 'layout.flexible.' . $template_info['name'],
        ),
      );
    }
    if (user_access('administer flexible templates')) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/structure/layouts/settings/flexible-template/' . $template_info['name'] . '/delete',
      );
    }
  }
  if ($is_default) {
    $links['disable'] = array(
      'title' => t('Disable'),
      'href' => 'admin/structure/layouts/settings/toggle/' . $template_info['name'] . '/disable',
      'query' => array('token' => backdrop_get_token('layout-template-' . $template_info['name'])),
    );
  }
  else {
    $links['enable'] = array(
      'title' => t('Enable'),
      'href' => 'admin/structure/layouts/settings/toggle/' . $template_info['name'] . '/enable',
      'query' => array('token' => backdrop_get_token('layout-template-' . $template_info['name'])),
    );
  }

  return $links;
}

/**
 * Menu callback; Show the region styles form.
 */
function layout_configure_region_page($form, &$form_state, Layout $layout, $renderer_name, $region_name, $block_key = NULL) {
  $form_state['layout'] = $layout;
  $form_state['region_name'] = $region_name;
  $layout_info = layout_get_layout_template_info($layout->layout_template);
  if (!array_key_exists($region_name, $layout_info['regions'])) {
    return MENU_NOT_FOUND;
  }

  $form['#tree'] = TRUE;

  $options = array(
    '' => t('No inner wrapper'),
    'div' => 'DIV',
    'nav' => 'NAV',
    'aside' => 'ASIDE',
    'section' => 'SECTION',
  );
  $form['element'] = array(
    '#title' => t('HTML element for inner wrapper'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($layout->settings[$region_name]['element']) ? $layout->settings[$region_name]['element'] : '',
  );
  $form['classes'] = array(
    '#title' => t('CSS classes for inner wrapper'),
    '#type' => 'textfield',
    '#default_value' => isset($layout->settings[$region_name]['classes']) ? $layout->settings[$region_name]['classes'] : '',
    '#description' => t('Separate class names with spaces. Example: <code>wrapper clearfix</code>'),
    '#states' => array(
      'disabled' => array(
        ':input[name="element"]' => array('value' => ''),
      ),
    ),
  );

  $form['title_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Region title'),
    '#default_value' => isset($layout->settings[$region_name]['title_enabled']) ? $layout->settings[$region_name]['title_enabled'] : 0,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($layout->settings[$region_name]['title']) ? $layout->settings[$region_name]['title'] : '',
    '#title' => t('Region title'),
    '#states' => array(
      'visible' => array(
        'form.layout-configure-region-page :input[name="title_enabled"]' => array('checked' => TRUE),
      ),
    ),
    '#maxlength' => 255,
  );
  $form['title_tag'] = array(
    '#title' => t('Region title heading level'),
    '#type' => 'select',
    '#options' => array(
      'h1' => 'H1',
      'h2' => 'H2',
      'h3' => 'H3',
      'h4' => 'H4',
      'h5' => 'H5',
      'h6' => 'H6',
      'div' => 'DIV',
      'span' => 'SPAN',
      'p' => 'P',
    ),
    '#default_value' => isset($layout->settings[$region_name]['title_tag']) ? $layout->settings[$region_name]['title_tag'] : 'h2',
    '#states' => array(
      'visible' => array(
        'form.layout-configure-region-page :input[name="title_enabled"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['title_classes'] = array(
    '#title' => t('Region title heading CSS classes'),
    '#type' => 'textfield',
    '#default_value' => isset($layout->settings[$region_name]['title_classes']) ? $layout->settings[$region_name]['title_classes'] : '',
    '#states' => array(
      'visible' => array(
        'form.layout-configure-region-page :input[name="title_enabled"]' => array('checked' => TRUE),
      ),
    ),
    '#description' => t('Additional classes to be added to the region title.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array(
      'layout_configure_region_page_submit',
    ),
    '#ajax' => array(
      'callback' => 'layout_ajax_form_save_dialog',
    ),
  );

  return $form;
}

/**
 * Submit handler for layout_configure_region_page().
 */
function layout_configure_region_page_submit($form, &$form_state) {
  $region_name = $form_state['region_name'];
  $form_state['layout']->settings[$region_name]['element'] = $form_state['values']['element'];
  $form_state['layout']->settings[$region_name]['classes'] = $form_state['values']['classes'];
  $form_state['layout']->settings[$region_name]['title_enabled'] = $form_state['values']['title_enabled'];
  $form_state['layout']->settings[$region_name]['title'] = $form_state['values']['title'];
  $form_state['layout']->settings[$region_name]['title_tag'] = $form_state['values']['title_tag'];
  $form_state['layout']->settings[$region_name]['title_classes'] = $form_state['values']['title_classes'];
  layout_set_layout_tempstore($form_state['layout']);
}

/**
 * Menu callback; Lists all blocks and their modules.
 */
function layout_block_list() {
  $rows = array();

  $module_blocks = layout_get_block_info();
  $usage = layout_get_block_usage();

  // Get all blocks.
  foreach ($module_blocks as $module => $blocks) {
    if (!empty($blocks)) {
      foreach ($blocks as $delta => $block) {
        $contexts = !empty($block['required contexts']) ? implode(",", $block['required contexts']) : '';
        $usages = array();
        if (isset($usage[$module][$delta])) {
          foreach ($usage[$module][$delta] as $layout => $layout_use) {
            $instances = array();
            foreach ($layout_use as $position => $block_use) {
              foreach ($block_use as $instance) {
                $instances[$position][] = $instance;
              }
            }
            $positions = implode(', ', array_map('ucfirst', array_keys($instances)));
            $usages[] = l($instance->layout_title . ' (' . $positions . ')', 'admin/structure/layouts/manage/' . $layout);
          }
        }
        $usage_list = $usages ? theme('item_list', array('items' => $usages)) : t('Not in use');
        $block_name = theme('label_machine_name__layout_block', array(
          'label' => $block['info'],
          'machine_name' => $delta,
        ));
        $rows[] = array('name' => $block_name, 'usage' => $usage_list, 'module' => $module, 'contexts' => $contexts);
      }
    }
  }

  $header = array(
    array('data' => 'Name', 'field' => 'name', 'sort' => 'ASC'),
    array('data' => 'Usage', 'field' => 'usage', 'sort' => 'ASC'),
    array('data' => 'Module', 'field' => 'module', 'sort' => 'ASC', 'class' => array(RESPONSIVE_PRIORITY_LOW)),
    array('data' => 'Required contexts', 'field' => 'contexts', 'class' => array(RESPONSIVE_PRIORITY_LOW)),
  );

  $order = tablesort_get_order($header);
  $sort = tablesort_get_sort($header);

  backdrop_sort($rows, array($order['sql'] => SORT_STRING));
  $rows = ($sort == 'asc') ? $rows : array_reverse($rows);

  $page['description_container'] = array(
    '#type' => 'container',
  );

  $page['description_container']['description'] = array(
    '#type' => 'help',
    '#markup' => t("Provides a summary of all available blocks. The 'Usage' column shows which layout and region to which blocks are currently assigned in the format 'Layout name (region)'."),
  );

  $page['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are no blocks.'),
  );

  return $page;
}

/**
 * Lists information about blocks in use in layouts.
 *
 * @param string $module
 *   The module providing the required blocks.
 * @param string $delta
 *   The block delta. If $delta is provided, $module is also required since
 *   block deltas are not required to be unique.
 *
 * @return array
 *   An array of block usage instances keyed by block module, delta, layout
 *   machine_name, and position. Each instance is a block object with an
 *   additional key for the layout name.
 */
function layout_get_block_usage($module = NULL, $delta = NULL) {
  $blocks = &backdrop_static(__FUNCTION__, array());
  if (empty($blocks)) {
    $layout_info = layout_load_all();
    foreach ($layout_info as $layout) {
      foreach ($layout->positions as $position => $uuids) {
        foreach ($uuids as $uuid) {
          $block = $layout->content[$uuid];
          $block->layout_title = $layout->title;
          $blocks[$block->module][$block->delta][$layout->name][$position][] = $block;
        }
      }
    }
  }

  if ($module && $delta) {
    return $blocks[$module][$delta];
  }
  elseif ($module) {
    return $blocks[$module];
  }
  return $blocks;
}
