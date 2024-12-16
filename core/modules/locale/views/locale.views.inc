<?php
/**
 * @file
 * Provides views data and handlers for locale.module.
 *
 * @ingroup views_module_handlers
 */

/**
 * Implements hook_views_data().
 */
function locale_views_data() {
  // Basic table information.

  // Define the base group of this table.
  $data['locales_source']['table']['group']  = t('Locale source');

  // Advertise this table as a possible base table.
  $data['locales_source']['table']['base'] = [
    'field' => 'lid',
    'title' => t('Locale source'),
    'help' => t('A source string for translation, in English or the default site language.'),
  ];

  // lid
  $data['locales_source']['lid'] = [
    'title' => t('LID'),
    'help' => t('The ID of the source string.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'lid',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // location
  $data['locales_source']['location'] = [
    'group' => t('Locale source'),
    'title' => t('Location'),
    'help' => t('A description of the location or context of the string.'),
    'field' => [
      'handler' => 'views_handler_field',
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

  // Source field
  $data['locales_source']['source'] = [
    'group' => t('Locale source'),
    'title' => t('Source'),
    'help' => t('The full original string.'),
    'field' => [
      'handler' => 'views_handler_field',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
  ];

  // Version field
  $data['locales_source']['version'] = [
    'group' => t('Locale source'),
    'title' => t('Version'),
    'help' => t('The version of Backdrop core that this string is for.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_locale_version',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  $data['locales_source']['edit_lid'] = [
    'group' => t('Locale source'),
    'field' => [
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the translations.'),
      'handler' => 'views_handler_field_locale_link_edit',
    ],
  ];

  // ----------------------------------------------------------------------
  // Locales target table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['locales_target']['table']['group']  = t('Locale target');

  // Join information
  $data['locales_target']['table']['join'] = [
    'locales_source' => [
      'left_field' => 'lid',
      'field' => 'lid',
    ],
  ];

  // Translation field
  $data['locales_target']['translation'] = [
    'group' => t('Locale target'),
    'title' => t('Translation'),
    'help' => t('The full translation string.'),
    'field' => [
      'handler' => 'views_handler_field',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
  ];

  // Language field
  $data['locales_target']['language'] = [
    'group' => t('Locale target'),
    'title' => t('Language'),
    'help' => t('The language this translation is in.'),
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

  $data['locales_target']['plid'] = [
    'group' => t('Locale target'),
    'title' => t('Singular LID'),
    'help' => t('The ID of the parent translation.'),
    'field' => [
      'handler' => 'views_handler_field',
    ],
  ];

  // Plural
  $data['locales_target']['plural'] = [
    'group' => t('Locale target'),
    'title' => t('Plural'),
    'help' => t('Whether or not the translation is plural.'),
    'field' => [
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Plural'),
      'type' => 'yes-no',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function locale_views_data_alter(&$data) {
  // Node language field.
  $data['node']['langcode'] = [
    'title' => t('Language'),
    'help' => t('The language the content is in.'),
    'field' => [
      'handler' => 'views_handler_field_node_language',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_node_language',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_node_language',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // Term language field.
  $data['taxonomy_term_data']['langcode'] = [
    'title' => t('Language'),
    'help' => t('The language the term is in.'),
    'field' => [
      'handler' => 'views_handler_field_term_language',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_term_language',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_term_language',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
}
