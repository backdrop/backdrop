<?php
/**
 * @file
 * Admin page callbacks for the Database Logging module.
 */

/**
 * Page callback: Displays a listing of database log messages.
 *
 * Messages are truncated at the length determined by the 'log_max_length'
 * setting. Full-length messages can be viewed on the message details page.
 *
 * @see dblog_clear_log_form()
 * @see dblog_event()
 * @see dblog_filter_form()
 * @see dblog_menu()
 */
function dblog_overview() {
  $filter = dblog_build_filter_query();
  $rows = [];
  $classes = watchdog_severity_levels('name');
  foreach ($classes as $key => $value) {
    $classes[$key] = 'dblog-' . $value;
  }

  $build['dblog_filter_form'] = backdrop_get_form('dblog_filter_form');

  $header = [
    '', // Icon column.
    ['data' => t('Type'), 'field' => 'w.type', 'class' => [RESPONSIVE_PRIORITY_MEDIUM]],
    ['data' => t('Date'), 'field' => 'w.wid', 'sort' => 'desc', 'class' => [RESPONSIVE_PRIORITY_LOW]],
    t('Message'),
    ['data' => t('User'), 'field' => 'u.name', 'class' => [RESPONSIVE_PRIORITY_MEDIUM]],
    ['data' => t('Operations'), 'class' => [RESPONSIVE_PRIORITY_LOW]],
  ];

  $query = db_select('watchdog', 'w')->extend('PagerDefault')->extend('TableSort');
  $query->leftJoin('users', 'u', 'w.uid = u.uid');
  $query
    ->fields('w', ['wid', 'uid', 'severity', 'type', 'timestamp', 'message', 'variables', 'link'])
    ->addField('u', 'name');
  if (!empty($filter['where'])) {
    $query->where($filter['where'], $filter['args']);
  }
  $result = $query
    ->limit(50)
    ->orderByHeader($header)
    ->execute();

  foreach ($result as $dblog) {
    if (!empty($dblog->uid) && empty($dblog->name)) {
      $user_info = t('Deleted User');
    }
    else {
      $user_info = theme('username', ['account' => $dblog]);
    }

    $log_date_format = config_get('system.core', 'log_date_format');
    $rows[] = ['data' =>
      [
        // Cells
        ['class' => 'icon'],
        t($dblog->type),
        format_date($dblog->timestamp, 'custom', $log_date_format),
        dblog_format_message($dblog, TRUE),
        $user_info,
        filter_xss($dblog->link),
      ],
      // Attributes for tr
      'class' => [backdrop_html_class('dblog-' . $dblog->type), $classes[$dblog->severity]],
    ];
  }

  $build['dblog_table'] = [
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => ['id' => 'admin-dblog', 'class' => ['admin-dblog']],
    '#empty' => t('No log messages available.'),
  ];
  $build['dblog_pager'] = ['#theme' => 'pager'];

  $build['dblog_clear_log_form'] = backdrop_get_form('dblog_clear_log_form');

  return $build;
}

/**
 * Page callback: Shows the most frequent log messages of a given event type.
 *
 * Messages are not truncated on this page because events detailed herein do not
 * have links to a detailed view.
 *
 * @param string $type
 *   Type of database log events to display (e.g., 'search').
 *
 * @return array
 *   A build array in the format expected by backdrop_render().
 *
 * @see dblog_menu()
 */
function dblog_top($type) {

  $header = [
    ['data' => t('Count'), 'field' => 'count', 'sort' => 'desc'],
    ['data' => t('Message'), 'field' => 'message']
  ];
  $count_query = db_select('watchdog');
  $count_query->addExpression('COUNT(DISTINCT(message))');
  $count_query->condition('type', $type);

  $query = db_select('watchdog', 'w')->extend('PagerDefault')->extend('TableSort');
  $query->addExpression('COUNT(wid)', 'count');
  $query = $query
    ->fields('w', ['message', 'variables'])
    ->condition('w.type', $type)
    ->groupBy('message')
    ->groupBy('variables')
    ->limit(30)
    ->orderByHeader($header);
  $query->setCountQuery($count_query);
  $result = $query->execute();

  $rows = [];
  foreach ($result as $dblog) {
    $rows[] = [$dblog->count, dblog_format_message($dblog)];
  }

  $build['dblog_top_table']  = [
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No log messages available.'),
  ];
  $build['dblog_top_pager'] = ['#theme' => 'pager'];

  return $build;
}

/**
 * Page callback: Displays details about a specific database log message.
 *
 * @param int $id
 *   Unique ID of the database log message.
 *
 * @return array|string
 *   If the ID is located in the Database Logging table, a build array in the
 *   format expected by backdrop_render(); otherwise, an empty string.
 *
 * @see dblog_menu()
 */
function dblog_event($id) {
  $severity = watchdog_severity_levels();
  $result = db_query('SELECT w.*, u.name, w.uid FROM {watchdog} w LEFT JOIN {users} u ON w.uid = u.uid WHERE w.wid = :id', [':id' => $id])->fetchObject();
  if ($dblog = $result) {
    if (!empty($dblog->uid) && empty($dblog->name)) {
      $user_label = t('User ID (deleted)');
      $user_info = $dblog->uid;
    }
    else {
      $user_label = t('User');
      $user_info = theme('username', ['account' => $dblog]);
    }

    $rows = [
      [
        ['data' => t('Type'), 'header' => TRUE],
        t($dblog->type),
      ],
      [
        ['data' => t('Date'), 'header' => TRUE],
        format_date($dblog->timestamp, 'long'),
      ],
      [
        ['data' => $user_label, 'header' => TRUE],
        $user_info,
      ],
      [
        ['data' => t('Location'), 'header' => TRUE],
        l($dblog->location, $dblog->location),
      ],
      [
        ['data' => t('Referrer'), 'header' => TRUE],
        l($dblog->referer, $dblog->referer),
      ],
      [
        ['data' => t('Message'), 'header' => TRUE],
        dblog_format_message($dblog),
      ],
      [
        ['data' => t('Severity'), 'header' => TRUE],
        $severity[$dblog->severity],
      ],
      [
        ['data' => t('Hostname'), 'header' => TRUE],
        check_plain($dblog->hostname),
      ],
      [
        ['data' => t('Operations'), 'header' => TRUE],
        $dblog->link,
      ],
    ];
    $build['dblog_table'] = [
      '#theme' => 'table',
      '#rows' => $rows,
      '#attributes' => ['class' => ['dblog-event']],
    ];
    return $build;
  }
  else {
    return '';
  }
}

/**
 * Builds a query for database log administration filters based on session.
 *
 * @return array
 *   An associative array with keys 'where' and 'args'.
 */
function dblog_build_filter_query() {
  if (empty($_SESSION['dblog_overview_filter'])) {
    return;
  }

  $filters = dblog_filters();

  // Build query
  $where = $args = [];
  foreach ($_SESSION['dblog_overview_filter'] as $key => $filter) {
    $filter_where = [];
    foreach ($filter as $value) {
      $filter_where[] = $filters[$key]['where'];
      $args[] = $value;
    }
    if (!empty($filter_where)) {
      $where[] = '(' . implode(' OR ', $filter_where) . ')';
    }
  }
  $where = !empty($where) ? implode(' AND ', $where) : '';

  return [
    'where' => $where,
    'args' => $args,
  ];
}

/**
 * Creates a list of database log administration filters that can be applied.
 *
 * @return array
 *   Associative array of filters. The top-level keys are used as the form
 *   element names for the filters, and the values are arrays with the following
 *   elements:
 *   - title: Title of the filter.
 *   - where: The filter condition.
 *   - options: Array of options for the select list for the filter.
 */
function dblog_filters() {
  $filters = [];

  foreach (_dblog_get_message_types() as $type) {
    $types[$type] = t($type);
  }

  if (!empty($types)) {
    $filters['type'] = [
      'title' => t('Type'),
      'where' => "w.type = ?",
      'options' => $types,
    ];
  }

  $filters['severity'] = [
    'title' => t('Severity'),
    'where' => 'w.severity = ?',
    'options' => watchdog_severity_levels(),
  ];

  return $filters;
}

/**
 * Returns HTML for a log message.
 *
 * @param object $event
 *   An object containing at least the message and variables properties.
 * @param bool $is_link
 *   (optional) Format message as link, event->wid is required.
 *
 * @return string
 *   HTML containing a link to the full report.
 */
function dblog_format_message($event, $is_link = FALSE) {
  $output = '';
  // Check for required properties.
  if (isset($event->message) && isset($event->variables)) {
    $event_variables = @unserialize($event->variables);
    // Messages without variables or user specified text.
    if ($event_variables === NULL) {
      $output = $event->message;
    }
    elseif (!is_array($event_variables)) {
      $output = t('Log data is corrupted and cannot be unserialized: @message', [
        '@message' => $event->message,
      ]);
    }
    // Message to translate with injected variables.
    else {
      $output = t($event->message, $event_variables);
    }
    // If the output is expected to be a link, strip all the tags and
    // special characters by using filter_xss() without any allowed tags.
    // If not, use filter_xss_admin() to allow some tags.
    if ($is_link && isset($event->wid)) {
      // Truncate message after stripping all the tags.
      $output = truncate_utf8(filter_xss($output, []), config_get('system.core', 'log_max_length'), TRUE, TRUE);
      $output = l($output, 'admin/reports/event/' . $event->wid, ['html' => TRUE]);
    }
    else {
      // Prevent XSS in log detail pages.
      $output = filter_xss_admin($output);
    }
  }

  return $output;
}

/**
 * Form constructor for the database logging filter form.
 *
 * @see dblog_filter_form_validate()
 * @see dblog_filter_form_submit()
 * @see dblog_overview()
 *
 * @ingroup forms
 */
function dblog_filter_form($form) {
  $filters = dblog_filters();

  $form['filters'] = [
    '#type' => 'fieldset',
    '#title' => t('Filter log messages'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($_SESSION['dblog_overview_filter']),
  ];
  foreach ($filters as $key => $filter) {
    $form['filters']['status'][$key] = [
      '#title' => $filter['title'],
      '#type' => 'select',
      '#multiple' => TRUE,
      '#size' => 8,
      '#options' => $filter['options'],
    ];
    if (!empty($_SESSION['dblog_overview_filter'][$key])) {
      $form['filters']['status'][$key]['#default_value'] = $_SESSION['dblog_overview_filter'][$key];
    }
  }

  $form['filters']['actions'] = [
    '#type' => 'actions',
    '#attributes' => ['class' => ['container-inline']],
  ];
  $form['filters']['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Filter'),
  ];
  if (!empty($_SESSION['dblog_overview_filter'])) {
    $form['filters']['actions']['reset'] = [
      '#type' => 'submit',
      '#value' => t('Reset')
    ];
  }
  return $form;
}

/**
 * Form validation handler for dblog_filter_form().
 *
 * @see dblog_filter_form_submit()
 */
function dblog_filter_form_validate($form, &$form_state) {
  if ($form_state['values']['op'] == t('Filter') && empty($form_state['values']['type']) && empty($form_state['values']['severity'])) {
    form_set_error('type', t('You must select something to filter by.'));
  }
}

/**
 * Form submission handler for dblog_filter_form().
 *
 * @see dblog_filter_form_validate()
 */
function dblog_filter_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];
  $filters = dblog_filters();
  switch ($op) {
    case t('Filter'):
      foreach ($filters as $name => $filter) {
        if (isset($form_state['values'][$name])) {
          $_SESSION['dblog_overview_filter'][$name] = $form_state['values'][$name];
        }
      }
      break;
    case t('Reset'):
      $_SESSION['dblog_overview_filter'] = [];
      break;
  }
  return 'admin/reports/dblog';
}

/**
 * Form constructor for the form that clears out the log.
 *
 * @see dblog_clear_log_submit()
 * @ingroup forms
 */
function dblog_clear_log_form($form) {
  $form['dblog_clear']['help'] = [
    '#type' => 'help',
    '#markup' => t('This action will permanently delete the log messages from the database.'),
  ];
  $form['dblog_clear']['clear'] = [
    '#type' => 'submit',
    '#value' => t('Clear log messages'),
    '#submit' => ['dblog_clear_log_submit'],
    '#attributes' => ['class' => ['button-danger']],
  ];

  return $form;
}

/**
 * Form submission handler for dblog_clear_log_form().
 */
function dblog_clear_log_submit() {
  $_SESSION['dblog_overview_filter'] = [];
  db_truncate('watchdog')->execute();
  backdrop_set_message(t('Database log cleared.'));
}
