<?php
/**
 * @file
 * Available Updates dashboard block.
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
    $panel = array(
      '#theme' => 'dashboard_panel__update',
    );

    if (!module_exists('update')) {
      $panel['last_check'] = array(
        '#markup' => t('The Update Manager module is not enabled.'),
      );
      if (user_access('administer modules')) {
        $panel['link'] = array(
          '#theme' => 'link',
          '#path' => 'admin/modules',
          '#text' => t('Enable'),
          '#options' => array(
            'query' => array(
              'search' => 'update',
            ),
          ),
        );
      }

      return $panel;
    }

    $header = array(
      array('data' => t('Project')),
      array('data' => t('Type')),
      array('data' => t('Update')),
    );
    $updates = array();

    $available = update_get_available();
    module_load_include('inc', 'update', 'update.compare');
    $data = update_calculate_project_data($available);

    $project_types = array(
      'core' => t('Core'),
      'module' => t('Module'),
      'theme' => t('Theme'),
      'layout' => t('Layout template'),
    );

    foreach ($data as $key => $project) {
      // Skip projects that are up to date already.
      if ($project['status'] == UPDATE_CURRENT) {
        continue;
      }

      // Give core a proper title.
      if ($project['project_type'] === 'core') {
        $project['title'] = t('Backdrop');
      }

      if (in_array($project['project_type'], $this->settings['project_types'])) {
        $project_type = $project_types[$project['project_type']];

        if (isset($project['recommended'])) {
          $update = $project['recommended'];

          $update_types = array(
            'security' => t('security update'),
            'unsupported' => t('unsupported version'),
          );

          switch ($project['status']) {
            case UPDATE_NOT_SECURE:
            case UPDATE_REVOKED:
              $type = 'security';
              $weight = 0;
              break;

            case UPDATE_NOT_SUPPORTED:
              $type = 'unsupported';
              $weight = 1;
              break;

            case UPDATE_UNKNOWN:
            case UPDATE_NOT_FETCHED:
            case UPDATE_NOT_CHECKED:
            case UPDATE_NOT_CURRENT:
            default:
              $type = 'recommended';
              $weight = 2;
              break;
          }
          if (isset($type) && isset($update_types[$type])) {
            $update .= ' <span class="update-type">(' . $update_types[$type] . ')</span>';
          }
          $class = array('update-' . $type);

          $updates[] = array(
            'data' => array(
              $project['title'],
              $project_type,
              $update,
            ),
            'class' => $class,
            'weight' => $weight,
          );
        }
      }
    }

    if (empty($updates)) {
      $panel['up_to_date'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('messages', 'status')),
      );
      $panel['up_to_date']['message'] = array(
        '#markup' => t('Your site is up to date.'),
      );
    }

    backdrop_add_css(backdrop_get_path('module', 'installer') . '/css/installer.css');
    $last = state_get('update_last_check', 0);
    $panel['last_check'] = array(
      '#markup' => theme('update_last_check__block', array('last' => $last)),
    );

    if (!empty($updates)) {
      // Ensure that entries with security updates or unsupported versions
      // bubble up to the top of the table.
      backdrop_sort($updates);

      $panel['table'] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $updates,
      );
    }

    if (!empty($updates) && module_exists('installer') && user_access('administer software updates')) {
      $link_path = 'admin/config/system/updates';
      $link_text = t('Install system updates');
    }
    elseif (user_access('access_site_reports')) {
      $link_path = 'admin/reports/updates';
      $link_text = t('More info');
    }
    if (isset($link_path) && isset($link_text)) {
      $panel['link'] = array(
        '#theme' => 'link',
        '#path' => $link_path,
        '#text' => $link_text,
      );
    }

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
