<?php
/**
 * @file
 * Provide views data and handlers for user.module.
 *
 * @ingroup views_module_handlers
 */

/**
 * Implements hook_views_data().
 */
function user_views_data() {
  // ----------------------------------------------------------------
  // users table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['users']['table']['group']  = t('User account');

  $data['users']['table']['base'] = [
    'field' => 'uid',
    'title' => t('User account'),
    'help' => t('Users who have created accounts on your site.'),
    'access query tag' => 'user_access',
  ];
  $data['users']['table']['entity type'] = 'user';


  $data['users']['table']['default_relationship'] = [
    'node' => [
      'table' => 'node',
      'field' => 'uid',
    ],
    'node_revision' => [
      'table' => 'node_revision',
      'field' => 'uid',
    ],
    'file' => [
      'table' => 'file',
      'field' => 'uid',
    ],
  ];

  // uid
  $data['users']['uid'] = [
    'title' => t('Uid'),
    'help' => t('The user account ID.'), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name', // display this field in the summary
    ],
    'filter' => [
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'relationship' => [
      'title' => t('Content authored'),
      'help' => t('Relate content to the user account of its creator. This relationship will create one record for each content item created by the user account.'),
      'handler' => 'views_handler_relationship',
      'base' => 'node',
      'base field' => 'uid',
      'field' => 'uid',
      'label' => t('nodes'),
    ],
  ];

  // uid_raw
  $data['users']['uid_raw'] = [
    'help' => t('The raw numeric user account ID.'),
    'real field' => 'uid',
    'filter' => [
      'title' => t('The user account ID'),
      'handler' => 'views_handler_filter_numeric',
    ],
  ];

  // uid
  $data['users']['uid_representative'] = [
    'relationship' => [
      'title' => t('Representative node'),
      'label'  => t('Representative node'),
      'help' => t('Obtains a single representative node for each user account, according to a chosen sort criterion.'),
      'handler' => 'views_handler_relationship_groupwise_max',
      'relationship field' => 'uid',
      'outer field' => 'users.uid',
      'argument table' => 'users',
      'argument field' =>  'uid',
      'base'   => 'node',
      'field'  => 'nid',
    ],
  ];

  // uid
  $data['users']['uid_current'] = [
    'real field' => 'uid',
    'title' => t('Current'),
    'help' => t('Filter the view to the currently logged in user account.'),
    'filter' => [
      'handler' => 'views_handler_filter_user_current',
      'type' => 'yes-no',
    ],
  ];

  // name
  $data['users']['name'] = [
    'title' => t('Username'), // The item it appears as on the UI,
    'help' => t('The username for the user account or author.'), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_user_name',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
      'title' => t('Name (raw)'),
      'help' => t('The username for the user account or author. This filter does not check if the user account exists and allows partial matching. Does not utilize autocomplete.')
    ],
  ];

  // mail
  // Note that this field implements field level access control.
  $data['users']['mail'] = [
    'title' => t('Email'), // The item it appears as on the UI,
    'help' => t('Email address for a given user account. This field is normally not disclosed, so be cautious when using it.'), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_user_mail',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  // language
  $data['users']['language'] = [
    'title' => t('Language'), // The item it appears as on the UI,
    'help' => t('The language for the user account.'),
    'field' => [
      'handler' => 'views_handler_field_user_language',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_node_language',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_node_language',
    ],
  ];

  // picture
  $data['users']['picture_fid']['moved to'] = ['users', 'picture'];
  $data['users']['picture'] = [
    'title' => t('Picture'),
    'help' => t("The user account picture, if allowed."), // The help that appears on the UI,
    // Information for displaying the uid
    'field' => [
      'handler' => 'views_handler_field_user_picture',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Has avatar'),
      'type' => 'yes-no',
    ],
  ];

  // link
  $data['users']['view_user'] = [
    'field' => [
      'title' => t('Link'),
      'help' => t('Provide a simple link to the user account.'),
      'handler' => 'views_handler_field_user_link',
    ],
  ];

  // created field
  $data['users']['created'] = [
    'title' => t('Created date'), // The item it appears as on the UI,
    'help' => t('The date the user account was created.'), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  $data['users']['created_fulldate'] = [
    'title' => t('Created date'),
    'help' => t('Date in the form of CCYYMMDD.'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_fulldate',
    ],
  ];

  $data['users']['created_year_month'] = [
    'title' => t('Created year + month'),
    'help' => t('Date in the form of YYYYMM.'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year_month',
    ],
  ];

  $data['users']['timestamp_year']['moved to'] = ['users', 'created_year'];
  $data['users']['created_year'] = [
    'title' => t('Created year'),
    'help' => t('Date in the form of YYYY.'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year',
    ],
  ];

  $data['users']['created_month'] = [
    'title' => t('Created month'),
    'help' => t('Date in the form of MM (01 - 12).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_month',
    ],
  ];

  $data['users']['created_day'] = [
    'title' => t('Created day'),
    'help' => t('Date in the form of DD (01 - 31).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_day',
    ],
  ];

  $data['users']['created_week'] = [
    'title' => t('Created week'),
    'help' => t('Date in the form of WW (01 - 53).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_week',
    ],
  ];

  // Changed field.
  $data['users']['changed'] = [
    // The item it appears as on the UI.
    'title' => t('Changed date'),
    // The help that appears on the UI.
    'help' => t('The date the user was changed.'),
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  $data['users']['changed_fulldate'] = [
    'title' => t('Changed date'),
    'help' => t('Date in the form of CCYYMMDD.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_fulldate',
    ],
  ];

  $data['users']['changed_year_month'] = [
    'title' => t('Changed year + month'),
    'help' => t('Date in the form of YYYYMM.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_year_month',
    ],
  ];

  $data['users']['changed_year'] = [
    'title' => t('Changed year'),
    'help' => t('Date in the form of YYYY.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_year',
    ],
  ];

  $data['users']['changed_month'] = [
    'title' => t('Changed month'),
    'help' => t('Date in the form of MM (01 - 12).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_month',
    ],
  ];

  $data['users']['changed_day'] = [
    'title' => t('Changed day'),
    'help' => t('Date in the form of DD (01 - 31).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_day',
    ],
  ];

  $data['users']['changed_week'] = [
    'title' => t('Changed week'),
    'help' => t('Date in the form of WW (01 - 53).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_week',
    ],
  ];

  // access field
  $data['users']['access'] = [
    'title' => t('Last access'), // The item it appears as on the UI,
    'help' => t("The user account's last access date."), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  // login field
  $data['users']['login'] = [
    'title' => t('Last login'), // The item it appears as on the UI,
    'help' => t("The user account's last login date."), // The help that appears on the UI,
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  // active status
  $data['users']['status'] = [
    'title' => t('Active'), // The item it appears as on the UI,
    'help' => t('Whether a user account is active or blocked.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => [
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
      'output formats' => [
        'active-blocked' => [t('Active'), t('Blocked')],
      ],
    ],
    'filter' => [
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Active'),
      'type' => 'active-blocked',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // log field
  $data['users']['signature'] = [
    'title' => t('Signature'), // The item it appears as on the UI,
    'help' => t("The user account's signature."), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => [
      'handler' => 'views_handler_field_markup',
      'format' => filter_fallback_format(),
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
  ];

  $data['users']['edit_node'] = [
    'field' => [
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the user account.'),
      'handler' => 'views_handler_field_user_link_edit',
    ],
  ];

  $data['users']['cancel_node'] = [
    'field' => [
      'title' => t('Cancel link'),
      'help' => t('Provide a simple link to cancel the user account.'),
      'handler' => 'views_handler_field_user_link_cancel',
    ],
  ];

  $data['users']['data'] = [
    'title' => t('Data'),
    'help' => t('Provide serialized data of the user account'),
    'field' => [
      'handler' => 'views_handler_field_serialized',
    ],
  ];

  $data['users']['bulk_form'] = [
    'title' => t('Bulk operations'),
    'help' => t('Provides a checkbox on each row and a form for executing bulk actions on user accounts.'),
    'real field' => 'uid',
    'field' => [
      'handler' => 'views_handler_field_bulk_form',
      'click sortable' => FALSE,
    ],
  ];

  // ----------------------------------------------------------------------
  // users_roles table

  $data['users_roles']['table']['group']  = t('User account');

  // Explain how this table joins to others.
  $data['users_roles']['table']['join'] = [
     // Directly links to users table.
    'users' => [
      'left_field' => 'uid',
      'field' => 'uid',
    ],
  ];

  $data['users_roles']['table']['default_relationship'] = [
    'node' => [
      'table' => 'node',
      'field' => 'uid',
    ],
    'node_revision' => [
      'table' => 'node_revision',
      'field' => 'uid',
    ],
  ];

  $data['users_roles']['role'] = [
    'title' => t('Roles'),
    'help' => t('Roles that a user account has been assigned.'),
    'field' => [
      'handler' => 'views_handler_field_user_roles',
      'no group by' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_user_roles',
      'numeric' => TRUE,
      'allow empty' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_users_roles',
      'empty field name' => t('No role'),
      'zero is null' => TRUE,
      'numeric' => TRUE,
    ],
  ];

  $data['users_roles']['permissions'] = [
    'title' => t('Permissions'),
    'help' => t('The user account permissions.'),
    'real field' => 'role',
    'field' => [
      'handler' => 'views_handler_field_user_permissions',
      'no group by' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_user_permissions',
    ],
  ];

  return $data;
}

/**
 * Implements hook_views_plugins().
 */
function user_views_plugins() {
  return [
    'module' => 'views', // This just tells our themes are elsewhere.
    'row' => [
      'user' => [
        'title' => t('User accounts'),
        'help' => t('Display the user accounts with standard user account view.'),
        'handler' => 'views_plugin_row_user_view',
        'base' => ['users'], // only works with 'users' as base.
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-users',
      ],
    ],
    'argument default' => [
      'user' => [
        'title' => t('User account ID from URL'),
        'handler' => 'views_plugin_argument_default_user',
        'path' => backdrop_get_path('module', 'views') . '/modules/user', // not necessary for most modules
      ],
      'current_user' => [
        'title' => t('User account ID from logged in person'),
        'handler' => 'views_plugin_argument_default_current_user',
        'path' => backdrop_get_path('module', 'views') . '/modules/user', // not necessary for most modules
      ],
    ],
    'argument validator' => [
      'user' => [
        'title' => t('User account'),
        'handler' => 'views_plugin_argument_validate_user',
        'path' => backdrop_get_path('module', 'views') . '/modules/user', // not necessary for most modules
      ],
    ],
  ];
}

/**
 * Allow replacement of current user ID so we can cache these queries.
 */
function user_views_query_substitutions($view) {
  global $user;
  return ['***CURRENT_USER***' => intval($user->uid)];
}
