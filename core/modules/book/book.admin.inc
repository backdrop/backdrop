<?php
/**
 * @file
 * Admin page callbacks for the Book module.
 */

/**
 * Returns an administrative overview of all books.
 *
 * @return string
 *   A HTML-formatted string with the administrative page content.
 *
 * @see book_menu()
 */
function book_admin_overview() {
  $rows = [];

  $headers = [t('Book'), t('Operations')];

  // Add any recognized books to the table list.
  foreach (book_get_books() as $book) {
    $row = [
      l($book['title'], $book['href'], $book['options']),
    ];
    $links = [];
    $links['edit'] = [
      'title' => t('edit order and titles'),
      'href' => 'admin/content/book/' . $book['nid'],
    ];
    $row[] = [
      'data' => [
        '#type' => 'operations',
        '#links' => $links,
      ],
    ];
    $rows[] = $row;
  }

  return theme('table', ['header' => $headers, 'rows' => $rows, 'empty' => t('No books available.')]);
}

/**
 * Form constructor for the book settings form.
 *
 * @see book_menu()
 * @see book_admin_settings_submit()
 * @see book_admin_settings_validate()
 * @ingroup forms
 */
function book_admin_settings($form, &$form_state) {
  $types = node_type_get_names();
  $config = config('book.settings');
  $form['book_allowed_types'] = [
    '#type' => 'checkboxes',
    '#title' => t('Content types allowed in book outlines'),
    '#default_value' => $config->get('book_allowed_types'),
    '#options' => $types,
    '#description' => t('Users with the %outline-perm permission can add all content types.', ['%outline-perm' => t('Administer book outlines')]),
    '#required' => TRUE,
  ];
  $form['book_child_type'] = [
    '#type' => 'select',
    '#title' => t('Content type for child pages'),
    '#default_value' => $config->get('book_child_type'),
    '#options' => $types,
    '#required' => TRUE,
  ];
  $form['book_options'] = [
    '#type' => 'fieldset',
    '#title' => t('Options'),
  ];
  $form['book_options']['book_navigation'] = [
    '#type' => 'checkbox',
    '#title' => t('Display default book navigation on book pages'),
    '#default_value' => $config->get('book_navigation'),
  ];
  $form['book_options']['book_navigation_options'] = [
    '#type' => 'checkboxes',
    '#title' => t('Select book navigation options'),
    '#default_value' => $config->get('book_navigation_options'),
    '#options' => [
      'tree' => t('Table of contents'),
      'pager' => t('Previous, Next, and Up links'),
    ],
    '#states' => [
      'invisible' => [
        ':input[name="book_navigation"]' => ['checked' => FALSE],
      ],
    ],
  ];
  $form['book_options']['book_links'] = [
    '#type' => 'checkboxes',
    '#title' => t('Select book links options'),
    '#default_value' => $config->get('book_links'),
    '#options' => [
      'book_add_child' => t('Add child page'),
      'book_reorder' => t('Reorder book'),
    ],
  ];
  $form['array_filter'] = ['#type' => 'value', '#value' => TRUE];
  $form['#validate'][] = 'book_admin_settings_validate';

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save configuration')
  ];
  return $form;
}

/**
 * Form validation handler for book_admin_settings().
 *
 * @see book_admin_settings_submit()
 */
function book_admin_settings_validate($form, &$form_state) {
  $child_type = $form_state['values']['book_child_type'];
  if (empty($form_state['values']['book_allowed_types'][$child_type])) {
    form_set_error('book_child_type', t('The content type for the %add-child link must be one of those selected as an allowed book outline type.', ['%add-child' => t('Add child page')]));
  }
}

 /**
 * Form submission handler for book_admin_settings().
 *
 * @see book_admin_settings_validate()
 */
function book_admin_settings_submit($form, &$form_state) {
  config('book.settings')
    ->set('book_allowed_types', $form_state['values']['book_allowed_types'])
    ->set('book_child_type', $form_state['values']['book_child_type'])
    ->set('book_navigation', $form_state['values']['book_navigation'])
    ->set('book_navigation_options', $form_state['values']['book_navigation_options'])
    ->set('book_links', $form_state['values']['book_links'])
    ->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form constructor for administering a single book's hierarchy.
 *
 * @param Node $node
 *   The node of the top-level page in the book.
 *
 * @see book_menu()
 * @see book_admin_edit_validate()
 * @see book_admin_edit_submit()
 * @ingroup forms
 */
function book_admin_edit($form, $form_state, Node $node) {
  backdrop_set_title($node->title);
  $form['#node'] = $node;
  _book_admin_table($node, $form);
  $form['save'] = [
    '#type' => 'submit',
    '#value' => t('Save book pages'),
  ];

  return $form;
}

/**
 * Form validation handler for book_admin_edit().
 *
 * Checks that the book has not been changed while using the form.
 *
 * @see book_admin_edit_submit()
 */
function book_admin_edit_validate($form, &$form_state) {
  if ($form_state['values']['tree_hash'] != $form_state['values']['tree_current_hash']) {
    form_set_error('', t('This book has been modified by another user, the changes could not be saved.'));
  }
}

/**
 * Form submission handler for book_admin_edit().
 *
 * This function takes care to save parent menu items before their children.
 * Saving menu items in the incorrect order can break the menu tree.
 *
 * @see book_admin_edit_validate()
 * @see menu_overview_form_submit()
 */
function book_admin_edit_submit($form, &$form_state) {
  // Save elements in the same order as defined in post rather than the form.
  // This ensures parents are updated before their children, preventing orphans.
  $order = array_flip(array_keys($form_state['input']['table']));
  $form['table'] = array_merge($order, $form['table']);

  foreach (element_children($form['table']) as $key) {
    if ($form['table'][$key]['#item']) {
      $row = $form['table'][$key];
      $values = $form_state['values']['table'][$key];

      // Update menu item if moved.
      if ($row['plid']['#default_value'] != $values['plid'] || $row['weight']['#default_value'] != $values['weight']) {
        $row['#item']['plid'] = $values['plid'];
        $row['#item']['weight'] = $values['weight'];
        menu_link_save($row['#item']);
      }

      // Update the title if changed.
      if ($row['title']['#default_value'] != $values['title']) {
        $node = node_load($values['nid']);
        $langcode = LANGUAGE_NONE;
        $node->title = $values['title'];
        $node->book['link_title'] = $values['title'];
        $node->revision = 1;
        $node->log = t('Title changed from %original to %current.', ['%original' => $node->title, '%current' => $values['title']]);

        $node->save();
        watchdog('content', 'book: updated %title.', ['%title' => $node->title], WATCHDOG_NOTICE, l(t('view'), 'node/' . $node->nid));
      }
    }
  }

  backdrop_set_message(t('Updated book %title.', ['%title' => $form['#node']->title]));
}

/**
 * Builds the table portion of the form for the book administration page.
 *
 * @param Node $node
 *   The node of the top-level page in the book.
 * @param $form
 *   The form that is being modified, passed by reference.
 *
 * @see book_admin_edit()
 */
function _book_admin_table(Node $node, &$form) {
  $form['table'] = [
    '#theme' => 'book_admin_table',
    '#tree' => TRUE,
  ];

  $tree = book_menu_subtree_data($node->book);
  $tree = array_shift($tree); // Do not include the book item itself.
  if ($tree['below']) {
    $hash = backdrop_hash_base64(serialize($tree['below']));
    // Store the hash value as a hidden form element so that we can detect
    // if another user changed the book hierarchy.
    $form['tree_hash'] = [
      '#type' => 'hidden',
      '#default_value' => $hash,
    ];
    $form['tree_current_hash'] = [
      '#type' => 'value',
      '#value' => $hash,
    ];
    _book_admin_table_tree($tree['below'], $form['table']);
  }

}

/**
 * Helps build the main table in the book administration page form.
 *
 * @param $tree
 *   A subtree of the book menu hierarchy.
 * @param $form
 *   The form that is being modified, passed by reference.
 *
 * @return
 *   The modified form array.
 *
 * @see book_admin_edit()
 */
function _book_admin_table_tree($tree, &$form) {
  // The delta must be big enough to give each node a distinct value.
  $count = count($tree);
  $delta = ($count < 30) ? 15 : intval($count / 2) + 1;

  foreach ($tree as $data) {
    $form['book-admin-' . $data['link']['nid']] = [
      '#item' => $data['link'],
      'nid' => ['#type' => 'value', '#value' => $data['link']['nid']],
      'depth' => ['#type' => 'value', '#value' => $data['link']['depth']],
      'href' => ['#type' => 'value', '#value' => $data['link']['href']],
      'title' => [
        '#type' => 'textfield',
        '#default_value' => $data['link']['link_title'],
        '#maxlength' => 255,
        '#size' => 40,
      ],
      'weight' => [
        '#type' => 'weight',
        '#default_value' => $data['link']['weight'],
        '#delta' => max($delta, abs($data['link']['weight'])),
        '#title' => t('Weight for @title', ['@title' => $data['link']['title']]),
        '#title_display' => 'invisible',
      ],
      'plid' => [
        '#type' => 'hidden',
        '#default_value' => $data['link']['plid'],
      ],
      'mlid' => [
        '#type' => 'hidden',
        '#default_value' => $data['link']['mlid'],
      ],
    ];
    if ($data['below']) {
      _book_admin_table_tree($data['below'], $form);
    }
  }

  return $form;
}

/**
 * Returns HTML for a book administration form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see book_admin_table()
 * @ingroup themeable
 */
function theme_book_admin_table($variables) {
  $form = $variables['form'];

  backdrop_add_tabledrag('book-outline', 'match', 'parent', 'book-plid', 'book-plid', 'book-mlid', TRUE, MENU_MAX_DEPTH - 2);
  backdrop_add_tabledrag('book-outline', 'order', 'sibling', 'book-weight');

  $header = [t('Title'), t('Weight'), t('Parent'), t('Operations')];

  $rows = [];
  $destination = backdrop_get_destination();
  $access = user_access('administer nodes');
  foreach (element_children($form) as $key) {
    $nid = $form[$key]['nid']['#value'];
    $href = $form[$key]['href']['#value'];

    // Add special classes to be used with tabledrag.js.
    $form[$key]['plid']['#attributes']['class'] = ['book-plid'];
    $form[$key]['mlid']['#attributes']['class'] = ['book-mlid'];
    $form[$key]['weight']['#attributes']['class'] = ['book-weight'];

    $data = [
      theme('indentation', ['size' => $form[$key]['depth']['#value'] - 2]) . backdrop_render($form[$key]['title']),
      backdrop_render($form[$key]['weight']),
      backdrop_render($form[$key]['plid']) . backdrop_render($form[$key]['mlid']),
    ];
    $links = [];
    $links['view'] = [
      'title' => t('view'),
      'href' => $href,
    ];
    if ($access) {
      $links['edit'] = [
        'title' => t('Edit'),
        'href' => "node/$nid/edit",
        'query' => $destination,
      ];
      $links['delete'] = [
        'title' => t('Delete'),
        'href' => "node/$nid/delete",
        'query' => $destination,
      ];
      $links['remove'] = [
        'title' => t('Remove'),
        'href' => "node/$nid/outline-remove",
        'query' => $destination,
      ];
    }
    $data[] = [
      'data' => [
        '#type' => 'operations',
        '#links' => $links,
      ],
    ];
    $row = ['data' => $data];
    if (isset($form[$key]['#attributes'])) {
      $row = array_merge($row, $form[$key]['#attributes']);
    }
    $row['class'][] = 'draggable';
    $rows[] = $row;
  }

  return theme('table', ['header' => $header, 'rows' => $rows, 'attributes' => ['id' => 'book-outline'], 'empty' => t('No book content available.')]);
}
