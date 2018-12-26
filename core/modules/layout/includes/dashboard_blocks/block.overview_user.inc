<?php

  /**
   * @file
   *
   * "User overview" content type. Displays a summary of user statistics
   * including: number of total users, active users, blocked users, and
   * users in each role.
   *
   */

class DashboardOverviewUserBlock extends Block {


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
    return t('Displays a summary of user statistics including: number 
      of total users, active users, blocked users, and users in each role.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
      dpm($this);
    $user_data = dashboard_get_user_overview($this->settings);

    $overview = array();
    $overview['all'] = format_plural($user_data['all'], '1 user account', '@count total user accounts');
    $overview['active'] = format_plural($user_data['active'], '1 active user account', '@count active user accounts');
    $overview['blocked'] = format_plural($user_data['blocked'], '1 blocked user account', '@count blocked user accounts');

    foreach ($user_data['roles'] as $rid => $data) {
        $overview['roles_' . $rid]  = format_plural($user_data['roles'][$rid]['count'], '1 user account', '@count user accounts');
        $overview['roles_' . $rid] .= ' with the role: ' . $user_data['roles'][$rid]['name'];
      }

    return theme('dashboard_overview_user', array('overview' => $overview));
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    // Add user settings to form.
    dashboard_add_user_pane_settings($form, $this->settings);
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

      dpm($form_state);
    $this->settings['user'] = $form_state['values']['user'];
    foreach (array_keys($form_state['values']['roles']) as $key) {
      $this->settings['roles'][$key] = $form_state['values']['roles'][$key];
    }
  }
}
