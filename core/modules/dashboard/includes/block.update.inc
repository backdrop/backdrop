<?php
/**
 * @file
 * "Update" dashboard block.
 */

class DashboardUpdateBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $this->settings += array(
      'project_types' => array(
        'core',
        'modules',
        'themes',
        'layouts',
      ),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Available Updates');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Available updates');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Displays available updates for core, modules, themes, and layouts.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!module_exists('update')) {
      return;
    }

    $items = array();
    $available = update_get_available();
    module_load_include('inc', 'update', 'update.compare');
    $data = update_calculate_project_data($available);

    foreach ($data as $key => $project) {
      // Filter out projects which are up to date already.
      if ($project['status'] == UPDATE_CURRENT) {
        continue;
      }

      $type = $project['project_type'];
      if (in_array($type, $this->settings['project_types'])) {
        $name = $project['info']['name'];
        if ($project['name'] == 'backdrop') {
          $name = t('Backdrop');
        }

        $item = $name . ' ' . $type;

        if (isset($project['recommended'])) {
          $new = t('version @new available', array('@new' => $project['recommended']));
          if (user_access('access_site_reports')) {
            $new = l($new, 'admin/reports/updates');
          }
          $item .= ' (' . $new . ')';

          if (module_exists('installer') && user_access('administer software updates')) {
            $options = array('attributes' => array('class' => array('hi')));
            $item .= ' - ' . l(t('Update'), 'admin/modules/update', $options);
          }
        }
        else {
          // No updates available.
          $item .= ' - ' . t('No recommended updates');
        }


        $items[] = $item;
      }
    }

    $panel = array(
      '#theme' => 'dashboard_panel__update',
    );
    $panel['list'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
    );

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!module_exists('update')) {
      return $form;
    }

    $form['project_types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display information about updates to the following'),
      '#multiple' => TRUE,
      '#options' => array(
        'core' => t('Backdrop core'),
        'module' => t('Modules'),
        'theme' => t('Themes'),
        'layout' => t('Layouts'),
      ),
      '#default_value' => $this->settings['project_types'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['project_types'] = array_keys(array_filter($form_state['values']['project_types']));
  }
}
