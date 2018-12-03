<?php

  /**
   * @file
   *
   * "Site overview" panel pane. Displays a summary of both content and user
   * statistics.
   *
   */

class DashboardOverviewBlock extends Block {

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Content and User overview');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('An overview of both content and users in one pane');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    // Content Overview.
    $overview_cnt = dashboard_get_content_overview($this->settings);

  // User Overview.  
    $overview_usr = array();
    $user_data = dashboard_get_user_overview($this->settings);
    $users_all = format_plural($user_data['all'], '1 total user', '@count total users');
    $users_active = format_plural($user_data['active'], '1 active', '@count active');
    $users_blocked = format_plural($user_data['blocked'], '1 blocked', '@count blocked');
    $overview_usr[] = $users_all . ' | ' . $users_active . ' | ' . $users_blocked;

  // Roles Overview.
    foreach ($user_data['roles'] as $rid => $data) {
      $overview_usr['role_' . $rid] = format_plural($user_data['roles'][$rid]['count'], '1 user account', '@count user accounts');
      $overview_usr['role_' . $rid] .= ' with the role: ' . $user_data['roles'][$rid]['name'];
    }

    return theme('dashboard_overview', array('content' => $overview_cnt, 'users' => $overview_usr));

  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    // Add content settings to form.
    dashboard_add_content_pane_settings($form, $this->settings);
    // Add user settings to form.
    dashboard_add_user_pane_settings($form, $this->settings);
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['spam'] = $form_state['values']['spam'];
    foreach (array_keys($form_state['values']['types']) as $key) {
      $this->settings['types'][$key] = $form_state['values']['types'][$key];
    }
    foreach (array_keys($form_state['values']['comments']) as $key) {
      $this->settings['comments'][$key] = $form_state['values']['comments'][$key];
    }
    $this->settings['user'] = $form_state['values']['user'];
    foreach (array_keys($form_state['values']['roles']) as $key) {
      $this->settings['roles'][$key] = $form_state['values']['roles'][$key];
    }
  }
}
