<?php
/**
 * Administrative pages for Telemetry module.
 */

function telemetry_report() {
  $telemetry_info = telemetry_get_info();
  $telemetry_data = telemetry_get_data();

  $module_path = backdrop_get_path('module', 'telemetry');
  backdrop_add_css($module_path . '/css/telemetry.admin.css');
  $page = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('telemetry-report')),
  );

  $help = '';
  $help .= '<p>' . t('Help us improve Backdrop CMS by sending anonymous, untracked information about what features your site uses.') . '</p>';
  $help .= '<p>' . t('Below is a list of all data collected by the enabled modules on your site. Only projects that are publicly available on BackdropCMS.org will store usage information.') . '</p>';
  $help .= '<p>' . t('If you prefer to not send usage information, you can disable the Telemetry module from the <a href="!modules">modules page</a>.', array('!modules' => url('admin/modules'))) . '</p>';

  $page['help'] = array(
    '#type' => 'help',
    '#markup' => $help
  );

  $header = array(
    t('Metric'),
    array('data' => t('Description'), 'class' => array('priority-low')),
    t('Value'),
  );

  $rows = array();
  foreach ($telemetry_info as $project => $project_data) {
    foreach ($project_data as $key => $info) {
      $row = array();
      if (!isset($telemetry_data[$project][$key])) {
        $value = '<span class="error">Missing value</span>';
      }
      else {
        $value = check_plain($telemetry_data[$project][$key]);
      }

      // Convert module name to readable label.
      $module_info = system_get_info('module', $info['module']);

      // Data directly recorded by Telemetry module is reported as "System".
      // This keeps Telemetry data collection all in one module for now.
      // Perhaps remove this conversion once more modules start collecting data.
      $module_info['name'] = $info['module'] === 'telemetry' ? t('System') : $info['label'];

      $row[] = '<div class="telemetry-item-label">' . check_plain($info['label']) . '</div><div class="telemetry-item-project">' . check_plain($module_info['name']) . '</div>';
      $row[] = array(
        'data' => '<div class="telemetry-item-description">' . check_plain($info['description']) . '</div>',
        'class' => array('priority-low'),
      );
      $row[] = array(
        'data' => $value,
        'class' => array('telemetry-item-value'),
      );
      $rows[] = $row;
    }
  }

  $page['items'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#sticky' => FALSE,
  );

  return $page;
}
