<?php
/**
 * @file
 * Views integration and data for the redirect module.
 */

/**
 * Implements hook_views_data().
 */
function redirect_views_data() {
  // Basic table information.
  $data['redirect']['table']['group'] = t('Redirect');

  // Advertise this table as a possible base table
  $data['redirect']['table']['base'] = [
    'field' => 'rid',
    'title' => t('URL redirects'),
    'help' => t('Listings of URL redirects.'),
    'weight' => 10,
  ];

  // {redirect}.rid
  $data['redirect']['rid'] = [
    'title' => t('Redirect ID'),
    'help' => t('The internal ID of the redirect.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  // {redirect}.type
  $data['redirect']['type'] = [
    'title' => t('Type'),
    'help' => t('The type of redirect.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'redirect_handler_filter_redirect_type',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  // {redirect}.uid
  $data['users']['table']['join']['redirect'] = [
    'left_field' => 'uid',
    'field' => 'uid',
  ];
  $data['redirect']['uid'] = [
    'title' => t('User ID'),
    'help' => t('ID of user who created the URL redirect.'),
    'field' => [
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name',
    ],
    'filter' => [
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'relationship' => [
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'base field' => 'uid',
      'label' => t('user'),
    ],
  ];
  $data['redirect']['uid_current'] = [
    'real field' => 'uid',
    'title' => t('Current user'),
    'help' => t('Filter the view to the currently logged in user.'),
    'filter' => [
      'handler' => 'views_handler_filter_user_current',
      'type' => 'yes-no',
    ],
  ];

  // {redirect}.source
  $data['redirect']['source'] = [
    'title' => t('Source URL'),
    'help' => t('The source URL which generates a redirect'),
    'field' => [
      'handler' => 'redirect_handler_field_redirect_source',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  // {redirect}.redirect
  $data['redirect']['redirect'] = [
    'title' => t('Redirect URL'),
    'help' => t('The destination URL'),
    'field' => [
      'handler' => 'redirect_handler_field_redirect_redirect',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  // Language field
  if (module_exists('locale')) {
    $data['redirect']['langcode'] = [
      'title' => t('Language'),
      'help' => t('The language the redirect is for.'),
      'field' => [
        'handler' => 'views_handler_field_locale_language',
        'click sortable' => TRUE,
      ],
      'filter' => [
        'handler' => 'views_handler_filter_locale_language',
      ],
      'argument' => [
        'handler' => 'views_handler_argument_locale_language',
      ],
    ];
  }

  // {redirect}.count
  $data['redirect']['count'] = [
    'title' => t('Clicks'),
    'help' => t('Number of times this URL redirect has been followed.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  // {redirect}.access
  $data['redirect']['access'] = [
    'title' => t('Last accessed date'),
    'help' => t('The date/time the URL redirect was last accessed.'),
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  $data['redirect']['operations'] = [
    'field' => [
      'title' => t('Operations'),
      'help' => t('Provide links to operations the user can perform for the URL redirect.'),
      'handler' => 'redirect_handler_field_redirect_operations',
    ],
  ];

  $data['redirect']['edit_redirect'] = [
    'field' => [
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the URL redirect.'),
      'handler' => 'redirect_handler_field_redirect_link_edit',
    ],
  ];

  $data['redirect']['delete_redirect'] = [
    'field' => [
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the URL redirect.'),
      'handler' => 'redirect_handler_field_redirect_link_delete',
    ],
  ];

  return $data;
}
