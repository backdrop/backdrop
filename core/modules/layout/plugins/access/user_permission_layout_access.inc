<?php
/**
 * @file
 * Plugin to provide access control based on user permission strings.
 */
class UserPermissionLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = []) {
    parent::__construct($plugin_name, $data);
    $this->settings += [
      'permission' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    if (empty($this->settings['permission'])) {
      return t('Error, unset permission');
    }

    $permissions = module_invoke_all('permission');
    if (!array_key_exists($this->settings['permission'], $permissions)) {
      return t('User has missing "@permission" permission (not provided by any module).', [
        '@permission' => $this->settings['permission'],
      ]);
    }
    return t('User has "@permission" permission.', ['@permission' => $permissions[$this->settings['permission']]['title']]);
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $account = $this->contexts['user']->data;
    return user_access($this->settings['permission'], $account);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $options = [];
    // Get list of permissions
    foreach (module_list(FALSE, FALSE, TRUE) as $module) {
      // By keeping them keyed by module we can use optgroups with the
      // 'select' type.
      if ($permissions = module_invoke($module, 'permission')) {
        foreach ($permissions as $id => $permission) {
          $options[$module][$id] = strip_tags($permission['title']);
        }
      }
    }

    $form['permission'] = [
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Permission'),
      '#default_value' => $this->settings['permission'],
      '#description' => t('Only users with the selected permission flag will be able to access this.'),
    ];
  }
}
