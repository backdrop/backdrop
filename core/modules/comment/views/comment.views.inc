<?php
/**
 * @file
 * Provide views data and handlers for comment.module.
 *
 * @ingroup views_module_handlers
 */

/**
 * Implements hook_views_data().
 */

function comment_views_data() {
  $data['comments']['moved to'] = 'comment';
  $data['comment']['comment']['moved to'] = ['field_data_comment_body', 'comment_body'];
  $data['comment']['comment']['field']['moved to'] = ['field_data_comment_body', 'comment_body'];
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['comment']['table']['group']  = t('Comment');

  $data['comment']['table']['base'] = [
    'field' => 'cid',
    'title' => t('Comment'),
    'help' => t("Comments are responses to node content."),
    'access query tag' => 'comment_access',
  ];
  $data['comment']['table']['entity type'] = 'comment';

  // Provide a "default relationship" to keep older views from choking.
  $data['comment']['table']['default_relationship'] = [
    'node' => [
      'table' => 'node',
      'field' => 'cid',
    ],
  ];

  // ----------------------------------------------------------------
  // Fields

  // subject
  $data['comment']['subject'] = [
    'title' => t('Title'),
    'help' => t('The title of the comment.'),
    'field' => [
      'handler' => 'views_handler_field_comment',
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

  // cid
  $data['comment']['cid'] = [
    'title' => t('ID'),
    'help' => t('The comment ID of the field'),
    'field' => [
      'handler' => 'views_handler_field_comment',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  // name (of comment author)
  $data['comment']['name'] = [
    'title' => t('Author'),
    'help' => t("The name of the comment's author. Can be rendered as a link to the author's homepage."),
    'field' => [
      'handler' => 'views_handler_field_comment_username',
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

  // homepage
  $data['comment']['homepage'] = [
    'title' => t("Author's website"),
    'help' => t("The website address of the comment's author. Can be rendered as a link. Will be empty if the author is a registered user."),
    'field' => [
      'handler' => 'views_handler_field_url',
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

  // hostname
  $data['comment']['hostname'] = [
    'title' => t('Hostname'),
    'help' => t('Hostname of user that posted the comment.'),
    'field' => [
      'handler' => 'views_handler_field',
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

  // mail
  $data['comment']['mail'] = [
    'title' => t('Mail'),
    'help' => t('Email of user that posted the comment. Will be empty if the author is a registered user.'),
    'field' => [
      'handler' => 'views_handler_field',
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

  // created (when comment was posted)
  $data['comment']['created'] = [
    'title' => t('Post date'),
    'help' => t('Date and time of when the comment was created.'),
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

  $data['comment']['changed_fulldata'] = [
    'title' => t('Post date'),
    'help' => t('Date in the form of CCYYMMDD.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_fulldate',
    ],
  ];

  $data['comment']['created_year_month'] = [
    'title' => t('Created year + month'),
    'help' => t('Date in the form of YYYYMM.'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year_month',
    ],
  ];

  $data['comment']['created_year'] = [
    'title' => t('Created year'),
    'help' => t('Date in the form of YYYY.'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year',
    ],
  ];

  $data['comment']['created_month'] = [
    'title' => t('Created month'),
    'help' => t('Date in the form of MM (01 - 12).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_month',
    ],
  ];

  $data['comment']['created_day'] = [
    'title' => t('Created day'),
    'help' => t('Date in the form of DD (01 - 31).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_day',
    ],
  ];

  $data['comment']['created_week'] = [
    'title' => t('Created week'),
    'help' => t('Date in the form of WW (01 - 53).'),
    'argument' => [
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_week',
    ],
  ];

  // Language field
  if (module_exists('locale')) {
    $data['comment']['language'] = [
      'title' => t('Language'),
      'help' => t('The language the comment is in.'),
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
      'sort' => [
        'handler' => 'views_handler_sort',
      ],
    ];
  }

  $data['comments']['timestamp']['moved to'] = ['comment', 'changed'];
  // changed (when comment was last updated)
  $data['comment']['changed'] = [
    'title' => t('Updated date'),
    'help' => t('Date and time of when the comment was last updated.'),
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

  $data['comments']['timestamp_fulldate']['moved to'] = ['comment', 'changed_fulldata'];
  $data['comment']['changed_fulldata'] = [
    'title' => t('Updated date'),
    'help' => t('Date in the form of CCYYMMDD.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_fulldate',
    ],
  ];

  $data['comments']['timestamp_year_month']['moved to'] = ['comment', 'changed_year_month'];
  $data['comment']['changed_year_month'] = [
    'title' => t('Updated year + month'),
    'help' => t('Date in the form of YYYYMM.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_year_month',
    ],
  ];

  $data['comments']['timestamp_year']['moved to'] = ['comment', 'changed_year'];
  $data['comment']['changed_year'] = [
    'title' => t('Updated year'),
    'help' => t('Date in the form of YYYY.'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_year',
    ],
  ];

  $data['comments']['timestamp_month']['moved to'] = ['comment', 'changed_month'];
  $data['comment']['changed_month'] = [
    'title' => t('Updated month'),
    'help' => t('Date in the form of MM (01 - 12).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_month',
    ],
  ];

  $data['comments']['timestamp_day']['moved to'] = ['comment', 'changed_day'];
  $data['comment']['changed_day'] = [
    'title' => t('Updated day'),
    'help' => t('Date in the form of DD (01 - 31).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_day',
    ],
  ];

  $data['comments']['timestamp_week']['moved to'] = ['comment', 'changed_week'];
  $data['comment']['changed_week'] = [
    'title' => t('Updated week'),
    'help' => t('Date in the form of WW (01 - 53).'),
    'argument' => [
      'field' => 'changed',
      'handler' => 'views_handler_argument_node_created_week',
    ],
  ];

  // status (approved or not)
  $data['comment']['status'] = [
    'title' => t('Approved'),
    'help' => t('Whether the comment is approved (or still in the moderation queue).'),
    'field' => [
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
      'output formats' => [
        'approved-not-approved' => [t('Approved'), t('Not Approved')],
      ],
    ],
    'filter' => [
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Approved comment'),
      'type' => 'yes-no',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
  ];

  // link to view comment
  $data['comment']['view_comment'] = [
    'field' => [
      'title' => t('View link'),
      'help' => t('Provide a simple link to view the comment.'),
      'handler' => 'views_handler_field_comment_link',
    ],
  ];

  // link to edit comment
  $data['comment']['edit_comment'] = [
    'field' => [
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the comment.'),
      'handler' => 'views_handler_field_comment_link_edit',
    ],
  ];

  // link to delete comment
  $data['comment']['delete_comment'] = [
    'field' => [
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the comment.'),
      'handler' => 'views_handler_field_comment_link_delete',
    ],
  ];


  // link to approve comment
  $data['comment']['approve_comment'] = [
    'field' => [
      'title' => t('Approve link'),
      'help' => t('Provide a simple link to approve the comment.'),
      'handler' => 'views_handler_field_comment_link_approve',
    ],
  ];

  // link to reply to comment
  $data['comment']['replyto_comment'] = [
    'field' => [
      'title' => t('Reply-to link'),
      'help' => t('Provide a simple link to reply to the comment.'),
      'handler' => 'views_handler_field_comment_link_reply',
    ],
  ];

  $data['comment']['thread'] = [
    'field' => [
      'title' => t('Depth'),
      'help' => t('Display the depth of the comment if it is threaded.'),
      'handler' => 'views_handler_field_comment_depth',
    ],
    'sort' => [
      'title' => t('Thread'),
      'help' => t('Sort by the threaded order. This will keep child comments together with their parents.'),
      'handler' => 'views_handler_sort_comment_thread',
    ],
  ];

  $data['comment']['nid'] = [
    'title' => t('Nid'),
    'help' => t('The node ID to which the comment is a reply to.'),
    'relationship' => [
      'title' => t('Content'),
      'help' => t('The content to which the comment is a reply to.'),
      'base' => 'node',
      'base field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Content'),
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
    'field' => [
      'handler' => 'views_handler_field_numeric',
    ],
  ];

  $data['comment']['uid'] = [
    'title' => t('Author uid'),
    'help' => t('If you need more fields than the uid add the comment: author relationship'),
    'relationship' => [
      'title' => t('Author'),
      'help' => t("The User ID of the comment's author."),
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('author'),
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
    'field' => [
      'handler' => 'views_handler_field_user',
    ],
  ];

  $data['comment']['pid'] = [
    'title' => t('Parent CID'),
    'help' => t('The Comment ID of the parent comment.'),
    'field' => [
      'handler' => 'views_handler_field',
    ],
    'relationship' => [
      'title' => t('Parent comment'),
      'help' => t('The parent comment.'),
      'base' => 'comment',
      'base field' => 'cid',
      'handler' => 'views_handler_relationship',
      'label' => t('Parent comment'),
    ],
  ];

  // ----------------------------------------------------------------------
  // node_comment_statistics table

  // define the group
  $data['node_comment_statistics']['table']['group']  = t('Content');

  // joins
  $data['node_comment_statistics']['table']['join'] = [
    //...to the node table
    'node' => [
      'type' => 'INNER',
      'left_field' => 'nid',
      'field' => 'nid',
     ],
  ];

  // last_comment_timestamp
  $data['node_comment_statistics']['last_comment_timestamp'] = [
    'title' => t('Last comment time'),
    'help' => t('Date and time of when the last comment was posted.'),
    'field' => [
      'handler' => 'views_handler_field_last_comment_timestamp',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  // last_comment_name (author's name)
  $data['node_comment_statistics']['last_comment_name'] = [
    'title' => t("Last comment author"),
    'help' => t('The name of the author of the last posted comment.'),
    'field' => [
      'handler' => 'views_handler_field_ncs_last_comment_name',
      'click sortable' => TRUE,
      'no group by' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_ncs_last_comment_name',
      'no group by' => TRUE,
    ],
  ];

  // comment_count
  $data['node_comment_statistics']['comment_count'] = [
    'title' => t('Comment count'),
    'help' => t('The number of comments a node has.'),
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
    'argument' => [
      'handler' => 'views_handler_argument',
    ],
  ];

  // last_comment_timestamp
  $data['node_comment_statistics']['last_updated'] = [
    'title' => t('Updated/commented date'),
    'help' => t('The most recent of last comment posted or node updated time.'),
    'field' => [
      'handler' => 'views_handler_field_ncs_last_updated',
      'click sortable' => TRUE,
      'no group by' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_ncs_last_updated',
      'no group by' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_ncs_last_updated',
    ],
  ];

  $data['node_comment_statistics']['cid'] = [
    'title' => t('Last comment CID'),
    'help' => t('Display the last comment of a node'),
    'relationship' => [
      'title' => t('Last Comment'),
      'help' => t('The last comment of a node.'),
      'group' => t('Comment'),
      'base' => 'comment',
      'base field' => 'cid',
      'handler' => 'views_handler_relationship',
      'label' => t('Last Comment'),
    ],
  ];

  // last_comment_uid
  $data['node_comment_statistics']['last_comment_uid'] = [
    'title' => t('Last comment uid'),
    'help' => t('The User ID of the author of the last comment of a node.'),
    'relationship' => [
      'title' => t('Last comment author'),
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('Last comment author'),
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
    'field' => [
      'handler' => 'views_handler_field_numeric',
    ],
  ];

  $data['comment']['bulk_form'] = [
    'title' => t('Bulk operations'),
    'help' => t('Provides a checkbox on each row and a form for executing bulk actions on comments.'),
    'real field' => 'cid',
    'field' => [
      'handler' => 'views_handler_field_bulk_form',
      'click sortable' => FALSE,
    ],
  ];

  return $data;
}

/**
 * Use views_data_alter to add items to the node table that are
 * relevant to comments.
 */
function comment_views_data_alter(&$data) {
  // new comments
  $data['node']['new_comments'] = [
    'title' => t('New comments'),
    'help' => t('The number of new comments on the node.'),
    'field' => [
      'handler' => 'views_handler_field_node_new_comments',
      'no group by' => TRUE,
    ],
  ];

  $data['node']['comments_link'] = [
    'field' => [
      'title' => t('Add comment link'),
      'help' => t('Display the standard add comment link used on regular nodes, which will only display if the viewing user has access to add a comment.'),
      'handler' => 'views_handler_field_comment_node_link',
    ],
  ];

  // Comment status of the node
  $data['node']['comment'] = [
    'title' => t('Comment status'),
    'help' => t('Whether comments are enabled or disabled on the node.'),
    'field' => [
      'handler' => 'views_handler_field_node_comment',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_node_comment',
    ],
  ];

  $data['node']['uid_touch'] = [
    'title' => t('User posted or commented'),
    'help' => t('Display nodes only if a user posted the node or commented on the node.'),
    'argument' => [
      'field' => 'uid',
      'name table' => 'users',
      'name field' => 'name',
      'handler' => 'views_handler_argument_comment_user_uid',
      'no group by' => TRUE,
    ],
    'filter' => [
      'field' => 'uid',
      'name table' => 'users',
      'name field' => 'name',
      'handler' => 'views_handler_filter_comment_user_uid'
    ],
  ];

  $data['node']['cid'] = [
    'title' => t('Comments of the node'),
    'help' => t('Relate all comments on the node. This will create 1 duplicate record for every comment. Usually if you need this it is better to create a comment view.'),
    'relationship' => [
      'group' => t('Comment'),
      'label' => t('Comments'),
      'base' => 'comment',
      'base field' => 'nid',
      'relationship field' => 'nid',
      'handler' => 'views_handler_relationship',
    ],
  ];
}

/**
 * Implements hook_views_plugins().
 */
function comment_views_plugins() {
  return [
    'module' => 'views',
    'row' => [
      'comment' => [
        'title' => t('Comment'),
        'help' => t('Display the comment with standard comment view.'),
        'handler' => 'views_plugin_row_comment_view',
        'theme' => 'views_view_row_comment',
        'path' => backdrop_get_path('module', 'views') . '/modules/comment', // not necessary for most modules
        'base' => ['comment'], // only works with 'comment' as base.
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-comment',
      ],
      'comment_rss' => [
        'title' => t('Comment'),
        'help' => t('Display the comment as RSS.'),
        'handler' => 'views_plugin_row_comment_rss',
        'theme' => 'views_view_row_rss',
        'path' => backdrop_get_path('module', 'views') . '/modules/comment', // not necessary for most modules
        'base' => ['comment'], // only works with 'comment' as base.
        'uses options' => TRUE,
        'type' => 'feed',
        'help topic' => 'style-comment-rss',
      ],
    ],
  ];
}

/**
 * Template helper for theme_views_view_row_comment
 */
function template_preprocess_views_view_row_comment(&$variables) {
  $options = $variables['options'];
  $view = &$variables['view'];
  $plugin = &$view->style_plugin->row_plugin;
  $comment = $plugin->comments[$variables['row']->{$variables['field_alias']}];
  $node = $plugin->nodes[$comment->nid];
  // Put the view on the node so we can retrieve it in the preprocess.
  $node->view = &$view;

  $build = comment_view_multiple([$comment->cid => $comment], $node, $plugin->options['view_mode']);
  // If we're displaying the comments without links, remove them from the
  // renderable array. There is no way to avoid building them in the first
  // place (see comment_build_content()).
  if (empty($options['links'])) {
    foreach ($build as $cid => &$comment_build) {
      if (isset($comment_build['links'])) {
        unset($comment_build['links']);
      }
    }
  }
  $variables['comment'] = backdrop_render($build);
}
