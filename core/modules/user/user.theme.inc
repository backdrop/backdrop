<?php
/**
 * @file
 * Theme functions for the User module.
 */

/**
 * Returns HTML for a list of users.
 *
 * @param $variables
 *   An associative array containing:
 *   - users: An array with user objects. Should contain at least the name and
 *     uid.
 *   - title: (optional) Title to pass on to theme_item_list().
 *
 * @ingroup themeable
 */
function theme_user_list($variables) {
  $users = $variables['users'];
  $title = $variables['title'];
  $items = array();

  if (!empty($users)) {
    foreach ($users as $user) {
      $items[] = theme('username', array('account' => $user));
    }
  }
  return theme('item_list', array('items' => $items, 'title' => $title));
}

/**
 * Returns HTML for a user signature.
 *
 * @param $variables
 *   An associative array containing:
 *   - signature: The user's signature.
 *
 * @ingroup themeable
 */
function theme_user_signature($variables) {
  $signature = $variables['signature'];
  $output = '';

  if ($signature) {
    $output .= '<div class="clear">';
    $output .= '<div>—</div>';
    $output .= $signature;
    $output .= '</div>';
  }

  return $output;
}

/**
 * Returns HTML for a username, potentially linked to the user's page.
 *
 * @param $variables
 *   An associative array containing:
 *   - account: The user object to format.
 *   - name: The user's name, sanitized.
 *   - extra: Additional text to append to the user's name, sanitized.
 *   - link_path: The path or URL of the user's profile page, home page, or
 *     other desired page to link to for more information about the user.
 *   - link_options: An array of options to pass to the l() function's $options
 *     parameter if linking the user's name to the user's page.
 *   - attributes: An array of attributes to pass to the
 *     backdrop_attributes() function if not linking to the user's page.
 *
 *   $variables['account'] is typically a User object, but it can be any object
 *   that provides the uid and name properties, for example, a Node.
 *
 * @see template_preprocess_username()
 * @see template_preprocess_node()
 */
function theme_username($variables) {
  if (isset($variables['link_path'])) {
    // Finalize the link_options array for passing to the l() function.
    // Additional classes may be added here as array elements like
    // $variables['attributes']['class'][] = 'myclass';

    // $variables['attributes'] contains attributes that should be applied
    // regardless of whether a link is being rendered.
    // $variables['link_attributes'] contains attributes that should only be
    // applied if a link is being rendered.
    // If a link is being rendered, these need to be merged. Some attributes are
    // themselves arrays, so the merging needs to be recursive.
    $variables['link_options']['attributes'] = array_merge_recursive($variables['link_attributes'], $variables['attributes']);
    // We have a link path, so we should generate a link using l().
    $output = l($variables['name'] . $variables['extra'], $variables['link_path'], $variables['link_options']);
  }
  else {
    $output = '<span' . backdrop_attributes($variables['attributes']) . '>' . $variables['name'] . $variables['extra'] . '</span>';
  }
  return $output;
}

/**
 * Returns HTML for the administer permissions page.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_user_admin_permissions($variables) {
  $form = $variables['form'];

  $roles = user_roles();
  foreach (element_children($form['permission']) as $key) {
    $row = array(
      'data' => array(),
    );

    // Hide the permission (or module) name and description element, since the
    // individual properties will be pulled out in each row.
    hide($form['permission'][$key]);

    // Module name row.
    if (is_numeric($key)) {
      $module_text = '<div class="permission-module-name table-filter-text-source">' . $form['permission'][$key]['#markup'] . '</div>';
      $row['class'] = array('module-row');
      $row['data'][] = array(
        'data' => $module_text,
        'class' => array('module'),
        'id' => 'module-' . $form['permission'][$key]['#id'],
        'colspan' => count($form['roles']['#value']) + 1,
      );
    }
    // Permission row.
    else {
      $permission_text = '<div class="permission-name table-filter-text-source">' . $form['permission'][$key]['#markup'] . '</div>';
      $permission_text .= $form['permission'][$key]['#description'];

      $row['data'][] = array(
        'data' => $permission_text,
        'class' => array('permission', 'table-filter-text'),
        'id' => $form['permission'][$key]['#id'],
      );
      foreach (element_children($form['checkboxes']) as $role_name) {
        $form['checkboxes'][$role_name][$key]['#title'] = $roles[$role_name] . ': ' . $form['permission'][$key]['#markup'];
        $form['checkboxes'][$role_name][$key]['#title_display'] = 'attribute';
        $row['data'][] = array(
          'data' => backdrop_render($form['checkboxes'][$role_name][$key]),
          'class' => array('checkbox'),
        );
      }
    }
    $rows[] = $row;
  }
  $header[] = (t('Permission'));
  foreach (element_children($form['role_names']) as $role_name) {
    $header_cell = array(
      'data' => backdrop_render($form['role_names'][$role_name]),
      'class' => array('checkbox'),
    );
    // Output a role description if present as a title attribute on the cell.
    if (isset($form['role_names'][$role_name]['#description'])) {
      $header_cell['title'] = check_plain(strip_tags($form['role_names'][$role_name]['#description']));
    }
    $header[] = $header_cell;
  }

  $output = backdrop_render($form['filter']);
  // Wrap the table in its own class so that we can make it horizontally
  // scrollable.
  $output .= '<div class="permissions-matrix">';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'permissions')));
  $output .= '</div>';
  $output .= backdrop_render_children($form);
  return $output;
}

/**
 * Returns HTML for an individual permission description.
 *
 * @param $variables
 *   An associative array containing:
 *   - permission_item: An associative array representing the permission whose
 *     description is being themed. Useful keys include:
 *     - description: The text of the permission description.
 *     - warning: A security-related warning message about the permission (if
 *       there is one).
 *
 * @ingroup themeable
 */
function theme_user_permission_description($variables) {
  $description = array();
  $permission_item = $variables['permission_item'];
  if (!empty($permission_item['description'])) {
    $description[] = '<div class="permission-description description table-filter-text-source">' . $permission_item['description'] . '</div>';
  }
  if (!empty($permission_item['restrict access'])) {
    $description[] = '<em class="permission-warning">' . t('Warning: Give to trusted roles only; this permission has security implications.') . '</em>';
  }
  if (!empty($permission_item['warning'])) {
    $description[] = '<a class="warning-toggle" href="#">more</a>';
    $description[] = '<div class="permission-warning-description">' . $permission_item['warning'] . '</div>';
  }
  if (!empty($description)) {
    return implode(' ', $description);
  }
}

/**
 * Returns HTML for the role order form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_user_admin_roles($variables) {
  $form = $variables['form'];

  $header = array(
    array('data' => t('Name'), 'class' => array('role-name')),
    array(
      'data' => t('Description'),
      'class' => array('role-description', 'priority-low'),
    ),
    array('data' => t('Weight')),
    array('data' => t('Operations'), 'class' => array('role-operations')),
  );
  foreach (element_children($form['roles']) as $role_name) {
    $label = $form['roles'][$role_name]['#role']->label;
    if (in_array($role_name, array(BACKDROP_ANONYMOUS_ROLE, BACKDROP_AUTHENTICATED_ROLE))) {
      $label .= ' ' . t('(required)');
    }

    $row = array();
    $row[] = array(
      'data' => theme('label_machine_name__role', array(
        'label' => $label,
        'machine_name' => $form['roles'][$role_name]['#role']->name,
      )),
      'class' => array('role-name'),
    );
    $row[] = array(
      'data' => filter_xss_admin($form['roles'][$role_name]['#role']->description),
      'class' => array('role-description', 'priority-low'),
    );
    $row[] = array(
      'data' => backdrop_render($form['roles'][$role_name]['weight']),
    );
    $row[] = array(
      'data' => backdrop_render($form['roles'][$role_name]['operations']),
      'class' => array('role-operations'),
    );
    $rows[] = array('data' => $row, 'class' => array('draggable'));
  }

  backdrop_add_tabledrag('user-roles', 'order', 'sibling', 'role-weight');

  $output = backdrop_render($form['roles_help']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'user-roles')));
  $output .= backdrop_render_children($form);

  return $output;
}

/**
 * Process variables for user-picture.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $account: A user, node or comment object with 'name', 'uid' and 'picture'
 *   fields.
 *
 * @see user-picture.tpl.php
 */
function template_preprocess_user_picture(&$variables) {
  $site_config = config('system.core');
  $variables['user_picture'] = '';
  if ($site_config->get('user_pictures')) {
    $account = $variables['account'];
    if (!empty($account->picture)) {
      // @TODO: Ideally this function would only be passed file entities, but
      // since there's a lot of legacy code that JOINs the {users} table to
      // {node} or {comments} and passes the results into this function if we
      // a numeric value in the picture field we'll assume it's a file id
      // and load it for them. Once we've got user_load_multiple() and
      // comment_load_multiple() functions the user module will be able to load
      // the picture files in mass during the object's load process.
      if (is_numeric($account->picture)) {
        $account->picture = file_load($account->picture);
      }
      if (!empty($account->picture->uri)) {
        $filepath = $account->picture->uri;
      }
    }
    elseif ($site_config->get('user_picture_default')) {
      $filepath = $site_config->get('user_picture_default');
    }
    if (isset($filepath)) {
      $alt = t("@user's picture", array('@user' => user_format_name($account)));
      // If the image does not have a valid Backdrop scheme (for eg. HTTP),
      // don't load image styles.
      if (module_exists('image') && file_valid_uri($filepath) && $style = $site_config->get('user_picture_style')) {
        $variables['user_picture'] = theme('image_style', array('style_name' => $style, 'uri' => $filepath, 'alt' => $alt, 'title' => $alt));
      }
      else {
        $variables['user_picture'] = theme('image', array('path' => $filepath, 'alt' => $alt, 'title' => $alt));
      }
      if (!empty($account->uid) && user_access('access user profiles')) {
        $attributes = array('attributes' => array('title' => t('View user profile.')), 'html' => TRUE);
        $variables['user_picture'] = l($variables['user_picture'], "user/$account->uid", $attributes);
      }
    }
  }
}

/**
 * Preprocesses variables for theme_username().
 *
 * Modules that make any changes to variables like 'name' or 'extra' must insure
 * that the final string is safe to include directly in the output by using
 * check_plain() or filter_xss().
 */
function template_preprocess_username(&$variables) {
  $account = $variables['account'];

  $variables['extra'] = '';
  if (empty($account->uid)) {
    $variables['uid'] = 0;
    $variables['extra'] = ' (' . t('not verified') . ')';
  }
  else {
    $variables['uid'] = (int) $account->uid;
  }

  // Set the name to a formatted name that is safe for printing and
  // that won't break tables by being too long. Keep an unshortened,
  // unsanitized version, in case other preprocess functions want to implement
  // their own shortening logic or add markup. If they do so, they must ensure
  // that $variables['name'] is safe for printing.
  $name = $variables['name_raw'] = user_format_name($account);
  if (backdrop_strlen($name) > 20) {
    $name = backdrop_substr($name, 0, 15) . '...';
  }
  $variables['name'] = check_plain($name);
  $variables['profile_access'] = user_access('access user profiles');

  // Populate link path and attributes if appropriate.
  if ($variables['uid'] && $variables['profile_access']) {
    // We are linking to a local user.
    $variables['link_attributes']['title'] = t('View user profile.');
    $variables['link_path'] = 'user/' . $variables['uid'];
  }
  elseif (!empty($account->homepage)) {
    // Like the 'class' attribute, the 'rel' attribute can hold a
    // space-separated set of values, so initialize it as an array to make it
    // easier for other preprocess functions to append to it.
    $variables['link_attributes']['rel'] = 'nofollow';
    $variables['link_path'] = $account->homepage;
    $variables['homepage'] = $account->homepage;
  }
  // We do not want the l() function to check_plain() a second time.
  $variables['link_options']['html'] = TRUE;
  // Set a default class.
  $variables['attributes'] = array('class' => array('username'));
}

/**
 * Process variables for user-profile.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $account
 *
 * @see user-profile.tpl.php
 */
function template_preprocess_user_profile(&$variables) {
  $view_mode = $variables['view_mode'] = $variables['elements']['#view_mode'];
  $account = $variables['elements']['#account'];

  // Helpful $user_profile variable for templates.
  foreach (element_children($variables['elements']) as $key) {
    $variables['user_profile'][$key] = $variables['elements'][$key];
  }

  // Preprocess fields.
  field_attach_preprocess('user', $account, $variables['elements'], $variables);

  // Add extra theme hook suggestions for user view_mode and id.
  $variables['theme_hook_suggestions'][] = 'user_profile__' . $view_mode;
  $variables['theme_hook_suggestions'][] = 'user_profile__' . $account->uid;
  $variables['theme_hook_suggestions'][] = 'user_profile__' . $account->uid . '__' . $view_mode;
}
