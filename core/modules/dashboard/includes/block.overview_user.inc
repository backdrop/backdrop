<?php
/**
 * @file
 * Dashboard block displaying informtion about user accounts, including:
 * - number of total user accounts.
 * - number of active user accounts.
 * - number of blocked user accounts.
 * - number of users in each role.
 */
class DashboardOverviewUserBlock extends Block {

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $role_options = user_roles(TRUE);
    unset($role_options[BACKDROP_AUTHENTICATED_ROLE]);

    // Set defaults.
    $this->settings += array(
      'user_counts' => TRUE,
      'user_counts_active' => TRUE,
      'user_counts_blocked' => TRUE,
      'roles' => $role_options,
    );
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('User accounts');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Overview of user accounts');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Displays a summary of user statistics including: number of total users, active users, blocked users, and users in each role.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    $settings = $this->settings;

    $user_data = array();
    if ($cache = cache_get('dashboard_user_overview')) {
      $user_data = $cache->data;
    }
    else {
      // User Overview.
      if ($settings['user_counts']) {
        $user_data['all'] = db_query("SELECT count(*) FROM {users}")->fetchField();
        // Account for UID 0. Faster than adding a where clause.
        $user_data['all']--;
      }
      if ($settings['user_counts_active']) {
        $user_data['active'] = db_query("SELECT count(*) FROM {users} WHERE status = 1 AND login <> 0")->fetchField();
      }
      if ($settings['user_counts_blocked']) {
        $user_data['blocked'] = db_query("SELECT count(*) FROM {users} WHERE status = 0")->fetchField();
        // Account for UID 0. Faster than adding a where clause.
        $user_data['blocked']--;
      }

      // Roles Overview.
      $roles = user_roles(TRUE);
      $user_data['roles'] = array();
      foreach ($roles as $role_name => $role_label) {
        if (($role_name != BACKDROP_AUTHENTICATED_ROLE) && (in_array($role_name, $settings['roles']))) {
          $user_role_count = db_query("SELECT count(*) FROM {users} u INNER JOIN {users_roles} r on u.uid = r.uid WHERE r.role = :rid", array(':rid' => $role_name))->fetchField();
          $user_data['roles'][$role_name] = array();
          $user_data['roles'][$role_name]['name'] = $role_label;
          $user_data['roles'][$role_name]['count'] = $user_role_count;
        }
      }

      cache_set('dashboard_user_overview', $user_data, 'cache');
    }

    $overview = array();

    if ($settings['user_counts']) {
      $overview['all'] = format_plural($user_data['all'], '1 user account', '@count total user accounts');
      $overview['active'] = format_plural($user_data['active'], '1 active user account', '@count active user accounts');
      $overview['blocked'] = format_plural($user_data['blocked'], '1 blocked user account', '@count blocked user accounts');
    }

    foreach ($user_data['roles'] as $role_name => $data) {
      $overview['roles_' . $role_name] = format_plural(
        $user_data['roles'][$role_name]['count'],
        '1 user account with the role @role',
        '@count user accounts with the role @role',
        array('@role' => check_plain($user_data['roles'][$role_name]['name']))
      );
    }

    return theme('dashboard_overview_user', array('overview' => $overview));
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $settings = $this->settings;

    $form['user_counts'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display the <strong>total</strong> number of user accounts.'),
      '#default_value' => $settings['user_counts'],
    );
    $form['user_counts_active'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display the number of <strong>active</strong> user accounts.'),
      '#default_value' => $settings['user_counts_active'],
    );
    $form['user_counts_blocked'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display the number of <strong>blocked</strong> user accounts.'),
      '#default_value' => $settings['user_counts_blocked'],
    );

    $role_options = user_roles(TRUE);
    unset($role_options[BACKDROP_AUTHENTICATED_ROLE]);

    if (!empty($role_options)) {
      $form['roles'] = array(
        '#type' => 'checkboxes',
        '#title' => t('List the number of accounts with the following roles:'),
        '#options' => $role_options,
        '#default_value' => $settings['roles'],
      );
    }
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['user_counts'] = (bool) $form_state['values']['user_counts'];
    $this->settings['user_counts_active'] = (bool) $form_state['values']['user_counts_active'];
    $this->settings['user_counts_blocked'] = (bool) $form_state['values']['user_counts_blocked'];
    $this->settings['roles'] = array_keys(array_filter($form_state['values']['roles']));
  }
}
