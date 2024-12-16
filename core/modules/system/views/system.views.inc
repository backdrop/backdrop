<?php
/**
 * @file
 * Provide views data and handlers for system.module.
 *
 * @ingroup views_module_handlers
 */

/**
 * Implements hook_views_data().
 */
function system_views_data() {
  $data = [];

  // ----------------------------------------------------------------------
  // system table
  $data['system']['table']['group']  = t('System');

  // Advertise this table as a possible base table
  $data['system']['table']['base'] = [
    'field' => 'filename',
    'title' => t('Project'),
    'help' => t('Modules/Themes/Theme engines in your codebase.'),
  ];

  // fields
  // - filename
  $data['system']['filename'] = [
    'title' => t('Project filename'),
    'help' => t('The path of the primary file for this module/theme/theme engine, relative to the Backdrop root; e.g. modules/node/node.module.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
      'name field' => 'filename', // the field to display in the summary.
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  // - name
  $data['system']['name'] = [
    'title' => t('Project name'),
    'help' => t('The name of the module/theme/theme engine; e.g. node.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
      'name field' => 'name', // the field to display in the summary.
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  // - type
  $data['system']['type'] = [
    'title' => t('Project type'),
    'help' => t('The type of the item, either module, theme, or theme_engine.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
      'name field' => 'type', // the field to display in the summary.
    ],
    'filter' => [
      'handler' => 'views_handler_filter_system_type',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  // - status
  $data['system']['status'] = [
    'title' => t('Project status'),
    'help' => t('Boolean indicating whether or not this module/theme/theme engine is enabled.'),
    'field' => [
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'status', // the field to display in the summary.
    ],
    'filter' => [
      'handler' => 'views_handler_filter_boolean_operator',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  // - schema version
  $data['system']['schema_version'] = [
    'title' => t('Schema version'),
    'help' => t("The module's database schema version number. -1 if the module is not installed (its tables do not exist); 0 or the largest N of the module's hook_update_N() function that has either been run or existed when the module was first installed."),
    'field' => [
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'schema_version', // the field to display in the summary.
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // - info
  $data['system']['info'] = [
    'title' => t('Project properties'),
    'help' => t('Selected properties from the module/theme/theme engine\'s info file.'),
    'field' => [
      'handler' => 'views_handler_field_system_info',
      'click sortable' => FALSE,
    ],
  ];  

  return $data;
}

function _views_file_status($choice = NULL) {
  $status = [
    0 => t('Temporary'),
    FILE_STATUS_PERMANENT => t('Permanent'),
  ];

  if (isset($choice)) {
    return isset($status[$choice]) ? $status[$choice] : t('Unknown');
  }

  return $status;
}
