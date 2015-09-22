<?php

/**
 * @file
 * Administrative page callbacks for menu module.
 */

/**
 * Menu callback which shows an overview page of all the custom menus and their descriptions.
 */
function menu_overview_page() {
  $menus = menu_load_all();
  backdrop_sort($menus, array('title' => SORT_STRING));

  // Enable language column if language module is enabled.
  $multilanguage = module_exists('language');

  if ($multilanguage) {
    $header = array(t('Title'), t('Language'), t('Operations'));
  } else {
    $header = array(t('Title'), t('Operations'));
  }
  
  $rows = array();

  foreach ($menus as $menu) {

    // Only translate the menu if multilingual is enabled. If the menu has a fixed language,
    // we translate it into that language. Otherwise we take the current interface language.
    if ($menu['multilingual_mode'] > MENU_MULTILINGUAL_DISABLED) {
      $options = array();
      if ($menu['langcode'] != LANGUAGE_NONE) {
        $options['langcode'] = $menu['langcode'];
      }
      $menu['title'] = t($menu['title'], array(), $options);
      $menu['description'] = t($menu['description'], array(), $options);
    }

    $row = array();
    $row[] = theme('menu_admin_overview', array('title' => $menu['title'], 'name' => $menu['menu_name'], 'description' => $menu['description']));

    if ($multilanguage) {
      $language = language_load($menu['langcode']);
      if ($language) {
        $row[] = $language->name;
      } else {
        $row[] = '';
      }
    }

    $links = array();
    $links['list'] = array(
      'title' => t('List links'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'],
    );
    $links['edit'] = array(
      'title' => t('Edit menu'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'] . '/edit',
    );
    $links['add'] = array(
      'title' => t('Add link'),
      'href' => 'admin/structure/menu/manage/' . $menu['menu_name'] . '/add',
    );
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
 * Returns HTML for a menu title and description for the menu overview page.
 *
 * @param $variables
 *   An associative array containing:
 *   - title: The menu's title.
 *   - description: The menu's description.
 *
 * @ingroup themeable
 */
function theme_menu_admin_overview($variables) {
  $output = check_plain($variables['title']);
  $output .= '<div class="description">' . filter_xss_admin($variables['description']) . '</div>';

  return $output;
}

/**
 * Form for editing an entire menu tree at once.
 *
 * Shows for one menu the menu links accessible to the current user and
 * relevant operations.
 */
function menu_overview_form($form, &$form_state, $menu) {
  global $menu_admin;
  global $language_interface;

  // Enable language if language module is enabled.
  $multilanguage = module_exists('language');

  $form['#attached']['css'] = array(backdrop_get_path('module', 'menu') . '/menu.css');

  $select = db_select('menu_links', 'ml', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('ml')
    ->condition('ml.menu_name', $menu['menu_name']);

  $select->leftJoin('menu_router', 'm', 'm.path = ml.router_path');
  $select->fields('m', array(
    'load_functions',
    'to_arg_functions',
    'access_callback',
    'access_arguments',
    'page_callback',
    'page_arguments',
    'delivery_callback',
    'title',
    'title_callback',
    'title_arguments',
    'type',
    'description',
  ));
  for ($i = 1; $i <= MENU_MAX_DEPTH; $i++) {
    $select->orderBy('p' . $i, 'ASC');
  }

  if ($multilanguage && $menu['multilingual_mode'] != MENU_MULTILINGUAL_DISABLED) {

    if ($menu['langcode'] != LANGUAGE_NONE) {
      $langcode = $menu['langcode'];

      $language = language_load($langcode);

      $form['langcode'] = array(
        '#markup' => t('This menu is set to a fixed language (:language)', array(':language' => $language->name))
      );

      $form['show_all'] = array(
        '#type' => 'checkbox',
        '#title' => t('Check to see links from all languages.'),
        '#default_value' => isset($form_state['values']['show_all']) ? $form_state['values']['show_all'] : 0,
        '#ajax' => array(
          'wrapper' => 'menu_overview_form_wrapper'
        )
      );
    } else {
      $langcode = (isset($form_state['values']['langcode'])?$form_state['values']['langcode']:LANGUAGE_NONE);

      // Show a select box;
      $languages = language_list(TRUE);
      $language_options = array();
      foreach ($languages as $lcode => $language) {
        $language_options[$lcode] = $language->name;
      }

      $form['langcode'] = array(
        '#type' => 'select',
        '#title' => t('Manage links for language'),
        '#description' => t('This menu is multilingual, select a language to manage the menu links for a specific localisation. To set this menu to a fixed language !edit the menu', array('!edit' => l('edit', 'admin/structure/menu/manage/' . $menu['menu_name'] . '/edit'))),
        '#default_value' => $langcode,
        '#options' => $language_options,
        '#empty_value' => LANGUAGE_NONE,
        '#ajax' => array(
          'wrapper' => 'menu_overview_form_wrapper'
        )
      );
    }

    if ($langcode!=LANGUAGE_NONE && (!isset($form_state['values']['show_all']) || !$form_state['values']['show_all'])) {
      $language_condition = db_or();
      $language_condition->condition('ml.langcode', $langcode);
      $language_condition->condition('ml.langcode', LANGUAGE_NONE);

      $select->condition($language_condition);
    }
  }

  $links = $select->execute()->fetchAll();

  $tree = menu_tree_data($links);
  $node_links = array();
  menu_tree_collect_node_links($tree, $node_links);
  // We indicate that a menu administrator is running the menu access check.
  $menu_admin = TRUE;
  menu_tree_check_access($tree, $node_links);
  $menu_admin = FALSE;

  $form = array_merge($form, _menu_overview_tree_form($tree, $menu));
  $form['#menu'] = $menu;

  if (count($links)) {
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    );
  } else {
    $form['#empty_text'] = t('There are no menu links yet. <a href="@link">Add link</a>.', array('@link' => url('admin/structure/menu/manage/' . $form['#menu']['menu_name'] . '/add')));
  }
  return $form;
}

/**
 * Recursive helper function for menu_overview_form().
 *
 * @param $tree
 *   The menu_tree retrieved by menu_tree_data.
 */
function _menu_overview_tree_form($tree, $menu)
{
  $form = & backdrop_static(__FUNCTION__, array('#tree' => TRUE));
  foreach ($tree as $data) {
    $title = '';
    $item = $data['link'];
    // Don't show callbacks; these have $item['hidden'] < 0.
    if ($item && $item['hidden'] >= 0) {
      $mlid = 'mlid:' . $item['mlid'];
      $form[$mlid]['#item'] = $item;
      $form[$mlid]['#attributes'] = $item['hidden'] ? array('class' => array('menu-disabled')) : array('class' => array('menu-enabled'));
      $form[$mlid]['title']['#markup'] = l($item['title'], $item['href'], $item['localized_options']);
      if ($item['hidden']) {
        $form[$mlid]['title']['#markup'] .= ' (' . t('disabled') . ')';
      } elseif ($item['link_path'] == 'user' && $item['module'] == 'system') {
        $form[$mlid]['title']['#markup'] .= ' (' . t('logged in users only') . ')';
      }

      $language = language_load($item['langcode']);
      $form[$mlid]['langcode'] = array(
        '#markup' => isset($language->name) ? $language->name : ''
      );

      if ($menu['langcode'] != LANGUAGE_NONE && $item['langcode'] != LANGUAGE_NONE && $item['langcode'] != $menu['langcode']) {
        $form[$mlid]['title']['#markup'] .= ' (' . t('hidden') . ')';
      }

      $form[$mlid]['hidden'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable @title menu link', array('@title' => $item['title'])),
        '#title_display' => 'invisible',
        '#default_value' => !$item['hidden'],
      );
      $form[$mlid]['weight'] = array(
        '#type' => 'weight',
        '#delta' => 50,
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
      } // Set the reset column.
      elseif ($item['module'] == 'system' && $item['customized']) {
        $links['reset'] = array(
          'title' => t('Reset'),
          'href' => 'admin/structure/menu/item/' . $item['mlid'] . '/reset',
        );
      }

      if ($menu['multilingual_mode'] == MENU_MULTILINGUAL_ENABLED_WITH_TRANSLATION) {
        $links['translate'] = array(
          'title' => t('Translate'),
          'href' => 'admin/structure/menu/item/' . $item['mlid'] . '/translate',
        );
      }

      $form[$mlid]['operations'] = array(
        '#type' => 'operations',
        '#links' => $links,
      );
    }

    if ($data['below']) {
      _menu_overview_tree_form($data['below'], $menu);
    }
  }

  $form['#prefix'] = '<div id="menu_overview_form_wrapper">';
  $form['#suffix'] = '</div>';

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
function menu_overview_form_submit($form, &$form_state)
{
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
 * Returns HTML for the menu overview form into a table.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_menu_overview_form($variables)
{
  $form = $variables['form'];

  backdrop_add_tabledrag('menu-overview', 'match', 'parent', 'menu-plid', 'menu-plid', 'menu-mlid', TRUE, MENU_MAX_DEPTH - 1);
  backdrop_add_tabledrag('menu-overview', 'order', 'sibling', 'menu-weight');

  // Enable language if language module is enabled.
  $multilanguage = module_exists('language');

  if ($multilanguage) {
    $header = array(
      t('Menu link'),
      t('Language'),
      array('data' => t('Enabled'), 'class' => array('checkbox')),
      t('Weight'),
      t('Operations'),
    );
  } else {
    $header = array(
      t('Menu link'),
      array('data' => t('Enabled'), 'class' => array('checkbox')),
      t('Weight'),
      t('Operations'),
    );
  }

  $rows = array();
  foreach (element_children($form) as $mlid) {
    if (isset($form[$mlid]['hidden'])) {
      $element = & $form[$mlid];

      // Add special classes to be used for tabledrag.js.
      $element['plid']['#attributes']['class'] = array('menu-plid');
      $element['mlid']['#attributes']['class'] = array('menu-mlid');
      $element['weight']['#attributes']['class'] = array('menu-weight');

      // Change the parent field to a hidden. This allows any value but hides the field.
      $element['plid']['#type'] = 'hidden';

      $row = array();
      $row[] = theme('indentation', array('size' => $element['#item']['depth'] - 1)) . backdrop_render($element['title']);
      if ($multilanguage) {
        $row[] = backdrop_render($element['langcode']);
      }
      $row[] = array('data' => backdrop_render($element['hidden']), 'class' => array('checkbox', 'menu-enabled'));
      $row[] = backdrop_render($element['weight']) . backdrop_render($element['plid']) . backdrop_render($element['mlid']);
      $row[] = backdrop_render($element['operations']);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'][] = 'draggable';
      $rows[] = $row;
    }
  }
  $output = '';
  if (empty($rows)) {
    $rows[] = array(array('data' => $form['#empty_text'], 'colspan' => count($header)));
  }
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'menu-overview')));
  $output .= backdrop_render_children($form);
  return $output;
}

/**
 * Menu callback; Build the menu link editing form.
 */
function menu_edit_item($form, &$form_state, $type, $item, $menu)
{
  if ($type == 'add' || empty($item)) {

    $langcode = $menu['langcode'];

    // This is an add form, initialize the menu link.
    $item = array(
      'link_title' => '',
      'mlid' => 0,
      'plid' => 0,
      'menu_name' => $menu['menu_name'],
      'weight' => 0,
      'link_path' => '',
      'options' => array(),
      'module' => 'menu',
      'expanded' => 0,
      'hidden' => 0,
      'has_children' => 0,
      'langcode' => $langcode,
      'tsid' => 0
    );
  } else {
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
  if ($item['module'] == 'menu') {
    $form['link_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path'),
      '#maxlength' => 255,
      '#default_value' => $path,
      '#description' => t('The path for this menu link. This can be an internal Drupal path such as %add-node or an external URL such as %drupal. Enter %front to link to the front page.', array('%front' => '<front>', '%add-node' => 'node/add', '%drupal' => 'http://drupal.org')),
      '#required' => TRUE,
    );
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#access' => $item['mlid'],
      '#submit' => array('menu_item_delete_submit'),
      '#weight' => 10,
    );
  } else {
    $form['_path'] = array(
      '#type' => 'item',
      '#title' => t('Path'),
      '#description' => l($item['link_title'], $item['href'], $item['options']),
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
    '#description' => t('If selected and this menu link has children, the menu will always appear expanded.'),
  );

  // Generate a list of possible parents (not including this link or descendants).
  $options = menu_parent_options(menu_get_menus(), $item);
  $default = $item['menu_name'] . ':' . $item['plid'];
  if (!isset($options[$default])) {
    $default = 'main-menu:0';
  }
  $form['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent link'),
    '#default_value' => $default,
    '#options' => $options,
    '#description' => t('The maximum depth for a link and all its children is fixed at !maxdepth. Some menu links may not be available as parents if selecting them would exceed this limit.', array('!maxdepth' => MENU_MAX_DEPTH)),
    '#attributes' => array('class' => array('menu-title-select')),
    '#ajax' => array(
      'wrapper' => 'langcode-wrapper',
      'callback' => 'menu_edit_item_langcode_ajax_callback'
    )
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#delta' => 50,
    '#default_value' => $item['weight'],
    '#description' => t('Optional. In the menu, the heavier links will sink and the lighter links will be positioned nearer the top.'),
  );

  // Enable language if language module is enabled.
  $multilanguage = module_exists('language');

  if ($multilanguage) {
    if (isset($form_state['values']['parent'])) {
      $menu_name = substr($form_state['values']['parent'], 0, strpos($form_state['values']['parent'], ':'));
    }
    else {
      $menu_name = $item['menu_name'];
    }
    $menu = menu_load($menu_name);

    $languages = language_list(TRUE);
    $language_options = array();
    foreach ($languages as $lcode => $language) {
      // When the menu has a fixed language, only allow the current items language
      // and the menu language.
      if ($menu['langcode']==LANGUAGE_NONE || $lcode==$menu['langcode'] || $lcode==$item['langcode']) {
        $language_options[$lcode] = $language->name;
      }
    }

    if ($menu['langcode']!=LANGUAGE_NONE) {
      if($type == 'add') {
        $description = t('The parent menu is fixed to :language.', array(':language' => $languages[$menu['langcode']]->name));
      }
      else {
        $description = t('The parent menu is fixed to :language. You can either switch to this language or keep the existing language (:existing)', array(
          ':language' => $languages[$menu['langcode']]->name,
          ':existing' => isset($languages[$item['langcode']])?$languages[$item['langcode']]->name:t('None')));
      }
    }
    else {
      $description = t('Set a language for this menu link. The menu link will only be visible in this language.');
    }

    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#description' => $description,
      '#default_value' => $item['langcode'],
      '#options' => $language_options,
      '#disabled' => ($type == 'add' && $menu['langcode']!=LANGUAGE_NONE),
      '#empty_value' => LANGUAGE_NONE,
      '#prefix' => '<div id="langcode-wrapper">',
      '#suffix' => '</div>'
    );
  }

  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

/**
 * Callback element needs only select the portion of the form to be updated.
 * Since #ajax['callback'] return can be HTML or a renderable array (or an
 * array of commands), we can just return a piece of the form.
 */
function menu_edit_item_langcode_ajax_callback($form, $form_state) {
  return $form['langcode'];
}

/**
 * Validate form values for a menu link being added or edited.
 */
function menu_edit_item_validate($form, &$form_state)
{
  $item = & $form_state['values'];
  $normal_path = backdrop_get_normal_path($item['link_path']);
  if ($item['link_path'] != $normal_path) {
    backdrop_set_message(t('The menu system stores system paths only, but will use the URL alias for display. %link_path has been stored as %normal_path', array('%link_path' => $item['link_path'], '%normal_path' => $normal_path)));
    $item['link_path'] = $normal_path;
  }
  if (!url_is_external($item['link_path'])) {
    $parsed_link = parse_url($item['link_path']);
    if (isset($parsed_link['query'])) {
      $item['options']['query'] = backdrop_get_query_array($parsed_link['query']);
    } else {
      // Use unset() rather than setting to empty string
      // to avoid redundant serialized data being stored.
      unset($item['options']['query']);
    }
    if (isset($parsed_link['fragment'])) {
      $item['options']['fragment'] = $parsed_link['fragment'];
    } else {
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
function menu_item_delete_submit($form, &$form_state)
{
  $form_state['redirect'] = 'admin/structure/menu/item/' . $form_state['values']['mlid'] . '/delete';
}

/**
 * Process menu and menu item add/edit form submissions.
 */
function menu_edit_item_submit($form, &$form_state)
{
  $item = & $form_state['values'];

  // The value of "hidden" is the opposite of the value
  // supplied by the "enabled" checkbox.
  $item['hidden'] = (int)!$item['enabled'];
  unset($item['enabled']);

  $item['options']['attributes']['title'] = $item['description'];
  list($item['menu_name'], $item['plid']) = explode(':', $item['parent']);
  if (!menu_link_save($item)) {
    backdrop_set_message(t('There was an error saving the menu link.'), 'error');
  } else {
    backdrop_set_message(t('Your configuration has been saved.'));
  }
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $item['menu_name'];
}

/**
 * Menu callback; Build the form that handles the adding/editing of a custom menu.
 */
function menu_edit_menu($form, &$form_state, $type, $menu = array(), $target = NULL)
{
  if ($type == 'add') {
    $query = backdrop_get_query_parameters();
    if (isset($query['translation'])) {
      $menu = array(
        'tmenu_name' => $query['translation'],
        'langcode' => $query['target'],
        'multilingual_mode' => MENU_MULTILINGUAL_ENABLED_WITH_TRANSLATION
      );
    }
  }

  $system_menus = menu_list_system_menus();
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

  // Enable language if language module is enabled.
  $multilanguage = module_exists('language');

  if ($multilanguage) {
    $form['multilingual_mode'] = array(
      '#type' => 'radios',
      '#title' => t('Multilingual support'),
      '#default_value' => $menu['multilingual_mode'],
      '#description' => '
        <ul>
          <li><strong>Disabled</strong>: The menu will show all links of all languages.</li>
          <li><strong>Enabled</strong>: The menu will show links in the current language.
          You may also select an optional fixed language.
          The menu will then only be visible in the selected language and show only links
          in this language. New links will be saved using the selected language.</li>
          <li><strong>Enabled, with translation</strong>: The menu is only visible in the selected language and show only links
           in this language. Menus in other languages can be linked as translations.</li>
        </ul>',
      '#options' => array(
        MENU_MULTILINGUAL_DISABLED => t('Disabled'),
        MENU_MULTILINGUAL_ENABLED => t('Enabled'),
        MENU_MULTILINGUAL_ENABLED_WITH_TRANSLATION => t('Enabled, with translation')
      )
    );

    $languages = language_list(TRUE);
    $language_options = array();
    foreach ($languages as $lcode => $language) {
      $language_options[$lcode] = $language->name;
    }
    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#description' => t('Provide a fixed language for this menu. The menu will only be visible in this language. New links will be saved using this language.<br>When selecting none, the links will be shown based on the current language. New links will then be saved without a language setting.'),
      '#default_value' => $menu['langcode'],
      '#states' => array(
        'visible' => array(
          ':input[name="multilingual_mode"]' => array('!value' => (string)MENU_MULTILINGUAL_DISABLED),
        ),
        'required' =>  array(
          ':input[name="multilingual_mode"]' => array('value' => (string)MENU_MULTILINGUAL_ENABLED_WITH_TRANSLATION),
        ),
      ),
      '#options' => $language_options,
      '#empty_value' => LANGUAGE_NONE,
    );
  }
  else {
    $form['langcode'] = array(
      '#type' => 'hidden',
      '#value' => $menu['langcode']
    );
  }
  $form['tmenu_name'] = array(
    '#type' => 'hidden',
    '#value' => $menu['tmenu_name']
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
    '#access' => $type == 'edit' && !isset($system_menus[$menu['menu_name']]),
    '#submit' => array('menu_custom_delete_submit'),
  );

  return $form;
}

/**
 * Submit function for the 'Delete' button on the menu editing form.
 */
function menu_custom_delete_submit($form, &$form_state)
{
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $form_state['values']['menu_name'] . '/delete';
}

/**
 * Menu callback; check access and get a confirm form for deletion of a custom menu.
 */
function menu_delete_menu_page($menu)
{
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
function menu_delete_menu_confirm($form, &$form_state, $menu)
{
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
function menu_delete_menu_confirm_submit($form, &$form_state)
{
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
function menu_edit_menu_name_exists($value)
{
  // 'menu-' is added to the menu name to avoid name-space conflicts.
  $value = 'menu-' . $value;
  $custom_exists = (bool) menu_load($value);
  $link_exists = db_query_range("SELECT 1 FROM {menu_links} WHERE menu_name = :menu", 0, 1, array(':menu' => $value))->fetchField();

  return $custom_exists || $link_exists;
}

/**
 * Submit function for adding or editing a custom menu.
 */
function menu_edit_menu_submit($form, &$form_state)
{
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
  } else {
    menu_save($menu);
    $result = db_query("SELECT mlid FROM {menu_links} WHERE link_path = :path", array(':path' => $path . $menu['menu_name']), array('fetch' => PDO::FETCH_ASSOC));
    foreach ($result as $m) {
      $link = menu_link_load($m['mlid']);
      $link['link_title'] = $menu['title'];
      menu_link_save($link);
    }
  }
  backdrop_set_message(t('Your configuration has been saved.'));
  $form_state['redirect'] = $path . $menu['menu_name'];
}

/**
 * Menu callback; Check access and present a confirm form for deleting a menu link.
 */
function menu_item_delete_page($item)
{
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
function menu_item_delete_form($form, &$form_state, $item)
{
  $form['#item'] = $item;
  return confirm_form($form, t('Are you sure you want to delete the custom menu link %item?', array('%item' => $item['link_title'])), 'admin/structure/menu/manage/' . $item['menu_name']);
}

/**
 * Process menu delete form submissions.
 */
function menu_item_delete_form_submit($form, &$form_state)
{
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
function menu_reset_item_confirm($form, &$form_state, $item)
{
  $form['item'] = array('#type' => 'value', '#value' => $item);
  return confirm_form($form, t('Are you sure you want to reset the link %item to its default values?', array('%item' => $item['link_title'])), 'admin/structure/menu/manage/' . $item['menu_name'], t('Any customizations will be lost. This action cannot be undone.'), t('Reset'));
}

/**
 * Process menu reset item form submissions.
 */
function menu_reset_item_confirm_submit($form, &$form_state)
{
  $item = $form_state['values']['item'];
  $new_item = menu_reset_item($item);
  backdrop_set_message(t('The menu link was reset to its default settings.'));
  $form_state['redirect'] = 'admin/structure/menu/manage/' . $new_item['menu_name'];
}

/**
 * Menu callback; Build the form presenting menu configuration options.
 *
 * @ingroup forms
 * @see menu_configure_submit()
 */
function menu_configure($form, &$form_state) {
  $config = config('menu.settings');
  $form['intro'] = array(
    '#type' => 'item',
    '#markup' => t('The menu module allows on-the-fly creation of menu links in the content authoring forms. To configure these settings for a particular content type, visit the <a href="@content-types">Content types</a> page, click the <em>edit</em> link for the content type, and go to the <em>Menu settings</em> section.', array('@content-types' => url('admin/structure/types'))),
  );

  $menu_options = menu_get_menus();

  $main = $config->get('menu_main_links');
  $form['menu_main_links_source'] = array(
    '#type' => 'select',
    '#title' => t('Source for the Main links'),
    '#default_value' => $main,
    '#empty_option' => t('No Main links'),
    '#options' => $menu_options,
    '#tree' => FALSE,
    '#description' => t('Select what should be displayed as the Main links (typically at the top of the page).'),
  );

  $form['menu_secondary_links_source'] = array(
    '#type' => 'select',
    '#title' => t('Source for the Secondary links'),
    '#default_value' => $config->get('menu_secondary_links'),
    '#empty_option' => t('No Secondary links'),
    '#options' => $menu_options,
    '#tree' => FALSE,
    '#description' => t('Select the source for the Secondary links. An advanced option allows you to use the same source for both Main links (currently %main) and Secondary links: if your source menu has two levels of hierarchy, the top level menu links will appear in the Main links, and the children of the active link will appear in the Secondary links.', array('%main' => $main ? $menu_options[$main] : t('none'))),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );
  return $form;
}

/**
 * Form submission handler for menu_configure().
 */
function menu_configure_submit($form, &$form_state) {
  config('menu.settings')
    ->set('menu_main_links', $form_state['values']['menu_main_links_source'])
    ->set('menu_secondary_links', $form_state['values']['menu_secondary_links_source'])
    ->save();
  // Display status messages for this form.
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Menu callback; Translate the menu form.
 */
function menu_translate_menu($form, &$form_state, $menu)
{
  if(strlen($menu['tmenu_name'])>0) {
    // Already part of a set, grab the set
    $translations = menu_get_menu_translation_set($menu['tmenu_name']);
  }
  else {
    // We have no translation source nid, this could be a new set, emulate that.
    $menu['tmenu_name'] = $menu['menu_name'];
    $translations = array(
      $menu['langcode'] => $menu
    );
  }

  $form['translation_set'] = array('#type' => 'value', '#value' => $translations);
  $header = array(t('Language'), t('Title'), t('Status'), t('Operations'));

  $menus = menu_load_all();
  $form['translations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translations'),
    '#tree' => TRUE,
    '#weight' => 50,
    '#description' => t('Select existing menus that will be considered as translations of each other. Only menus with multilingual support enabled are listed here.'),
  );

  foreach (language_list() as $langcode => $language) {
    $options = array();
    $language_name = $language->name;
    $status = '';

    if (isset($translations[$langcode])) {
      // Existing translation in the translation set: display status.
      // We load the full menu to check whether the user can edit it.
      $translation_menu = menu_load($translations[$langcode]['menu_name']);
      $title = $translation_menu['title'];
      $path = 'admin/structure/menu/manage/' . $translation_menu['menu_name'] . '/edit';

      $options['edit'] = array(
        'title' => t('Edit menu'),
        'href' => $path
      );

      if ($translation_menu['menu_name'] == $menu['tmenu_name']) {
        $language_name = t('<strong>@language_name</strong> (source)', array('@language_name' => $language_name));

        // We've got a predefined item for this language
        $form['translations'][$langcode] = array(
          '#type' => 'value',
          '#value' => $translation_menu['menu_name']
        );
        $form['translations']['display'] = array(
          '#type' => 'item',
          '#title' => $language->name,
          '#markup' => check_plain($translation_menu['title']),
        );
      }
    }
    else {
      // No such translation in the set yet: help user to create it.
      $title = t('N/A');
      $options['add'] = array(
        'title' => t('Add translation'),
        'href' => 'admin/structure/menu/add',
        'query' => array('translation' => $menu['menu_name'], 'target' => $langcode)
      );

      $status = t('Not translated');
    }

    if (!isset($form['translations'][$langcode])) {
      // Generate a list of possible translation candidates (not including this menu).
      $translation_options = array();
      foreach($menus as $candidate) {
        if ( ($candidate['menu_name'] != $menu['menu_name']
          && $candidate['multilingual_mode'] > MENU_MULTILINGUAL_DISABLED
          && ($candidate['langcode'] == LANGUAGE_NONE || $candidate['langcode'] == $langcode))
        || ($candidate['menu_name'] == $menu['menu_name'] && $langcode == $menu['langcode'])
        ) {
          $translation_options[$candidate['menu_name']] = $candidate['title'];
        }
      }

      $default = isset($translations[$langcode]) ? $translations[$langcode]['menu_name'] : 'navigation';
      if (!isset($translation_options[$default])) {
        $default = 'navigation';
      }
      $form['translations'][$langcode] = array(
        '#type' => 'select',
        '#title' => $language->name,
        '#default_value' => $default,
        '#options' => $translation_options,
        '#description' => t('Select the :language menu.', array(':language'=>$language->name)),
        '#attributes' => array('class' => array('menu-title-select')),
        '#empty_value' => '',
      );
    }

    $row = array();
    $row[] = $language_name;
    $row[] = $title;
    $row[] = $status;
    $row[] = array(
      'data' => array(
        '#type' => 'operations',
        '#links' => $options,
      ),
    );
    $rows[] = $row;
  }

  backdrop_set_title(t('Translations of %title', array('%title' => $menu['title'])), PASS_THROUGH);

  $form['translation_node_overview'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['update'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

/**
 * Callback for the translation menu edit form.
 * @param $form
 * @param $form_state
 */
function menu_translate_menu_submit($form, &$form_state) {

  switch ($form_state['values']['op']) {
    case t('Save'):
      $translations = $form_state['values']['translation_set'];

      // Reset the translations.
      foreach($translations as $langcode => $menu) {
        $source_name = $menu['tmenu_name'];
        $menu['tmenu_name'] = '';
        menu_save($menu);
      }

      // Save the new ones.
      $menu_names = array_filter($form_state['values']['translations']);
      foreach ($menu_names as $langcode => $menu_name) {
        $menu = menu_load($menu_name);
        $menu['multilingual_mode'] = MENU_MULTILINGUAL_ENABLED_WITH_TRANSLATION;
        $menu['langcode'] = $langcode;
        $menu['tmenu_name'] = $source_name;
        menu_save($menu);
      }
      backdrop_set_message(t('The menu translations have been saved.'));

      break;
  }

}