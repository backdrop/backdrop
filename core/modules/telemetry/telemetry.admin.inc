<?php
/**
 * Administrative pages for Telemetry module.
 */

function telemetry_report() {
  $telemetry_info = telemetry_get_info();
  $telemetry_data = telemetry_get_data();

  $module_path = backdrop_get_path('module', 'telemetry');
  backdrop_add_css($module_path . '/css/telemetry.admin.css');
  $page = [
    '#type' => 'container',
    '#attributes' => ['class' => ['telemetry-report']],
  ];

  $help = '<p>' . t('Help us improve Backdrop CMS by sending anonymous, untracked information about what features your site uses.') . '</p>';
  $help .= '<p>' . t('Below is a list of all data collected by the enabled modules on your site. Only projects that are publicly available on BackdropCMS.org will store usage information.') . '</p>';
  $help .= '<p>' . t('If you prefer to not send usage information, you can disable the Telemetry module from the <a href="!modules">modules page</a>.', ['!modules' => url('admin/modules')]) . '</p>';

  $page['help'] = [
    '#type' => 'help',
    '#markup' => $help
  ];

  $header = [
    t('Metric'),
    ['data' => t('Description'), 'class' => ['priority-low']],
    t('Value'),
  ];

  $rows = [];
  foreach ($telemetry_info as $project => $project_data) {
    foreach ($project_data as $key => $info) {
      $row = [];
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
      $module_info['name'] = $info['module'] === 'telemetry' ? t('System') : $module_info['name'];

      $row[] = '<div class="telemetry-item-label">' . check_plain($info['label']) . '</div><div class="telemetry-item-project">' . check_plain($module_info['name']) . '</div>';
      $row[] = [
        'data' => '<div class="telemetry-item-description">' . check_plain($info['description']) . '</div>',
        'class' => ['priority-low'],
      ];
      $row[] = [
        'data' => $value,
        'class' => ['telemetry-item-value'],
      ];
      $rows[] = $row;
    }
  }

  $page['items'] = [
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#sticky' => FALSE,
  ];

  return $page;
}
