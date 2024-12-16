<?php
/**
 * @file
 * Provide views data and handlers for file.module.
 *
 * @ingroup views_module_handlers
 */

/**
 * Implements hook_field_views_data().
 */
function file_views_data() {
  $data = [];

  // ----------------------------------------------------------------------
  // file_managed table

  $data['files']['moved to'] = 'file_managed';
  $data['file_managed']['table']['group']  = t('File');

  // Advertise this table as a possible base table
  $data['file_managed']['table']['base'] = [
    'field' => 'fid',
    'title' => t('File'),
    'help' => t("Files maintained by Backdrop and various modules."),
    'defaults' => [
      'field' => 'filename'
    ],
  ];
  $data['file_managed']['table']['entity type'] = 'file';

  // Rendered file.
  $data['file_managed']['rendered'] = [
    'title' => t('Rendered'),
    'help' => t('Display the file in a specific view mode.'),
    'field' => [
      'handler' => 'views_handler_field_file_rendered',
      'click sortable' => TRUE,
      'real field' => 'fid',
      'additional fields' => [
        'fid',
      ],
    ],
  ];

  // fid
  $data['file_managed']['fid'] = [
    'title' => t('File ID'),
    'help' => t('The ID of the file.'),
    'field' => [
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_file_fid',
      'name field' => 'filename', // the field to display in the summary.
      'numeric' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // File type.
  $data['file_managed']['type'] = [
    'title' => t('Type'),
    'help' => t('The type of the file (for example, "audio", "image", "video", etc).'),
    'field' => [
      'handler' => 'views_handler_field_file_type',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_file_type',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_file_type',
    ],
  ];

  // filename
  $data['file_managed']['filename'] = [
    'title' => t('Name'),
    'help' => t('The name of the file.'),
    'field' => [
      'handler' => 'views_handler_field_file',
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

  // uri
  $data['file_managed']['uri'] = [
    'title' => t('Path'),
    'help' => t('The internal path (URI) of the file. May also display the file URL.'),
    'field' => [
      'handler' => 'views_handler_field_file_uri',
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

  // Edit link.
  $data['file_managed']['edit'] = [
    'title' => t('Manage link'),
    'help' => t('A link to edit the file entity.'),
    'field' => [
      'handler' => 'views_handler_field_file_link_edit',
      'real field' => 'fid',
      'additional fields' => [
        'fid',
      ],
    ],
  ];

  // Delete link.
  $data['file_managed']['delete'] = [
    'title' => t('Delete link'),
    'help' => t('Provide a simple link to delete the file entity.'),
    'field' => [
      'handler' => 'views_handler_field_file_link_delete',
      'real field' => 'fid',
      'additional fields' => [
        'fid',
      ],
    ],
  ];

  // filemime
  $data['file_managed']['filemime'] = [
    'title' => t('Mime type'),
    'help' => t('The mime type of the file.'),
    'field' => [
      'handler' => 'views_handler_field_file_filemime',
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

  // extension
  $data['file_managed']['extension'] = [
    'title' => t('Extension'),
    'help' => t('The extension of the file.'),
    'real field' => 'filename',
    'field' => [
      'handler' => 'views_handler_field_file_extension',
      'click sortable' => FALSE,
     ],
  ];

  // Icon based on file extension.
  $data['file_managed']['icon'] = [
    'title' => t('Icon'),
    'help' => t('An SVG icon according to file extension.'),
    'real field' => 'uri',
    'field' => [
      'handler' => 'views_handler_field_file_icon',
      'click sortable' => FALSE,
     ],
  ];

  // filesize
  $data['file_managed']['filesize'] = [
    'title' => t('Size'),
    'help' => t('The size of the file.'),
    'field' => [
      'handler' => 'views_handler_field_file_size',
      'click sortable' => TRUE,
     ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
  ];

  // status
  $data['file_managed']['status'] = [
    'title' => t('Status'),
    'help' => t('The status of the file.'),
    'field' => [
      'handler' => 'views_handler_field_file_status',
      'click sortable' => TRUE,
     ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_file_status',
    ],
  ];

  // timestamp field
  $data['file_managed']['timestamp'] = [
    'title' => t('Upload date'),
    'help' => t('The date the file was uploaded.'),
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

  // uid
  $data['file_managed']['uid'] = [
    'title' => t('User who uploaded'),
    'help' => t('The user account that uploaded the file. If more fields than UID are needed, add the "File: User who uploaded" relationship.'),
    'relationship' => [
      'title' => t('User who uploaded'),
      'label' => t('User who uploaded'),
      'base' => 'users',
      'base field' => 'uid',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_user_name',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
    'field' => [
      'handler' => 'views_handler_field_user',
    ],
  ];

  $data['file_managed']['bulk_form'] = [
    'title' => t('Bulk operations'),
    'help' => t('Provides a checkbox on each row and a form for executing bulk actions on files.'),
    'real field' => 'fid',
    'field' => [
      'handler' => 'views_handler_field_bulk_form',
      'click sortable' => FALSE,
    ],
  ];

  // ----------------------------------------------------------------------
  // file_usage table

  $data['file_usage']['table']['group']  = t('File Usage');

  // Provide field-type-things to several base tables; on the core files table ("file_managed") so
  // that we can create relationships from files to entities, and then on each core entity type base
  // table so that we can provide general relationships between entities and files.
  $data['file_usage']['table']['join'] = [
    // Link ourself to the {file_managed} table so we can provide file->entity relationships.
    'file_managed' => [
      'field' => 'fid',
      'left_field' => 'fid',
    ],
    // Link ourself to the {node} table so we can provide node->file relationships.
    'node' => [
      'field' => 'id',
      'left_field' => 'nid',
      'extra' => [['field' => 'type', 'value' => 'node']],
    ],
    // Link ourself to the {users} table so we can provide user->file relationships.
    'users' => [
      'field' => 'id',
      'left_field' => 'uid',
      'extra' => [['field' => 'type', 'value' => 'user']],
    ],
    // Link ourself to the {comment} table so we can provide comment->file relationships.
    'comment' => [
      'field' => 'id',
      'left_field' => 'cid',
      'extra' => [['field' => 'type', 'value' => 'comment']],
    ],
    // Link ourself to the {taxonomy_term_data} table so we can provide taxonomy_term->file relationships.
    'taxonomy_term_data' => [
      'field' => 'id',
      'left_field' => 'tid',
      'extra' => [['field' => 'type', 'value' => 'taxonomy_term']],
    ],
  ];

  // Provide a relationship between the files table and each entity type, and between each entity
  // type and the files table. Entity->file relationships are type-restricted in the joins
  // declared above, and file->entity relationships are type-restricted in the relationship
  // declarations below.

  // Relationships between files and nodes.
  $data['file_usage']['file_to_node'] = [
    'title' => t('Content'),
    'help' => t('Content that is associated with this file, usually because this file is in a field on the content.'),
    // Only provide this field/relationship/etc. when the 'file_managed' base table is present.
    'skip base' => ['node', 'node_revision', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'id',
    'relationship' => [
      'title' => t('Content'),
      'label' => t('Content'),
      'base' => 'node',
      'base field' => 'nid',
      'relationship field' => 'id',
      'extra' => [['table' => 'file_usage', 'field' => 'type', 'operator' => '=', 'value' => 'node']],
    ],
  ];
  $data['file_usage']['node_to_file'] = [
    'title' => t('File'),
    'help' => t('A file that is associated with this node, usually because it is in a field on the node.'),
    // Only provide this field/relationship/etc. when the 'node' base table is present.
    'skip base' => ['file_managed', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'fid',
    'relationship' => [
      'title' => t('File'),
      'label' => t('File'),
      'base' => 'file_managed',
      'base field' => 'fid',
      'relationship field' => 'fid',
    ],
  ];

  // Relationships between files and users.
  $data['file_usage']['file_to_user'] = [
    'title' => t('User'),
    'help' => t('A user that is associated with this file, usually because this file is in a field on the user.'),
    // Only provide this field/relationship/etc. when the 'file_managed' base table is present.
    'skip base' => ['node', 'node_revision', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'id',
    'relationship' => [
      'title' => t('User'),
      'label' => t('User'),
      'base' => 'users',
      'base field' => 'uid',
      'relationship field' => 'id',
      'extra' => [['table' => 'file_usage', 'field' => 'type', 'operator' => '=', 'value' => 'user']],
    ],
  ];
  $data['file_usage']['user_to_file'] = [
    'title' => t('File'),
    'help' => t('A file that is associated with this user, usually because it is in a field on the user.'),
    // Only provide this field/relationship/etc. when the 'users' base table is present.
    'skip base' => ['file_managed', 'node', 'node_revision', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'fid',
    'relationship' => [
      'title' => t('File'),
      'label' => t('File'),
      'base' => 'file_managed',
      'base field' => 'fid',
      'relationship field' => 'fid',
    ],
  ];

  // Relationships between files and comments.
  $data['file_usage']['file_to_comment'] = [
    'title' => t('Comment'),
    'help' => t('A comment that is associated with this file, usually because this file is in a field on the comment.'),
    // Only provide this field/relationship/etc. when the 'file_managed' base table is present.
    'skip base' => ['node', 'node_revision', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'id',
    'relationship' => [
      'title' => t('Comment'),
      'label' => t('Comment'),
      'base' => 'comment',
      'base field' => 'cid',
      'relationship field' => 'id',
      'extra' => [['table' => 'file_usage', 'field' => 'type', 'operator' => '=', 'value' => 'comment']],
    ],
  ];
  $data['file_usage']['comment_to_file'] = [
    'title' => t('File'),
    'help' => t('A file that is associated with this comment, usually because it is in a field on the comment.'),
    // Only provide this field/relationship/etc. when the 'comment' base table is present.
    'skip base' => ['file_managed', 'node', 'node_revision', 'users', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'fid',
    'relationship' => [
      'title' => t('File'),
      'label' => t('File'),
      'base' => 'file_managed',
      'base field' => 'fid',
      'relationship field' => 'fid',
    ],
  ];

  // Relationships between files and taxonomy_terms.
  $data['file_usage']['file_to_taxonomy_term'] = [
    'title' => t('Taxonomy Term'),
    'help' => t('A taxonomy term that is associated with this file, usually because this file is in a field on the taxonomy term.'),
    // Only provide this field/relationship/etc. when the 'file_managed' base table is present.
    'skip base' => ['node', 'node_revision', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'id',
    'relationship' => [
      'title' => t('Taxonomy Term'),
      'label' => t('Taxonomy Term'),
      'base' => 'taxonomy_term_data',
      'base field' => 'tid',
      'relationship field' => 'id',
      'extra' => [['table' => 'file_usage', 'field' => 'type', 'operator' => '=', 'value' => 'taxonomy_term']],
    ],
  ];
  $data['file_usage']['taxonomy_term_to_file'] = [
    'title' => t('File'),
    'help' => t('A file that is associated with this taxonomy term, usually because it is in a field on the taxonomy term.'),
    // Only provide this field/relationship/etc. when the 'taxonomy_term_data' base table is present.
    'skip base' => ['file_managed', 'node', 'node_revision', 'users', 'comment', 'taxonomy_vocabulary'],
    'real field' => 'fid',
    'relationship' => [
      'title' => t('File'),
      'label' => t('File'),
      'base' => 'file_managed',
      'base field' => 'fid',
      'relationship field' => 'fid',
    ],
  ];

  // Relationships between files and taxonomy_vocabulary items.
  $data['file_usage']['file_to_taxonomy_vocabulary'] = [
    'title' => t('Taxonomy Vocabulary'),
    'help' => t('A taxonomy vocabulary that is associated with this file, usually because this file is in a field on the taxonomy vocabulary.'),
    // Only provide this field/relationship/etc. when the 'file_managed' base table is present.
    'skip base' => ['node', 'node_revision', 'users', 'comment', 'taxonomy_term_data', 'taxonomy_vocabulary'],
    'real field' => 'id',
    'relationship' => [
      'title' => t('Taxonomy Vocabulary'),
      'label' => t('Taxonomy Vocabulary'),
      'base' => 'taxonomy_vocabulary',
      'base field' => 'vid',
      'relationship field' => 'id',
      'extra' => [['table' => 'file_usage', 'field' => 'type', 'operator' => '=', 'value' => 'taxonomy_vocabulary']],
    ],
  ];
  $data['file_usage']['taxonomy_vocabulary_to_file'] = [
    'title' => t('File'),
    'help' => t('A file that is associated with this taxonomy vocabulary, usually because it is in a field on the taxonomy vocabulary.'),
    // Only provide this field/relationship/etc. when the 'taxonomy_vocabulary' base table is present.
    'skip base' => ['file_managed', 'node', 'node_revision', 'users', 'comment', 'taxonomy_term_data'],
    'real field' => 'fid',
    'relationship' => [
      'title' => t('File'),
      'label' => t('File'),
      'base' => 'file_managed',
      'base field' => 'fid',
      'relationship field' => 'fid',
    ],
  ];

  // Provide basic fields from the {file_usage} table to all of the base tables we've declared
  // joins to (because there is no 'skip base' property on these fields).
  $data['file_usage']['module'] = [
    'title' => t('Module'),
    'help' => t('The module managing this file relationship.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  $data['file_usage']['type'] = [
    'title' => t('Entity type'),
    'help' => t('The type of entity that is related to the file.'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  $data['file_usage']['id'] = [
    'title' => t('Entity ID'),
    'help' => t('The ID of the entity that is related to the file.'),
    'field' => [
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];
  $data['file_usage']['count'] = [
    'title' => t('Use count'),
    'help' => t('The number of times the file is used by this entity.'),
    'field' => [
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  return $data;
}

/**
 * Implements hook_field_views_data().
 *
 * Views integration for file fields. Adds a file relationship to the default
 * field data.
 *
 * @see field_views_field_default_views_data()
 */
function file_field_views_data($field) {
  $data = field_views_field_default_views_data($field);
  foreach ($data as $table_name => $table_data) {
    // Add the relationship only on the fid field.
    $data[$table_name][$field['field_name'] . '_fid']['relationship'] = [
      'handler' => 'views_handler_relationship',
      'base' => 'file_managed',
      'entity type' => 'file',
      'base field' => 'fid',
      'label' => t('file from !field_name', ['!field_name' => $field['field_name']]),
    ];
  }

  return $data;
}

/**
 * Implements hook_field_views_data_views_data_alter().
 *
 * Views integration to provide reverse relationships on file fields.
 */
function file_field_views_data_views_data_alter(&$data, $field) {
  foreach ($field['bundles'] as $entity_type => $bundles) {
    $entity_info = entity_get_info($entity_type);
    $pseudo_field_name = 'reverse_' . $field['field_name'] . '_' . $entity_type;

    list($label, $all_labels) = field_views_field_label($field['field_name']);
    $entity = $entity_info['label'];
    if ($entity == t('Node')) {
      $entity = t('Content');
    }

    $data['file_managed'][$pseudo_field_name]['relationship'] = [
      'title' => t('@entity using @field', ['@entity' => $entity, '@field' => $label]),
      'help' => t('Relate each @entity with a @field set to the file.', ['@entity' => $entity, '@field' => $label]),
      'handler' => 'views_handler_relationship_entity_reverse',
      'field_name' => $field['field_name'],
      'field table' => _field_sql_storage_tablename($field),
      'field field' => $field['field_name'] . '_fid',
      'base' => $entity_info['base table'],
      'base field' => $entity_info['entity keys']['id'],
      'label' => t('!field_name', ['!field_name' => $field['field_name']]),
      'join_extra' => [
        0 => [
          'field' => 'entity_type',
          'value' => $entity_type,
        ],
        1 => [
          'field' => 'deleted',
          'value' => 0,
          'numeric' => TRUE,
        ],
      ],
    ];
  }
}
