<?php

/**
 * @file
 *
 * Helper functions for total control.
 *
 */


/**
 * Retrieves overview data for all content on the site.
 *
 * @param $settings
 *   Block settings data.
 *
 * @return $items
 *   An array of items for a bulleted list.
 *
 */
function dashboard_get_content_overview($settings = array()) {
  $items = array();
  $types = node_type_get_types();
  $comments_exist = module_exists('comment');

  foreach ($types as $type => $object) {
    // Compare against type option on block settings.
    if ((!array_key_exists($type, $settings['types'])) || (isset($settings['types']) && $settings['types'][$type] == $type)) {
      $type_count = db_query("SELECT count(*) FROM {node} WHERE type = :type and status = 1", array(':type' => $type))->fetchField();
      $content_data[$type] = format_plural($type_count, '1 ' . $object->name . ' item', '@count ' . $object->name . ' items');

      // Check if comments module is enabled.
      if ($comments_exist) {

        // Compare against comment options on block settings.
        if (array_key_exists($type, $settings['comments']) && $settings['comments'][$type] === $type) {
          $comment_count = db_query("SELECT count(DISTINCT cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 1 AND n.status = 1", array(':type' => $type))->fetchField();
          $content_data[$type . '_comments'] =  format_plural($comment_count, '1 comment', '@count comments');

          // Compare against spam option checkbox on block settings.
          if (isset($settings['spam']) && $settings['spam'] == 1) {
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
 * Adds content settings to the block settings form.
 *
 * @param $form
 *   Block settings form.
 * @param $settings
 *   Block settings data.
 *
 */
function dashboard_add_content_block_settings(&$form, $settings = array()) {
  $types = node_type_get_types();
  $type_options = array();
  $type_defaults = array();
  $comment_defaults = array();

  // Set defaults based on block settings.
  if (isset($settings['types'])) {
    $type_defaults = $settings['types'];
  }

  if (isset($settings['comments'])) {
    $comment_defaults = $settings['comments'];
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
      '#default_value' => ($form_state['op'] == 'add') ? 1 : $settings['spam'],
    );
  }
}

/**
 * Retrieves overview data for all users on the site.
 *
 * @param $settings
 *   Block settings data.
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
function dashboard_get_user_overview($settings = array()) {
  $items = array();

  // User Overview
  if (!$settings['user'] || $settings['user'] == 1) {
    $items['all'] = db_query("SELECT count(*) FROM {users}")->fetchField();
    $items['active'] = db_query("SELECT count(*) FROM {users} WHERE status = 1 AND login <> 0")->fetchField();
    $items['blocked'] = db_query("SELECT count(*) FROM {users} WHERE status = 0")->fetchField();
  }

  // Roles Overview
  $roles = user_roles(TRUE);
  $items['roles'] = array();
  foreach ($roles as $rid => $role) {
    if (($rid != 2) && (array_key_exists($rid, $settings['roles']) && $settings['roles'][$rid])) {
      $user_role_count = db_query("SELECT count(*) FROM {users} u INNER JOIN {users_roles} r on u.uid = r.uid WHERE r.role = :rid", array(':rid' => $rid))->fetchField();
      $items['roles'][$rid] = array();
      $items['roles'][$rid]['name'] = $role;
      $items['roles'][$rid]['count'] = $user_role_count;
    }
  }

  return $items;
}

/**
 * Adds user settings to the block settings form.
 *
 * @param $form
 *   Block settings form.
 * @param $settings
 *   Block settings data.
 *
 */
function dashboard_add_user_block_settings(&$form, $settings = array()) {
  $user_options = array(
    0 => t('no'),
    1 => t('Include the total number of user accounts, including active and blocked.'),
  );
  $form['user'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include the total number of user accounts, including active and blocked.'),
    '#options' => $user_options,
    '#default_value' => ($form_state['op'] == 'add') ? TRUE : $settings['user'],
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
      '#default_value' => ($settings['roles']) ? $settings['roles'] : $roles_defaults,
    );
  }
}
