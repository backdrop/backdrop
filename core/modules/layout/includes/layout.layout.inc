<?php
/**
 * @file
 * Contains hook implementations Layout module provides for itself.
 */

/**
 * Implements hook_layout_context_info().
 *
 * @see node_layout_context_info().
 * @see user_layout_context_info().
 * @see taxonomy_layout_context_info().
 */
function layout_layout_context_info() {
  $info['node'] = [
    'title' => t('Node'),
    // Define the class which is used to handle this context.
    'class' => 'EntityLayoutContext',
    // Define menu paths where the node ID is a "known" context.
    'menu paths' => [
      'node/%node',
      'node/%node/view',
    ],
    // Given the menu paths defined above, identify the part of the path that
    // is needed to generate this context.
    'path placeholder' => '%node',

    // Given an argument, the callback that will be responsible for loading the
    // main context data.
    'load callback' => 'node_load',
  ];
  $info['user'] = [
    'title' => t('User account'),
    'class' => 'EntityLayoutContext',
    'menu paths' => [
      'user/%user',
      'user/%user/view',
    ],
    'path placeholder' => '%user',
    'load callback' => 'user_load',
  ];
  $info['taxonomy_term'] = [
    'title' => t('Term'),
    'class' => 'EntityLayoutContext',
    'menu paths' => [
      'taxonomy/term/%term',
      'taxonomy/term/%term/view',
    ],
    'path placeholder' => '%term',
    'load callback' => 'taxonomy_term_load',
  ];
  $info['file'] = [
    'title' => t('File'),
    'class' => 'EntityLayoutContext',
    'menu paths' => [
      'file/%file',
      'file/%file/view',
    ],
    'path placeholder' => '%file',
    'load callback' => 'file_load',
  ];
  $info['overrides_path'] = [
    'title' => t('Overrides path'),
    'description' => t('Layout overrides existing path provided by another module.'),
    'class' => 'LayoutOverridesPathContext',
    'path placeholder' => NULL,
    'menu paths' => [],
    'hidden' => TRUE,
  ];
  $info['string'] = [
    'title' => t('String pass-through'),
    'class' => 'LayoutStringContext',
    'path placeholder' => NULL,
    'menu paths' => [],
  ];
  return $info;
}

/**
 * Implements hook_layout_relationship_info().
 */
function layout_layout_relationship_info() {
  $info['author_from_node'] = [
    'title' => t('Author of content'),
    'class' => 'LayoutRelationshipAuthorFromNode',
    'context' => 'node',
    'context_label' => t('Content'),
    'description' => t('Creates a user account context based on the author of content.'),
  ];

  return $info;
}

/**
 * Implements hook_layout_access_info().
 */
function layout_layout_access_info() {
  $entity_types = entity_get_info();

  // Provide type-specific plugins for each entity type.
  foreach ($entity_types as $entity_type => $entity_info) {

    // Create a key such as "node_type" or "comment_node_type".
    // Skip users, which don't support multiple types.
    if (!empty($entity_info['entity keys']['bundle'])) {
      $key = $entity_type . '_' . $entity_info['entity keys']['bundle'];
      $bundle_label = array_key_exists('bundle label', $entity_info) ? $entity_info['bundle label'] : 'Type';
      $info[$key] = [
        'title' => t('@entity: @bundle', ['@entity' => $entity_info['label'], '@bundle' => $bundle_label]),
        'description' => t('Control access by @entity_type @bundle_label.', ['@entity_type' => $entity_type, '@bundle_label' => backdrop_strtolower($bundle_label)]),
        'entity_type' => $entity_type,
        'class' => 'EntityBundleLayoutAccess',
        'required contexts' => [
          $entity_type => $entity_type,
        ],
      ];
    }

    // Create a entity id key such as "nid" or "cid".
    $key = $entity_type . '_' . $entity_info['entity keys']['id'];
    $key_label = strtoupper($entity_info['entity keys']['id']);
    $info[$key] = [
      'title' => t('@entity: @entity_id_key', ['@entity' => $entity_info['label'], '@entity_id_key' => $key_label]),
      'description' => t('Control access by @entity_type @entity_id_key.', ['@entity_type' => $entity_type, '@entity_id_key' => $key_label]),
      'entity_type' => $entity_type,
      'class' => 'EntityIDLayoutAccess',
      'required contexts' => [
        $entity_type => $entity_type,
      ],
    ];
  }

  $info['front'] = [
    'title' => t('Home page'),
    'description' => t('Is this the home page.'),
    'class' => 'FrontLayoutAccess',
  ];
  $info['language'] = [
    'title' => t('Site language'),
    'description' => t('Control access by the currently active interface language.'),
    'class' => 'LanguageLayoutAccess',
  ];
  $info['user_permission'] = [
    'title' => t('User account: Permission'),
    'description' => t('Control access by permission string.'),
    'class' => 'UserPermissionLayoutAccess',
    'required contexts' => [
      'user' => 'user',
    ],
  ];
  $info['user_role'] = [
    'title' => t('User account: Role'),
    'description' => t('Control access by role.'),
    'class' => 'UserRoleLayoutAccess',
    // Contexts are specified as context key => context type. The key will be
    // used in the $contexts array passed to the access class methods. The type
    // references a context provided by hook_layout_context_info().
    'required contexts' => [
      'user' => 'user',
    ],
    // Optional if needing to clarify between contexts of the same type.
    'required contexts labels' => [
      'user' => t('User account'),
    ],
  ];
  $info['user_compare'] = [
    'title' => t('User account: Compare'),
    'description' => t('Compare two user accounts. For example: logged-in vs profile being viewed.'),
    'class' => 'UserCompareLayoutAccess',
    // Contexts are specified as context key => context type. The key will be
    // used in the $contexts array passed to the access class methods. The type
    // references a context provided by hook_layout_context_info().
    'required contexts' => [
      'user1' => 'user',
      'user2' => 'user',
    ],
    // Optional if needing to clarify between contexts of the same type.
    'required contexts labels' => [
      'user1' => t('The logged-in user account'),
      'user2' => t('User account to compare to the logged-in user account'),
    ],
  ];
  $info['path'] = [
    'title' => t('URL path'),
    'description' => t('Control access by the current path.'),
    'class' => 'PathLayoutAccess',
  ];
  $info['http_response_code'] = [
    'title' => t('Server response'),
    'description' => t('Control access by the HTTP response status code returned by the server.'),
    'class' => 'HttpResponseCodeLayoutAccess',
  ];
  return $info;
}

/**
 * Implements hook_layout_renderer_info().
 */
function layout_layout_renderer_info() {
  $info['broken'] = [
    'class' => 'LayoutRendererBroken',
  ];
  $info['editor'] = [
    'class' => 'LayoutRendererEditor',
  ];
  $info['standard'] = [
    'class' => 'LayoutRendererStandard',
  ];
  $info['flexible'] = [
    'class' => 'LayoutRendererFlexible',
  ];
  return $info;
}

/**
 * Implements hook_layout_style_info().
 */
function layout_layout_style_info() {
  $info['default'] = [
    'title' => t('Default'),
    'description' => t('The default block rendering style with predictable tags provided by the site template.'),
    'block theme' => 'block',
    'class' => 'LayoutStyle',
    'template' => 'templates/block',
    'file' => 'layout.theme.inc',
  ];
  $info['dynamic'] = [
    'title' => t('Dynamic'),
    'description' => t('Customizable rendering style in which every HTML tag can be configured.'),
    'block theme' => 'block_dynamic',
    'class' => 'LayoutStyleDynamic',
    'template' => 'templates/block-dynamic',
    'file' => 'layout.theme.inc',
  ];
  return $info;
}
