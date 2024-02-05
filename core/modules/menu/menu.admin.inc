<?php
/**
 * @file
 * Admin page callbacks for the Menu module.
 */

/**
 * Menu callback which shows an overview page of all the custom menus and their
 * descriptions.
 */
function menu_overview_page() {
  $menus = menu_load_all();
  backdrop_sort($menus, array('title' => SORT_STRING));
  $header = array(t('Title'), t('Description'), t('Operations'));
  $rows = array();
  foreach ($menus as $menu) {
    $row = array();
    $row[] = theme('label_machine_name__menu', array(
      'label' => $menu['title'],
      'machine_name' => $menu['menu_name'],
    ));
    $row[] = filter_xss_admin($menu['description']);
    $links = array();
    $links['list'] = array(
      'title' => t('Manage links'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'],
    );
    $links['add'] = array(
      'title' => t('Add link'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'] . '/add',
    );
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'] . '/configure',
    );
    $system_menus = menu_list_system_menus();
    if (!isset($system_menus[$menu['menu_name']])) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/structure/menu/manage/' . $menu['menu_name'] . '/delete',
      );
    }
    if (module_exists('config') && user_access('synchronize configuration')) {
      $links['export'] = array(
        'title' => t('Export'),
        'href' => 'admin/config/development/configuration/single/export',
        'query' => array(
          'group' => 'Menus',
          'name' => 'menu.menu.' . $menu['menu_name'],
        ),
      );
    }
    $row[] = array(
      'data' => array(
        '#type' => 'operations',
        '#links' => $links,
      ),
    );
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form for editing an entire menu tree at once.
 *
 * Shows for one menu the menu links accessible to the current user and
 * relevant operations.
 */
function menu_overview_form($form, &$form_state, $menu) {
  global $menu_admin;
  $form['#attached']['css'] = array(backdrop_get_path('module', 'menu') . '/css/menu.css');

  $default_language = language_default();
  $filter_langcode = isset($_GET['langcode']) ? $_GET['langcode'] : $default_language->langcode;

  $form['langcode'] = array(
    '#title' => t('Show menu in language'),
    '#type' => 'select',
    '#options' => language_list(TRUE, TRUE),
    '#empty_value' => LANGUAGE_NONE,
    '#empty_option' => t('All languages'),
    '#default_value' => $filter_langcode,
    '#access' => module_exists('language'),
    '#weight' => -10,
    '#description' => t('Show menu links for a given language. Menu links with the "All" language will be shown in every language.'),
  );
  $form['langcode_filter'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#attributes' => array('class' => array('button-primary')),
    '#submit' => array('menu_overview_form_language_filter_submit'),
  );

  $sql = "
    SELECT m.load_functions, m.to_arg_functions, m.access_callback, m.access_arguments, m.page_callback, m.page_arguments, m.delivery_callback, m.title, m.title_callback, m.title_arguments, m.type, m.description, ml.*
    FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path
    WHERE ml.menu_name = :menu
    ORDER BY p1 ASC, p2 ASC, p3 ASC, p4 ASC, p5 ASC, p6 ASC, p7 ASC, p8 ASC, p9 ASC";
  $result = db_query($sql, array(':menu' => $menu['menu_name']), array('fetch' => PDO::FETCH_ASSOC));
  $links = array();
  foreach ($result as $item) {
    $links[] = $item;
  }
  $link_count = db_query("SELECT COUNT(*) AS counter FROM {menu_links} WHERE menu_name = :menu AND link_path NOT LIKE :link_path", array(':menu' => $menu['menu_name'], ':link_path' => '%\%%'))->fetchObject();
  $counter = intval($link_count->counter / 2 ) + 1;

  $tree = menu_tree_data($links);
  $node_links = array();
  menu_tree_collect_node_links($tree, $node_links);
  // We indicate that a menu administrator is running the menu access check.
  $menu_admin = TRUE;
  menu_tree_check_access($tree, $node_links);
  $menu_admin = FALSE;

  $tree_langcode = $filter_langcode === LANGUAGE_NONE ? NULL : $filter_langcode;
  $delta = _menu_get_menu_weight_delta($menu['menu_name'], $counter);
  $form = array_merge($form, _menu_overview_tree_form($tree, $delta, $tree_langcode));
  $form['#menu'] = $menu;

  // Only show the form submit button if there are menu items.
  if (count($links)) {
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    );
  }

  return $form;
}

/**
 * Recursive helper function for menu_overview_form().
 *
 * @param $tree
 *   The menu_tree retrieved by menu_tree_data.
 * @param $delta
 *   The number of items to use in the menu weight selector. Defaults to 50.
 * @param $langcode
 *   A language code by which to filter the list of menu items.
 */
function _menu_overview_tree_form($tree, $delta = 50, $langcode = NULL) {
  $form = &backdrop_static(__FUNCTION__, array('#tree' => TRUE));
  $language_options = language_list(TRUE, TRUE);
  $language_options[LANGUAGE_NONE] = t('All');

  foreach ($tree as $data) {
    $item = $data['link'];

    // Skip all items not in the requested language.
    if ($langcode && $item['langcode'] !== $langcode && $item['langcode'] !== LANGUAGE_NONE) {
      continue;
    }

    // Don't show callbacks; these have $item['hidden'] < 0.
    if ($item && $item['hidden'] >= 0) {
      $mlid = 'mlid:' . $item['mlid'];
      $form[$mlid]['#item'] = $item;
      $form[$mlid]['#attributes'] = $item['hidden'] ? array('class' => array('menu-disabled')) : array('class' => array('menu-enabled'));
      $form[$mlid]['title']['#markup'] = l($item['title'], $item['href'], $item['localized_options']);
      if ($item['hidden']) {
        $form[$mlid]['title']['#markup'] .= ' (' . t('disabled') . ')';
      }
      elseif ($item['link_path'] == 'user' && $item['module'] == 'system') {
        $form[$mlid]['title']['#markup'] .= ' (' . t('logged in users only') . ')';
      }

      $langcode_label = isset($language_options[$item['langcode']]) ? $language_options[$item['langcode']] : $item['langcode'];
      $form[$mlid]['langcode'] = array(
        '#type' => 'markup',
        '#markup' => check_plain($langcode_label),
        '#access' => module_exists('language'),
      );
      $form[$mlid]['hidden'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable @title menu link', array('@title' => $item['title'])),
        '#title_display' => 'invisible',
        '#default_value' => !$item['hidden'],
      );
      $form[$mlid]['weight'] = array(
        '#type' => 'weight',
        '#delta' => $delta,
        '#default_value' => $item['weight'],
        '#title_display' => 'invisible',
        '#title' => t('Weight for @title', array('@title' => $item['title'])),
      );
      $form[$mlid]['mlid'] = array(
        '#type' => 'hidden',
        '#value' => $item['mlid'],
      );
      $form[$mlid]['plid'] = array(
        '#type' => 'hidden',
        '#default_value' => $item['plid'],
      );
      // Build a list of operations.
      $links = array();
      $links['edit'] = array(
        'title' => t('Edit'),
        'href' => 'admin/structure/menu/item/' . $item['mlid'] . '/edit',
      );
      // Only items created by the menu module can be deleted.
      if ($item['module'] == 'menu' || $item['updated'] == 1) {
        $links['delete'] = array(
          'title' => t('Delete'),
          'href' => 'admin/structure/menu/item/' . $item['mlid'] . '/delete',
        );
      }
      // Set the reset column.
      elseif ($item['module'] == 'system' && $item['customized']) {
        $links['reset'] = array(
          'title' => t('Reset'),
          'href' => 'admin/structure/menu/item/' . $item['mlid'] . '/reset',
        );
      }
      $form[$mlid]['operations'] = array(
        '#type' => 'operations',
        '#links' => $links,
      );
    }

    if ($data['below']) {
      _menu_overview_tree_form($data['below'], $delta, $langcode);
    }
  }
  return $form;
}

/**
 * Submit handler for the menu overview form.
 *
 * This function takes great care in saving parent items first, then items
 * underneath them. Saving items in the incorrect order can break the menu tree.
 *
 * @see menu_overview_form()
 */
function menu_overview_form_submit($form, &$form_state) {
  // When dealing with saving menu items, the order in which these items are
  // saved is critical. If a changed child item is saved before its parent,
  // the child item could be saved with an invalid path past its immediate
  // parent. To prevent this, save items in the form in the same order they
  // are sent by $_POST, ensuring parents are saved first, then their children.
  // See http://drupal.org/node/181126#comment-632270
  $order = array_flip(array_keys($form_state['input'])); // Get the $_POST order.
  $form = array_intersect_key(array_merge($order, $form), $form); // Update our original form with the new order.

  $updated_items = array();
  $fields = array('weight', 'plid');
  foreach (element_children($form) as $mlid) {
    if (isset($form[$mlid]['#item'])) {
      $element = $form[$mlid];
      // Update any fields that have changed in this menu item.
      foreach ($fields as $field) {
        if ($element[$field]['#value'] != $element[$field]['#default_value']) {
          $element['#item'][$field] = $element[$field]['#value'];
          $updated_items[$mlid] = $element['#item'];
        }
      }
      // Hidden is a special case, the value needs to be reversed.
      if ($element['hidden']['#value'] != $element['hidden']['#default_value']) {
        // Convert to integer rather than boolean due to PDO cast to string.
        $element['#item']['hidden'] = $element['hidden']['#value'] ? 0 : 1;
        $updated_items[$mlid] = $element['#item'];
      }
    }
  }

  // Save all our changed items to the database.
  foreach ($updated_items as $item) {
    $item['customized'] = 1;
    menu_link_save($item);
  }
  backdrop_set_message(t('Your configuration has been saved.'));
}

/**
 * Submit handler for menu_overview_form().
 *
 * Filters the displayed form to a particular language.
 */
function menu_overview_form_language_filter_submit($form, &$form_state) {
  $query['langcode'] = $form_state['values']['langcode'];
  if (isset($_GET['destination'])) {
    $query['destination'] = $_GET['destination'];
  }
  backdrop_goto($_GET['q'], array('query' => $query));
}

/**
 * Menu callback; Build the menu link editing form.
 */
function menu_edit_item($form, &$form_state, $type, $item, $menu) {
  if ($type == 'add' || empty($item)) {
    // This is an add form, initialize the menu link.
    $item = array(
      'menu_name' => $menu['menu_name'],
      'mlid' => 0,
      'plid' => 0,
      'link_title' => '',
      'link_path' => '',
      'langcode' => isset($_GET['langcode']) ? $_GET['langcode'] : LANGUAGE_NONE,
      'weight' => 0,
      'options' => array(),
      'module' => 'menu',
      'expanded' => 0,
      'hidden' => 0,
      'has_children' => 0,
    );
  }
  else {
    // Get the human-readable menu title from the given menu name
    $titles = menu_get_menus();
    $current_title = $titles[$item['menu_name']];

    // Get the current breadcrumb and add a link to that menu's overview page.
    $breadcrumb = menu_get_active_breadcrumb();
    $breadcrumb[] = l($current_title, 'admin/structure/menu/manage/' . $item['menu_name']);
    backdrop_set_breadcrumb($breadcrumb);
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['link_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Menu link title'),
    '#maxlength' => 255,
    '#default_value' => $item['link_title'],
    '#description' => t('The text to be used for this link in the menu.'),
    '#required' => TRUE,
  );
  foreach (array('link_path', 'mlid', 'module', 'has_children', 'options') as $key) {
    $form[$key] = array('#type' => 'value', '#value' => $item[$key]);
  }
  // Any item created or edited via this interface is considered "customized".
  $form['customized'] = array('#type' => 'value', '#value' => 1);
  $form['original_item'] = array('#type' => 'value', '#value' => $item);

  $path = $item['link_path'];
  if (isset($item['options']['query'])) {
    $path .= '?' . backdrop_http_build_query($item['options']['query']);
  }
  if (isset($item['options']['fragment'])) {
    $path .= '#' . $item['options']['fragment'];
  }
  $path_alias = backdrop_get_path_alias($path);
  if ($item['module'] == 'menu') {
    $form['link_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path'),
      '#maxlength' => 255,
      '#default_value' => (!empty($path)) ? $path_alias : $path,
      '#description' => t('The path for this menu link. This can be an internal site path such as %add-node or an external URL such as %external. Enter %front to link to the home page.', array('%front' => '<front>', '%add-node' => 'node/add', '%external' => 'http://example.com')),
      '#required' => TRUE,
      '#autocomplete_path' => 'path-autocomplete',
    );
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#access' => $item['mlid'],
      '#submit' => array('menu_item_delete_submit'),
      '#weight' => 10,
    );
  }
  else {
    $form['_path'] = array(
      '#type' => 'item',
      '#title' => t('Path'),
      '#description' => l($item['link_title'], $item['href'], $item['options']),
    );
  }

  if (module_exists('language')) {
    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => $item['langcode'],
      '#options' => language_list(TRUE, TRUE),
      '#empty_value' => LANGUAGE_NONE,
      '#empty_option' => t('- All (always shown) -'),
      '#description' => t('Set a language for this menu link. The menu link will only be visible in that language.'),
      '#ajax' => array('callback' => 'menu_edit_item_callback'),
    );
  }
  else {
    // Maintain langcode settings if Language module is not present.
    $form['langcode'] = array(
      '#type' => 'value',
      '#value' => $item['langcode'],
    );
  }

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => isset($item['options']['attributes']['title']) ? $item['options']['attributes']['title'] : '',
    '#rows' => 1,
    '#description' => t('Shown when hovering over the menu link.'),
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => !$item['hidden'],
    '#description' => t('Menu links that are not enabled will not be listed in any menu.'),
  );
  $form['expanded'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show as expanded'),
    '#default_value' => $item['expanded'],
    '#description' => t('When displayed in a "hierarchical tree" menu style, children of expanded menu links will always be visible.'),
  );

  $menu_name = $item['menu_name'];
  $menus = menu_get_menus();
  $form['parent_menu'] = array(
    '#type' => 'select',
    '#title' => t('Menu'),
    '#default_value' => $menu_name,
    '#options' => $menus,
    '#description' => t('Select the menu that this menu link should belong to.'),
    '#attributes' => array('class' => array('menu-menu-select')),
    '#ajax' => array('callback' => 'menu_edit_item_callback'),
    '#wrapper_attributes' => array('id' => 'menu-parent-menu-wrapper'),
  );

  if (isset($form_state['values']['parent_menu'])) {
    $menu_name = $form_state['values']['parent_menu'];
  }
  // Generate a list of possible parents (not including this link or
  // descendants) from the selected menu. Note that whenever the item language
  // changes, the available parents also have to get updated.
  if (isset($form_state['values']['langcode'])) {
    $item['langcode'] = $form_state['values']['langcode'];
  }
  $options = menu_parent_options(array($menu_name => $menus[$menu_name]), $item);
  $default = $item['menu_name'] . ':' . $item['plid'];
  if (!isset($options[$default])) {
    $default = key($options);
  }
  // @TODO in Backdrop 2.0, replace #prefix/#suffix with #wrapper_attributes.
  // Kept here for backward compatibility with existing contrib, themes.
  $form['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent link'),
    '#default_value' => $default,
    '#options' => $options,
    '#description' => t('The maximum depth for a link and all its children is fixed at !maxdepth. Some menu links may not be available as parents if selecting them would exceed this limit.', array('!maxdepth' => MENU_MAX_DEPTH)),
    '#attributes' => array('class' => array('menu-title-select')),
    '#prefix' => '<div id="menu-parent-select-wrapper">',
    '#suffix' => '</div>',
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#delta' => _menu_get_menu_weight_delta($menu_name),
    '#default_value' => $item['weight'],
    '#description' => t('Optional. In the menu, the heavier links will sink and the lighter links will be positioned nearer the top.'),
    '#attributes' => array('class' => array('menu-weight-select')),
    '#wrapper_attributes' => array('id' => 'menu-parent-weight-wrapper'),
  );

  return $form;
}

/**
 * Callback function to return updated list of possible parent links and
 * weights.
 */
function menu_edit_item_callback($form, $form_state) {
  $commands = array();
  $commands[] = ajax_command_replace("#menu-parent-menu-wrapper", backdrop_render($form['parent_menu']));
  $commands[] = ajax_command_replace("#menu-parent-select-wrapper", backdrop_render($form['parent']));
  $commands[] = ajax_command_replace("#menu-parent-weight-wrapper", backdrop_render($form['weight']));
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Validate form values for a menu link being added or edited.
 */
function menu_edit_item_validate($form, &$form_state) {
  $item = &$form_state['values'];
  $normal_path = backdrop_get_normal_path($item['link_path']);
  $form_state['original_link_path'] = $item['link_path'];
  if ($item['link_path'] != $normal_path) {
    $item['link_path'] = $normal_path;
  }

  if (!url_is_external($item['link_path'])) {
    $parsed_link = parse_url($item['link_path']);
    if (isset($parsed_link['query'])) {
      $item['options']['query'] = backdrop_get_query_array($parsed_link['query']);
    }
    else {
      // Use unset() rather than setting to empty string to avoid redundant
      // serialized data being stored.
      unset($item['options']['query']);
    }
    if (isset($parsed_link['fragment'])) {
      $item['options']['fragment'] = $parsed_link['fragment'];
    }
    else {
      unset($item['options']['fragment']);
    }
    if (isset($parsed_link['path']) && $item['link_path'] != $parsed_link['path']) {
      $item['link_path'] = $parsed_link['path'];
    }
  }
  if (!trim($item['link_path']) || !backdrop_valid_path($item['link_path'], TRUE)) {
    form_set_error('link_path', t("The path '@link_path' is either invalid or you do not have access to it.", array('@link_path' => $item['link_path'])));
  }
}

/**
 * Submit function for the delete button on the menu item editing form.
 */
function menu_item_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/menu/item/' . $form_state['values']['mlid'] . '/delete';
}

/**
 * Process menu and menu item add/edit form submissions.
 */
function menu_edit_item_submit($form, &$form_state) {
  $item = &$form_state['values'];

  if ($item['link_path'] != $form_state['original_link_path']) {
    backdrop_set_message(t('The menu system stores system paths only, but will use the URL alias for display. %link_path has been stored as %normal_path', array(
      '%link_path' => $form_state['original_link_path'],
      '%normal_path' => $item['link_path'],
    )));
  }

  // The value of "hidden" is the opposite of the value supplied by the
  // "enabled" checkbox.
  $item['hidden'] = (int) !$item['enabled'];
  unset($item['enabled']);

  $item['options']['attributes']['title'] = $item['description'];
  list($item['menu_name'], $item['plid']) = explode(':', $item['parent']);
  if (!menu_link_save($item)) {
    backdrop_set_message(t('There was an error saving the menu link.'), 'error');
  }
  else {
    backdrop_set_message(t('Your configuration has been saved.'));
  }

  $redirect = array('path' => 'admin/structure/menu/manage/' . $item['menu_name']);
  if ($item['langcode'] !== LANGUAGE_NONE) {
    $redirect['options']['query'] = array('langcode' => $item['langcode']);
  }
  $form_state['redirect'] = $redirect;
}

/**
 * Menu callback; Build the form that handles the adding/editing of a custom menu.
 */
function menu_edit_menu($form, &$form_state, $type, $menu = array()) {
  $system_menus = menu_list_system_menus();
  if (empty($menu['menu_name'])) {
    backdrop_set_title(t('Add menu'));
  }

  $menu += array(
    'menu_name' => '',
    'old_name' => !empty($menu['menu_name']) ? $menu['menu_name'] : '',
    'title' => '',
    'description' => '',
  );
  // Allow menu_edit_menu_submit() and other form submit handlers to determine
  // whether the menu already exists.
  $form['#insert'] = empty($menu['old_name']);
  $form['old_name'] = array(
    '#type' => 'value',
    '#value' => $menu['old_name'],
  );

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $menu['title'],
    '#required' => TRUE,
  );

  $form['menu_name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Menu name'),
    '#default_value' => $menu['menu_name'],
    '#maxlength' => MENU_MAX_MENU_NAME_LENGTH_UI,
    '#description' => t('A unique name to construct the URL for the menu. It must only contain lowercase letters, numbers and hyphens.'),
    '#field_prefix' => empty($menu['old_name']) ? 'menu-' : '',
    '#machine_name' => array(
      'exists' => 'menu_edit_menu_name_exists',
      'source' => array('title'),
      'replace_pattern' => '[^a-z0-9-]+',
      'replace' => '-',
    ),
    // A menu's machine name cannot be changed.
    '#disabled' => !empty($menu['old_name']) || isset($system_menus[$menu['menu_name']]),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $menu['description'],
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  // Only custom menus may be deleted.
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#access' => $type == 'configure' && !isset($system_menus[$menu['menu_name']]),
    '#submit' => array('menu_custom_delete_submit'),
  );

  return $form;
}

/**
 * Submit function for the 'Delete' button on the menu editing form.
 */
function menu_custom_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $form_state['values']['menu_name'] . '/delete';
}

/**
 * Menu callback; check access and get a confirm form for deletion of a custom menu.
 */
function menu_delete_menu_page($menu) {
  // System-defined menus may not be deleted.
  $system_menus = menu_list_system_menus();
  if (isset($system_menus[$menu['menu_name']])) {
    return MENU_ACCESS_DENIED;
  }
  return backdrop_get_form('menu_delete_menu_confirm', $menu);
}

/**
 * Build a confirm form for deletion of a custom menu.
 */
function menu_delete_menu_confirm($form, &$form_state, $menu) {
  $form['#menu'] = $menu;
  $caption = '';
  $num_links = db_query("SELECT COUNT(*) FROM {menu_links} WHERE menu_name = :menu", array(':menu' => $menu['menu_name']))->fetchField();
  if ($num_links) {
    $caption .= '<p>' . format_plural($num_links, '<strong>Warning:</strong> There is currently 1 menu link in %title. It will be deleted (system-defined items will be reset).', '<strong>Warning:</strong> There are currently @count menu links in %title. They will be deleted (system-defined links will be reset).', array('%title' => $menu['title'])) . '</p>';
  }
  $caption .= '<p>' . t('This action cannot be undone.') . '</p>';
  return confirm_form($form, t('Are you sure you want to delete the custom menu %title?', array('%title' => $menu['title'])), 'admin/structure/menu/manage/' . $menu['menu_name'], $caption, t('Delete'));
}

/**
 * Delete a custom menu and all links in it.
 */
function menu_delete_menu_confirm_submit($form, &$form_state) {
  $menu = $form['#menu'];
  $form_state['redirect'] = 'admin/structure/menu';

  // System-defined menus may not be deleted - only menus defined by this module.
  $system_menus = menu_list_system_menus();
  if (isset($system_menus[$menu['menu_name']])) {
    return;
  }

  // Reset all the menu links defined by the system via hook_menu().
  $result = db_query("SELECT * FROM {menu_links} ml INNER JOIN {menu_router} m ON ml.router_path = m.path WHERE ml.menu_name = :menu AND ml.module = 'system' ORDER BY m.number_parts ASC", array(':menu' => $menu['menu_name']), array('fetch' => PDO::FETCH_ASSOC));
  foreach ($result as $link) {
    menu_reset_item($link);
  }

  // Delete all links to the overview page for this menu.
  $result = db_query("SELECT mlid FROM {menu_links} ml WHERE ml.link_path = :link", array(':link' => 'admin/structure/menu/manage/' . $menu['menu_name']), array('fetch' => PDO::FETCH_ASSOC));
  foreach ($result as $link) {
    menu_link_delete($link['mlid']);
  }

  // Delete the custom menu and all its menu links.
  menu_delete($menu);

  $t_args = array('%title' => $menu['title']);
  backdrop_set_message(t('The custom menu %title has been deleted.', $t_args));
  watchdog('menu', 'Deleted custom menu %title and all its menu links.', $t_args, WATCHDOG_NOTICE);
}

/**
 * Returns whether a menu name already exists.
 *
 * @see menu_edit_menu()
 * @see form_validate_machine_name()
 */
function menu_edit_menu_name_exists($value) {
  // 'menu-' is added to the menu name to avoid name-space conflicts.
  $value = 'menu-' . $value;
  $custom_exists = (bool) menu_load($value);
  $link_exists = db_query_range("SELECT 1 FROM {menu_links} WHERE menu_name = :menu", 0, 1, array(':menu' => $value))->fetchField();

  return $custom_exists || $link_exists;
}

/**
 * Submit function for adding or editing a custom menu.
 */
function menu_edit_menu_submit($form, &$form_state) {
  $menu = $form_state['values'];
  $path = 'admin/structure/menu/manage/';
  if ($form['#insert']) {
    // Add 'menu-' to the menu name to help avoid name-space conflicts.
    $menu['menu_name'] = 'menu-' . $menu['menu_name'];
    $link['link_title'] = $menu['title'];
    $link['link_path'] = $path . $menu['menu_name'];
    $link['router_path'] = $path . '%';
    $link['module'] = 'menu';
    $link['plid'] = db_query("SELECT mlid FROM {menu_links} WHERE link_path = :link AND module = :module", array(
      ':link' => 'admin/structure/menu',
      ':module' => 'system'
    ))
    ->fetchField();

    menu_link_save($link);
    menu_save($menu);
  }
  else {
    menu_save($menu);
    $result = db_query("SELECT mlid FROM {menu_links} WHERE link_path = :path", array(':path' => $path . $menu['menu_name']), array('fetch' => PDO::FETCH_ASSOC));
    foreach ($result as $m) {
      $link = menu_link_load($m['mlid']);
      $link['link_title'] = $menu['title'];
      menu_link_save($link);
    }
  }
  backdrop_set_message(t('Your configuration has been saved.'));
  $form_state['redirect'] = 'admin/structure/menu';
}

/**
 * Menu callback; Check access and present a confirm form for deleting a menu link.
 */
function menu_item_delete_page($item) {
  // Links defined via hook_menu may not be deleted. Updated items are an
  // exception, as they can be broken.
  if ($item['module'] == 'system' && !$item['updated']) {
    return MENU_ACCESS_DENIED;
  }
  return backdrop_get_form('menu_item_delete_form', $item);
}

/**
 * Build a confirm form for deletion of a single menu link.
 */
function menu_item_delete_form($form, &$form_state, $item) {
  $form['#item'] = $item;
  return confirm_form($form, t('Are you sure you want to delete the custom menu link %item?', array('%item' => $item['link_title'])), 'admin/structure/menu/manage/' . $item['menu_name']);
}

/**
 * Process menu delete form submissions.
 */
function menu_item_delete_form_submit($form, &$form_state) {
  $item = $form['#item'];
  menu_link_delete($item['mlid']);
  $t_args = array('%title' => $item['link_title']);
  backdrop_set_message(t('The menu link %title has been deleted.', $t_args));
  watchdog('menu', 'Deleted menu link %title.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $item['menu_name'];
}

/**
 * Menu callback; reset a single modified menu link.
 */
function menu_reset_item_confirm($form, &$form_state, $item) {
  $form['item'] = array('#type' => 'value', '#value' => $item);
  return confirm_form($form, t('Are you sure you want to reset the link %item to its default values?', array('%item' => $item['link_title'])), 'admin/structure/menu/manage/' . $item['menu_name'], t('Any customizations will be lost. This action cannot be undone.'), t('Reset'));
}

/**
 * Process menu reset item form submissions.
 */
function menu_reset_item_confirm_submit($form, &$form_state) {
  $item = $form_state['values']['item'];
  $new_item = menu_reset_item($item);
  backdrop_set_message(t('The menu link was reset to its default settings.'));
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $new_item['menu_name'];
}

/**
 * Menu callback; global settings form for menus.
 */
function menu_settings_form($form, &$form_state) {
  $form['#config'] = 'menu.settings';
  $config = config('menu.settings');

  $form['menu_breakpoint'] = array(
    '#type' => 'radios',
    '#title' => t('Breakpoint'),
    '#description' => t("The 'min-width' for the menu breakpoint. Screens smaller than this may display a menu toggle button, depending on the menu-specific settings."),
    '#options' => array(
      'default' => t('Default (48em)'),
      'custom' => t('Custom'),
    ),
    '#default_value' => $config->get('menu_breakpoint'),
  );
  $form['menu_breakpoint_custom'] = array(
    '#type' => 'textfield',
    '#description' => t("A valid CSS width value (e.g.: '768px', '34em')."),
    '#default_value' => $config->get('menu_breakpoint_custom'),
    '#size' => 8,
    '#field_prefix' => 'min-width:',
    '#states' => array(
      'visible' => array(
        ':input[name="menu_breakpoint"]' => array('value' => 'custom'),
      ),
    ),
  );
  $form['#submit'][] = 'menu_settings_form_submit';

  return system_settings_form($form);
}

/**
 * Validate form values for the global settings form.
 */
function menu_settings_form_validate($form, &$form_state) {
  // Do some basic validation of the menu breakpoint setting.
  if ($form_state['values']['menu_breakpoint'] == 'custom') {
    // This list of CSS units has been taken from:
    // https://developer.mozilla.org/en-US/docs/Web/CSS/length
    if (!preg_match('/^[0-9.]+(px|em|ch|ex|rem|vh|vw|vmin|vmax|cm|mm|in|pc|pt)$/i', $form_state['values']['menu_breakpoint_custom'])) {
      form_set_error('menu_breakpoint_custom', t("'@breakpoint' does not appear to be a valid CSS width value.", array(
        '@breakpoint' => $form_state['values']['menu_breakpoint_custom'],
      )));
    }
  }
}

/**
 * Custom submission handler for the global settings form.
 */
function menu_settings_form_submit($form, &$form_state) {
  // Flush page cache to allow new breakpoint to take effect immediately.
  cache_flush('page');
}
