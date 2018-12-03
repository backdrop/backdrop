<?php

/**
 * @file
 *
 * Helper functions for total control.
 *
 */

/**
 * Submit function for content type form
 *
 * @param $form
 *   The form triggering the display to be added
 * @param $form_state
 *   The state of the form when the request is made
 *
 */
function dashboard_add_type_submit($form, &$form_state) {
  // Get the type names.
  $machine_type = $form_state['values']['type'];
  $content_type = $form_state['values']['name'];
  dashboard_views_add_display($content_type, $machine_type);
}

/**
 * Submit function for delete content type confirm form
 *
 * @param $form
 *   The form triggering the display to be added
 * @param $form_state
 *   The state of the form when the request is made
 *
 */
function dashboard_remove_type_submit($form, &$form_state) {
  // Get the type name.
  $machine_type = $form_state['values']['type'];
  dashboard_views_delete_page_display($machine_type);
  dashboard_views_delete_pane_display($machine_type);
}

/**
 * Removes view page displays.
 *
 * @param $type
 *   The machine readable name of the content type
 *
 */
function dashboard_views_delete_page_display($type) {
  $view = views_get_view('control_content');
  if (array_key_exists('page_tc_' . $type, $view->display)) {
    unset($view->display['page_tc_' . $type]);
    $view->save();
    menu_cache_clear_all();
  }

  return;
}

/**
 * Removes view pane displays.
 *
 * @param $type
 *   The machine readable name of the content type
 *
 */
function dashboard_views_delete_pane_display($type) {
  $view = views_get_view('control_content_panes');
  if (array_key_exists('pane_tc_' . $type, $view->display)) {
    unset($view->display['pane_tc_' . $type]);
    $view->save();
  }

  return;
}

/**
 * Adds view displays.
 *
 * @param $content_type
 *   The human readable name of the content type
 * @param $machine_type
 *   The machine readable name of the content type
 *
 */
function dashboard_views_add_display($content_type = '', $machine_type = '') {
  $config = config('dashboard.settings');
  if ($machine_type = '') {
    backdrop_set_message(t('no content type provided'), 'error');
    return;
  }

  // Get total control settings.
  $auto_page_default = $config->get('dashboard_auto_pages');
  $auto_pane_default = $config->get('dashboard_auto_panels');

  if (!empty($auto_page_default) && (array_key_exists('type', $auto_page_default)) 
       && !($auto_page_default['type'] === 0)) {
    // Add page display.
    $view = views_get_view('control_content');
    if (!array_key_exists('page_tc_' . $machine_type, $view->display)) {
      dashboard_add_views_page_display($view, $content_type, $machine_type);
      $defaults = $config->get('dashboard_type_pages');
      if (array_key_exists($machine_type, $defaults) || $defaults[$machine_type] != $machine_type) {
        $defaults[$machine_type] = $machine_type;
        $defaults = config_set('dashboard.settings', 'dashboard_type_pages', $defaults);
      }
    }
    else {
      backdrop_set_message(t('Total Control was unable to create your 
        administrative page view because a view already exists with the name: 
        !name', array('!name' => 'page_tc_' . $machine_type)));
    }
  }

  if (!empty($auto_pane_default) && (array_key_exists('type', $auto_pane_default)) 
       && !($auto_pane_default['type'] === 0)) {
    // Add pane display.
    $paneview = views_get_view('control_content_panes');
    if (!array_key_exists('pane_tc_' . $machine_type, $paneview->display)) {
      dashboard_add_views_pane_display($paneview, $content_type, $machine_type);
      $defaults = $config->get('dashboard_type_panes');
      $defaults[$machine_type] = $machine_type;
      $defaults = config_set('dashboard.settings', 'dashboard_type_panes', $defaults);
    }
    else {
      backdrop_set_message(t('Total Control was unable to create your 
        administrative pane view because a view already exists with the name: 
        !name', array('!name' => 'pane_tc_' . $machine_type)));
    }
  }

  return;
}

/**
 * Adds a pane display to the view
 *
 * @param $view
 *   The view to which the display is added
 * @param $content_type
 *   The human-readable content type name
 * @param $machine_type
 *   The machine-readable content type name
 *
 */
function dashboard_add_views_pane_display(&$view, $content_type, $machine_type) {
  // Build the display.
  $handler = $view->new_display('panel_pane', $content_type . ' pane', 'panel_pane_tc_' . $machine_type);
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'New ' . $content_type . ' content';
  $handler->display->display_options['defaults']['filters'] = FALSE;
  // Add a filter for Content: Type.
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    $machine_type => $machine_type,
  );
  $handler->display->display_options['filters']['type']['expose']['operator'] = FALSE;
  $handler->display->display_options['pane_title'] = 'New ' . $content_type . ' content';
  $handler->display->display_options['pane_description'] = 'Total Control ' . $content_type . ' summary';
  $handler->display->display_options['pane_category']['name'] = 'Total Control';
  $handler->display->display_options['pane_category']['weight'] = '0';
  $handler->display->display_options['allow']['items_per_page'] = 'items_per_page';

  // save the view and return
  $view->save();
  return;
}

/**
 * Adds a page display to an existing view
 *
 * @param $view
 *   The view to which the display is added
 * @param $content_type
 *   The human-readable content type name
 * @param $machine_type
 *   The machine-readable content type name
 *
 */
function dashboard_add_views_page_display(&$view, $content_type, $machine_type) {
  // Prepare variables.
  $machine_path =  str_replace('_', '-', $machine_type);
  // Build the display.
  $handler = $view->new_display('page', $content_type . ' Page', 'page_tc_' . $machine_type);

  $handler->display->display_options['title'] = $content_type . ' content';
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['defaults']['header'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  $handler->display->display_options['path'] = 'admin/dashboard/content/' . $machine_path;
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = $content_type . ' content';
  $handler->display->display_options['menu']['weight'] = '0';

  // Filters differ depending on search module.
  $filters = dashboard_add_filters($machine_type);
  $handler->override_option('filters', $filters);

  // Save the view, flush the menu cache, and return.
  $view->save();
  menu_cache_clear_all();

  return;
}

/**
 * Returns filters to a view - tests for existence of search module
 *
 * @param $machine_type
 *   The machine-readable content type name
 *
 * @return $filters
 *   An array of filters for the view
 *
 */
function dashboard_add_filters($machine_type = NULL) {
  $filters = array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'value' => '1',
      'exposed' => TRUE,
      'expose' => array(
        'identifier' => 'status',
        'label' => 'Published',
        'required' => FALSE,
      ),
    ),
    'uid' => array(
      'id' => 'uid',
      'table' => 'users',
      'field' => 'uid',
      'operator' => 'in',
      'value' => '',
      'exposed' => TRUE,
      'expose' => array(
        'operator_id' => 'uid_op',
        'label' => 'Author',
        'operator' => 'uid_op',
        'identifier' => 'uid',
        'required' => FALSE,
        'reduce' => 0,
      ),
    ),
  );

  if ($machine_type != NULL) {
    $filters['type_1'] = array(
      'id' => 'type_1',
      'table' => 'node',
      'field' => 'type',
      'value' => array(
        $machine_type => $machine_type,
      ),
      'expose' => array(
        'operator' => FALSE,
      ),
    );
  }

  if (module_exists('search')) {
    $filters['keys'] = array(
      'id' => 'keys',
      'table' => 'search_index',
      'field' => 'keys',
      'exposed' => TRUE,
      'expose' => array(
        'operator_id' => 'keys_op',
        'label' => 'Search:',
        'operator' => 'keys_op',
        'identifier' => 'keys',
        'required' => FALSE,
      ),
    );
  }

  return $filters;
}

/**
 * Retrieves overview data for all content on the site.
 *
 * @param $conf
 *   Panel content pane config data.
 *
 * @return $items
 *   An array of items for a bulleted list.
 *
 */
function dashboard_get_content_overview($conf = array()) {
  $items = array();
  $types = node_type_get_types();
  $comments_exist = module_exists('comment');

  foreach ($types as $type => $object) {
    // Compare against type option on pane config.
    if ((!array_key_exists($type, $conf['types'])) || (isset($conf['types']) && $conf['types'][$type] == $type)) {
      $type_count = db_query("SELECT count(*) FROM {node} WHERE type = :type and status = 1", array(':type' => $type))->fetchField();
      $content_data[$type] = format_plural($type_count, '1 ' . $object->name . ' item', '@count ' . $object->name . ' items');

      // Check if comments module is enabled.
      if ($comments_exist) {

        // Compare against comment options on pane config.
        if (array_key_exists($type, $conf['comments']) && $conf['comments'][$type] === $type) {
          $comment_count = db_query("SELECT count(DISTINCT cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 1 AND n.status = 1", array(':type' => $type))->fetchField();
          $content_data[$type . '_comments'] =  format_plural($comment_count, '1 comment', '@count comments');

          // Compare against spam option checkbox on pane config.
          if (isset($conf['spam']) && $conf['spam'] == 1) {
            $spam_count = db_query("SELECT count(DISTINCT c.cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 0 AND n.status = 1", array(':type' => $type))->fetchField();
            $content_data[$type . '_comments_spam'] = format_plural($spam_count, '1 spam', '@count spam');
          }
        }
      }

      $line =  $content_data[$type];
      $line .= (isset($content_data[$type . '_comments'])) ? ' with ' . $content_data[$type . '_comments'] : '';
      $line .= (isset($content_data[$type . '_comments_spam'])) ? ' (' . $content_data[$type . '_comments_spam'] . ')' : '';
      $items[] = $line;
    }
  }

  return $items;
}

/**
 * Adds content settings to the panel pane config form.
 *
 * @param $form
 *   Panel pane config form.
 * @param $conf
 *   Panel content pane config data.
 *
 */
function dashboard_add_content_pane_settings(&$form, $conf = array()) {
  $types = node_type_get_types();
  $type_options = array();
  $type_defaults = array();
  $comment_defaults = array();

  // Set defaults based on pane config.
  if (isset($conf['types'])) {
    $type_defaults = $conf['types'];
  }

  if (isset($conf['comments'])) {
    $comment_defaults = $conf['comments'];
  }

  foreach ($types as $machine_type => $type) {
    $type_options[$machine_type] = $type->name;
    // Display new content types by default.
    if (!array_key_exists($machine_type, $type_defaults)) {
      $type_defaults[$machine_type] = $machine_type;
    }
    // Do not display comments on new types by default unless it's a blog or a forum.
    if (!array_key_exists($machine_type, $comment_defaults)) {
      if ($machine_type == 'blog' || $machine_type == 'forum topic') {
        $comment_defaults[$machine_type] = $machine_type;
      }
      else {
        $comment_defaults[$machine_type] = 0;
      }
    }
  }

  $form['types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show <strong>post</strong> counts for the following content types'),
    '#options' => $type_options,
    '#default_value' => $type_defaults,
  );

  if (module_exists('comment')) {
    $form['comments'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Show <strong>comment</strong> counts for the following content types'),
      '#options' => $type_options,
      '#default_value' => $comment_defaults,
    );
    $spam_options = array(
      0 => t('no'),
      1 => t('Include spam counts with comments'),
    );
    $form['spam'] = array(
      '#type' => 'checkbox',
      '#title' => t('Include spam counts with comments'),
      '#options' => $spam_options,
      '#default_value' => ($form_state['op'] == 'add') ? 1 : $conf['spam'],
    );
  }
}

/**
 * Retrieves overview data for all users on the site.
 *
 * @param $conf
 *   Panel content pane config data.
 *
 * @return $items
 *   A structured array of items containing user data, in the following format:
 *     $items = array(
 *       'all' => 16,
 *       'active' => 9,
 *       'blocked' => 2,
 *       'roles' => array(
 *         3 => array(
 *           'name' => 'Administrator',
 *           'count' => 4,
 *         ),
 *       ),
 *     );
 */
function dashboard_get_user_overview($conf = array()) {
  $items = array();

  // User Overview
  if (!$conf['user'] || $conf['user'] == 1) {
    $items['all'] = db_query("SELECT count(*) FROM {users}")->fetchField();
    $items['active'] = db_query("SELECT count(*) FROM {users} WHERE status = 1 AND login <> 0")->fetchField();
    $items['blocked'] = db_query("SELECT count(*) FROM {users} WHERE status = 0")->fetchField();
  }

  // Roles Overview
  $roles = user_roles(TRUE);
  $items['roles'] = array();
  foreach ($roles as $rid => $role) {
    if (($rid != 2) && (array_key_exists($rid, $conf['roles']) && $conf['roles'][$rid])) {
      $user_role_count = db_query("SELECT count(*) FROM {users} u INNER JOIN {users_roles} r on u.uid = r.uid WHERE r.role = :rid", array(':rid' => $rid))->fetchField();
      $items['roles'][$rid] = array();
      $items['roles'][$rid]['name'] = $role;
      $items['roles'][$rid]['count'] = $user_role_count;
    }
  }

  return $items;
}

/**
 * Adds user settings to the panel pane config form.
 *
 * @param $form
 *   Panel pane config form.
 * @param $conf
 *   Panel content pane config data.
 *
 */
function dashboard_add_user_pane_settings(&$form, $conf = array()) {
  $user_options = array(
    0 => t('no'),
    1 => t('Include the total number of user accounts, including active and blocked.'),
  );
  $form['user'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include the total number of user accounts, including active and blocked.'),
    '#options' => $user_options,
    '#default_value' => ($form_state['op'] == 'add') ? TRUE : $conf['user'],
  );

  $roles = user_roles(TRUE);
  $roles_options = array();
  $roles_defaults = array();
  foreach ($roles as $rid => $role) {
    if ($rid != 2) {
      $roles_options[$rid] = $role;
      $roles_defaults[] = $rid;
    }
  }

  if (!empty($roles_options)) {
    $form['roles'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Include the number of user accounts with the following roles:'),
      '#options' => $roles_options,
      '#default_value' => ($conf['roles']) ? $conf['roles'] : $roles_defaults,
    );
  }
}
