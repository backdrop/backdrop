<?php

/**
 * @file
 * Administrative page callbacks for the redirect module.
 */

function redirect_list_form($form, &$form_state) {
  $form['#operations'] = redirect_get_redirect_operations();
  if (isset($form_state['values']['operation']) && empty($form_state['values']['confirm'])) {
    return redirect_list_form_operations_confirm_form($form, $form_state, $form_state['values']['operation'], array_filter($form_state['values']['rids']));
  }

  $destination = backdrop_get_destination();
  $config = config('redirect.settings');
  $default_status_code = $config->get('default_status_code');

  // Set up the header.
  $header = array(
    'source' => array('data' => t('From'), 'field' => 'source', 'sort' => 'asc'),
    'redirect' => array('data' => t('To'), 'field' => 'redirect'),
    'status_code' => array('data' => t('Type'), 'field' => 'status_code'),
    'language' => array('data' => t('Language'), 'field' => 'langcode'),
    'count' => array('data' => t('Count'), 'field' => 'count'),
    'access' => array('data' => t('Last accessed'), 'field' => 'access'),
    'operations' => array('data' => t('Operations')),
  );

  // Do not include the language column if locale is disabled.
  if (!module_exists('locale')) {
    unset($header['language']);
  }

  // Get filter keys and add the filter form.
  $keys = func_get_args();
  $keys = array_splice($keys, 2); // Offset the $form and $form_state parameters.
  $keys = implode('/', $keys);
  $form['redirect_list_filter_form'] = redirect_list_filter_form($keys);

  // Build the 'Update options' form.
  $form['operations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
    '#attributes' => array(
      'class' => array('redirect-list-operations'),
    ),
  );
  $operations = array();
  foreach ($form['#operations'] as $key => $operation) {
    $operations[$key] = $operation['action'];
  }
  $form['operations']['operation'] = array(
    '#type' => 'select',
    '#options' => $operations,
    '#default_value' => 'delete',
  );
  $form['operations']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('redirect_list_form_operations_validate'),
    '#submit' => array('redirect_list_form_operations_submit'),
  );

  // Building the SQL query and load the redirects.
  $query = db_select('redirect', 'r')->extend('TableSort')->extend('PagerDefault');
  $query->addField('r', 'rid');
  $query->condition('r.type', 'redirect');
  $query->orderByHeader($header);
  $query->limit(50);
  $query->addTag('redirect_list');
  $query->addTag('redirect_access');
  redirect_build_filter_query($query, array('source', 'redirect'), $keys);
  $rids = $query->execute()->fetchCol();
  $redirects = redirect_load_multiple($rids);

  $rows = array();
  foreach ($redirects as $rid => $redirect) {
    $row = array();
    $redirect->source_options = array_merge($redirect->source_options, array('alias' => TRUE, 'langcode' => redirect_language_load($redirect->langcode)));
    $source_url = redirect_url($redirect->source, $redirect->source_options);
    $redirect_url = redirect_url($redirect->redirect, array_merge($redirect->redirect_options, array('alias' => TRUE)));
    backdrop_alter('redirect_url', $redirect->source, $redirect->source_options);
    backdrop_alter('redirect_url', $redirect->redirect, $redirect->redirect_options);
    $row['source'] = l($source_url, $redirect->source, $redirect->source_options);
    $row['redirect'] = l($redirect_url, $redirect->redirect, $redirect->redirect_options);
    $row['status_code'] = $redirect->status_code ? $redirect->status_code : t('Default (@default)', array('@default' => $default_status_code));
    $row['language'] = language_name($redirect->langcode);
    $row['count'] = $redirect->count;
    if ($redirect->access) {
      $row['access'] = array(
        'data' => t('!interval ago', array('!interval' => format_interval(REQUEST_TIME - $redirect->access))),
        'title' => t('Last accessed on @date', array('@date' => format_date($redirect->access))),
      );
    }
    else {
      $row['access'] = t('Never');
    }

    // Mark redirects that override existing paths with a warning in the table.
    if (backdrop_valid_path($redirect->source)) {
      $row['#attributes']['class'][] = 'warning';
      $row['#attributes']['title'] = t('This redirect overrides an existing internal path.');
    }

    $operations = array();
    if (redirect_access('update', $redirect)) {
      $operations['edit'] = array(
        'title' => t('Edit'),
        'href' => 'admin/config/search/redirect/edit/' . $rid,
        'query' => $destination,
      );
    }
    if (redirect_access('delete', $redirect)) {
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/config/search/redirect/delete/' . $rid,
        'query' => $destination,
      );
    }
    $row['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
      ),
    );

    $rows[$rid] = $row;
  }

  $form['rids'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No URL redirects available.'),
    '#attributes' => array(
      'class' => array('redirect-list-tableselect'),
    ),
    '#attached' => array(
      'js' => array(
        backdrop_get_path('module', 'redirect') . '/redirect.admin.js',
      ),
    ),
  );
  if (redirect_access('create', 'redirect')) {
    $form['rids']['#empty'] .= ' ' . l(t('Add URL redirect.'), 'admin/config/search/redirect/add', array('query' => $destination));
  }
  $form['pager'] = array('#theme' => 'pager');
  return $form;
}

/**
 * Return a form to filter URL redirects.
 *
 * @see redirect_list_filter_form_submit()
 *
 * @ingroup forms
 */
function redirect_list_filter_form($keys = '') {
  $form['#attributes'] = array('class' => array('search-form'));
  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter redirects'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['basic']['filter'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => $keys,
    '#maxlength' => 128,
    '#size' => 25,
  );
  $form['basic']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#submit' => array('redirect_list_filter_form_submit'),
  );
  if ($keys) {
    $form['basic']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#submit' => array('redirect_list_filter_form_reset'),
    );
  }
  return $form;
}

/**
 * Process filter form submission when the Filter button is pressed.
 */
function redirect_list_filter_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/redirect/list/' . trim($form_state['values']['filter']);
}

/**
 * Process filter form submission when the Reset button is pressed.
 */
function redirect_list_filter_form_reset($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/redirect';
}

/**
 * Extends a query object for URL redirect filters.
 *
 * @param $query
 *   Query object that should be filtered.
 * @param $keys
 *   The filter string to use.
 */
function redirect_build_filter_query(QueryAlterableInterface $query, array $fields, $keys = '') {
  if ($keys && $fields) {
    // Replace wildcards with PDO wildcards.
    $conditions = db_or();
    $wildcard = '%' . trim(preg_replace('!\*+!', '%', db_like($keys)), '%') . '%';
    foreach ($fields as $field) {
      $conditions->condition($field, $wildcard, 'LIKE');
    }
    $query->condition($conditions);
  }
}

/**
 * Validate redirect_list_form form submissions.
 *
 * Check if any redirects have been selected to perform the chosen
 * 'Update option' on.
 */
function redirect_list_form_operations_validate($form, &$form_state) {
  // Error if there are no redirects selected.
  if (!is_array($form_state['values']['rids']) || !count(array_filter($form_state['values']['rids']))) {
    form_set_error('', t('No redirects selected.'));
  }
}

/**
 * Process redirect_list_form form submissions.
 *
 * Execute the chosen 'Update option' on the selected redirects.
 */
function redirect_list_form_operations_submit($form, &$form_state) {
  $operations = $form['#operations'];
  $operation = $operations[$form_state['values']['operation']];

  // Filter out unchecked redirects
  $rids = array_filter($form_state['values']['rids']);

  if (!empty($operation['confirm']) && empty($form_state['values']['confirm'])) {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of redirects.
    $form_state['rebuild'] = TRUE;
  }
  else {
    $function = $operation['callback'];

    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($rids), $operation['callback arguments']);
    }
    else {
      $args = array($rids);
    }
    call_user_func_array($function, $args);

    $count = count($form_state['values']['rids']);
    watchdog('redirect', '@action @count redirects.', array('@action' => $operation['action_past'], '@count' => $count));
    backdrop_set_message(format_plural(count($rids), '@action @count redirect.', '@action @count redirects.', array('@action' => $operation['action_past'], '@count' => $count)));
  }
}

function redirect_list_form_operations_confirm_form($form, &$form_state, $operation, $rids) {
  $operations = $form['#operations'];
  $operation = $operations[$form_state['values']['operation']];

  $form['rids_list'] = array(
    '#theme' => 'item_list',
    '#items' => array(),
  );
  $form['rids'] = array(
    '#type' => 'value',
    '#value' => $rids,
  );

  $redirects = redirect_load_multiple($rids);
  foreach ($redirects as $rid => $redirect) {
    $form['rids_list']['#items'][$rid] = check_plain(redirect_url($redirect->source, $redirect->source_options));
  }

  $form['operation'] = array('#type' => 'hidden', '#value' => $form_state['values']['operation']);
  $form['#submit'][] = 'redirect_list_form_operations_submit';
  $confirm_question = format_plural(count($rids), 'Are you sure you want to @action this redirect?', 'Are you sure you want to @action these redirects?', array('@action' => backdrop_strtolower($operation['action'])));

  return confirm_form(
    $form,
    $confirm_question,
    'admin/config/search/redirect', // @todo This does not redirect back to filtered page.
    t('This action cannot be undone.'),
    $operation['action'],
    t('Cancel')
  );
}

/**
 * Page callback; Redirect to the form for creating a redirect.
 */
function redirect_add_redirect_page() {
  backdrop_goto('admin/config/search/redirect/add');
}

/**
 * Form builder to add or edit an URL redirect.
 *
 * @see redirect_element_validate_source()
 * @see redirect_element_validate_redirect()
 * @see redirect_edit_form_validate()
 * @see redirect_edit_form_submit()
 *
 * @ingroup forms
 */
function redirect_edit_form($form, &$form_state, $redirect = NULL) {
  if (!isset($redirect)) {
    $redirect = new Redirect(array(
      'source' => isset($_GET['source']) ? urldecode($_GET['source']) : '',
      'source_options' => isset($_GET['source_options']) ? backdrop_get_query_array($_GET['source_options']) : array(),
      'redirect' => isset($_GET['redirect']) ? urldecode($_GET['redirect']) : '',
      'redirect_options' => isset($_GET['redirect_options']) ? backdrop_get_query_array($_GET['redirect_options']) : array(),
      'langcode' => isset($_GET['langcode']) ? urldecode($_GET['langcode']) : LANGUAGE_NONE,
    ));
  }
  $config = config('redirect.settings');

  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $redirect->rid,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $redirect->type,
  );
  $form['hash'] = array(
    '#type' => 'value',
    '#value' => $redirect->hash,
  );
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $redirect->uid,
  );

  $form['source'] = array(
    '#type' => 'textfield',
    '#title' => t('From'),
    '#description' => t('Enter an internal Backdrop path or path alias to redirect (e.g. %example1 or %example2). Fragment anchors (e.g. %anchor) are <strong>not</strong> allowed.', array('%example1' => 'node/123', '%example2' => 'taxonomy/term/123', '%anchor' => '#anchor')),
    '#maxlength' => 560,
    '#default_value' => $redirect->rid || $redirect->source ? redirect_url($redirect->source, $redirect->source_options + array('alter' => FALSE)) : '',
    '#required' => TRUE,
    '#field_prefix' => $GLOBALS['base_url'] . '/' . (config_get('system.core', 'clean_url') ? '' : '?q='),
    '#element_validate' => array('redirect_element_validate_source'),
  );
  $form['source_options'] = array(
    '#type' => 'value',
    '#value' => $redirect->source_options,
    '#tree' => TRUE,
  );
  $form['redirect'] = array(
    '#type' => 'textfield',
    '#title' => t('To'),
    '#maxlength' => 560,
    '#default_value' => $redirect->rid || $redirect->redirect ? redirect_url($redirect->redirect, $redirect->redirect_options, TRUE) : '',
    '#required' => TRUE,
    '#description' => t('Enter an internal Backdrop path, path alias, or complete external URL (like http://example.com/) to redirect to. Use %front to redirect to the front page.', array('%front' => '<front>')),
    '#element_validate' => array('redirect_element_validate_redirect'),
  );
  $form['redirect_options'] = array(
    '#type' => 'value',
    '#value' => $redirect->redirect_options,
    '#tree' => TRUE,
  );

  // This will be a hidden value unless locale module is enabled.
  $form['langcode'] = array(
    '#type' => 'value',
    '#value' => $redirect->langcode,
  );

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced']['status_code'] = array(
    '#type' => 'select',
    '#title' => t('Redirect status'),
    '#description' => t('You can find more information about HTTP redirect status codes at <a href="@status-codes">@status-codes</a>.', array('@status-codes' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection')),
    '#default_value' => $redirect->status_code,
    '#options' => array(0 => t('Default (@default)', array('@default' => $config->get('redirect_default_status_code')))) + redirect_status_code_options(),
  );

  $form['override'] = array(
    '#type' => 'checkbox',
    '#title' => t('I understand the following warnings and would like to proceed with saving this URL redirect'),
    '#default_value' => FALSE,
    '#access' => FALSE,
    '#required' => FALSE,
    '#weight' => -100,
    '#prefix' => '<div class="messages warning">',
    '#suffix' => '</div>',
  );
  if (!empty($form_state['storage']['override_messages'])) {
    $form['override']['#access'] = TRUE;
    //$form['override']['#required'] = TRUE;
    $form['override']['#description'] = theme('item_list', array('items' => $form_state['storage']['override_messages']));
    // Reset the messages.
    $form_state['storage']['override_messages'] = array();
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => isset($_GET['destination']) ? $_GET['destination'] : 'admin/config/search/redirect',
  );

  return $form;
}

/**
 * Element validate handler; validate the source of an URL redirect.
 *
 * @see redirect_edit_form()
 */
function redirect_element_validate_source($element, &$form_state) {
  $value = &$element['#value'];

  // Check that the source contains no URL fragment.
  if (strpos($value, '#') !== FALSE) {
    form_error($element, t('The source path cannot contain an URL fragment anchor.'));
  }

  _redirect_extract_url_options($element, $form_state);

  // Disallow redirections from the frontpage.
  if ($value === '<front>') {
    form_error($element, t('The source path cannot be the front page.'));
  }

  // Cannot create redirects for valid paths.
  if (empty($form_state['values']['override'])) {
    $menu_item = menu_get_item($value);
    if ($menu_item && $menu_item['page_callback'] != 'redirect_redirect' && $value == $menu_item['path']) {
      $form_state['storage']['override_messages']['valid-path'] = t('The source path %path is likely a valid path. It is preferred to <a href="@url-alias">create URL aliases</a> for existing paths rather than redirects.', array('%path' => $value, '@url-alias' => url('admin/config/search/path/add')));
      $form_state['rebuild'] = TRUE;
    }
  }

  return $element;
}

/**
 * Element validate handler; validate the redirect of an URL redirect.
 *
 * @see redirect_edit_form()
 */
function redirect_element_validate_redirect($element, &$form_state) {
  $value = &$element['#value'];
  _redirect_extract_url_options($element, $form_state);
  $value = &$form_state['values']['redirect'];


  // Normalize the path.
  $value = backdrop_get_normal_path($value, $form_state['values']['langcode']);

  if (!valid_url($value) && !valid_url($value, TRUE) && $value != '<front>' && $value != '' && !file_exists($value)) {
    form_error($element, t('The redirect path %value is not valid.', array('%value' => $value)));
  }

  return $element;
}

/**
 * Extract the query and fragment parts out of an URL field.
 */
function _redirect_extract_url_options(&$element, &$form_state) {
  $value = &$element['#value'];
  $type = $element['#name'];
  $options = &$form_state['values']["{$type}_options"];

  $parsed = redirect_parse_url($value);

  if (isset($parsed['fragment'])) {
    $options['fragment'] = $parsed['fragment'];
  }
  else {
    unset($options['fragment']);
  }

  if (isset($parsed['query'])) {
    $options['query'] = $parsed['query'];
  }
  else {
    unset($options['query']);
  }

  if (isset($parsed['scheme']) && $parsed['scheme'] == 'https') {
    $options['https'] = TRUE;
  }
  else {
    unset($options['https']);
  }

  if (!url_is_external($parsed['url'])) {
    $parsed['url'] = backdrop_get_normal_path($parsed['url'], $form_state['values']['langcode']);
  }

  form_set_value($element, $parsed['url'], $form_state);
  return $parsed;
}

/**
 * Form validate handler; validate an URL redirect
 *
 * @see redirect_edit_form()
 */
function redirect_edit_form_validate($form, &$form_state) {
  $redirect = new Redirect($form_state['values']);

  if (empty($form_state['values']['override'])) {
    if ($existing = redirect_load_by_source($redirect->source, $redirect->langcode)) {
      if ($redirect->rid != $existing->rid && $redirect->langcode == $existing->langcode) {
        // The "from" path should not conflict with another redirect
        $form_state['storage']['override_messages']['redirect-conflict'] = t('The base source path %source is already being redirected. Do you want to <a href="@edit-page">edit the existing redirect</a>?', array('%source' => $redirect->source, '@edit-page' => url('admin/config/search/redirect/edit/'. $existing->rid)));
        $form_state['rebuild'] = TRUE;
      }
    }

    if ($form['override']['#access']) {
      backdrop_set_message('Did you read the warnings and click the checkbox?', 'error');
      $form_state['rebuild'] = TRUE;
      //form_set_error('override', 'CLICK DA BUTTON!');
    }
  }

  redirect_validate($redirect, $form, $form_state);
  $form_state['redirect'] = $redirect;
}

/**
 * Form submit handler; insert or update an URL redirect.
 *
 * @see redirect_edit_form()
 */
function redirect_edit_form_submit($form, &$form_state) {
  $redirect = $form_state['redirect'];
  $redirect->save();
  backdrop_set_message(t('The redirect has been saved.'));
  $form_state['redirect'] = 'admin/config/search/redirect';
}

/**
 * Form builder to delete an URL redirect.
 *
 * @see redirect_delete_form()
 * @see confirm_form()
 *
 * @ingroup forms
 */
function redirect_delete_form($form, &$form_state, $redirect) {
  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $redirect->rid,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the URL redirect from %source to %redirect?', array('%source' => $redirect->source, '%redirect' => $redirect->redirect)),
    'admin/config/search/redirect'
  );
}

/**
 * Form submit handler; delete an URL redirect after confirmation.
 *
 * @see redirect_delete_form()
 */
function redirect_delete_form_submit($form, &$form_state) {
  redirect_delete($form_state['values']['rid']);
  backdrop_set_message(t('The redirect has been deleted.'));
  $form_state['redirect'] = 'admin/config/search/redirect';
}

/**
 * Form builder for redirection settings.
 *
 * @see system_settings_form()
 * @see redirect_settings_form_submit()
 *
 * @ingroup forms
 */
function redirect_settings_form($form, &$form_state) {
  $config = config('redirect.settings');
  $form['redirect_auto_redirect'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically create redirects when URL aliases are changed.'),
    '#default_value' => $config->get('auto_redirect'),
    '#disabled' => !module_exists('path'),
  );
  $form['redirect_passthrough_querystring'] = array(
    '#type' => 'checkbox',
    '#title' => t('Retain query string through redirect.'),
    '#default_value' => $config->get('passthrough_querystring'),
    '#description' => t('For example, given a redirect from %source to %redirect, if a user visits %sourcequery they would be redirected to %redirectquery. The query strings in the redirection will always take precedence over the current query string.', array('%source' => 'source-path', '%redirect' => 'node?a=apples', '%sourcequery' => 'source-path?a=alligators&b=bananas', '%redirectquery' => 'node?a=apples&b=bananas')),
  );
  $form['redirect_warning'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display a warning message to users when they are redirected.'),
    '#default_value' => $config->get('warning'),
    '#access' => FALSE,
  );
  $form['redirect_default_status_code'] = array(
    '#type' => 'select',
    '#title' => t('Default redirect status'),
    '#description' => t('You can find more information about HTTP redirect status codes at <a href="@status-codes">@status-codes</a>.', array('@status-codes' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection')),
    '#options' => redirect_status_code_options(),
    '#default_value' => $config->get('default_status_code'),
  );
  $form['redirect_page_cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow redirects to be saved into the page cache.'),
    '#default_value' => $config->get('page_cache'),
    '#description' => t('This feature requires <a href="@performance">Cache pages for anonymous users</a> to be enabled and the %variable variable to be TRUE.', array('@performance' => url('admin/config/development/performance'), '%variable' => "\$settings['page_cache_invoke_hooks']")),
    '#disabled' => !$config->get('cache') || !settings_get('page_cache_invoke_hooks', FALSE),
  );
  $form['redirect_purge_inactive'] = array(
    '#type' => 'select',
    '#title' => t('Delete redirects that have not been accessed for'),
    '#default_value' => $config->get('purge_inactive'),
    '#options' => array(0 => t('Never (do not discard)')) + backdrop_map_assoc(array(604800, 1209600, 1814400, 2592000, 5184000, 7776000, 10368000, 15552000, 31536000), 'format_interval'),
    '#description' => t('Only redirects managed by the redirect module itself will be deleted. Redirects managed by other modules will be left alone.'),
    '#disabled' => $config->get('page_cache') && !settings_get('page_cache_invoke_hooks', FALSE),
  );

  $form['globals'] = array(
    '#type' => 'fieldset',
    '#title' => t('Always enabled redirections'),
    '#description' => t('(formerly Global Redirect features)'),
    '#access' => FALSE,
  );
  $form['globals']['redirect_global_home'] = array(
    '#type' => 'checkbox',
    '#title' => t('Redirect from paths like index.php and /node to the root directory.'),
    '#default_value' => $config->get('global_home'),
    '#access' => FALSE,
  );
  $form['globals']['redirect_global_clean'] = array(
    '#type' => 'checkbox',
    '#title' => t('Redirect from non-clean URLs to clean URLs.'),
    '#default_value' => $config->get('global_clean'),
    '#disabled' => !$config->get('clean_url'),
    '#access' => FALSE,
  );
  $form['globals']['redirect_global_canonical'] = array(
    '#type' => 'checkbox',
    '#title' => t('Redirect from non-canonical URLs to the canonical URLs.'),
    '#default_value' => $config->get('global_canonical'),
  );
  $form['globals']['redirect_global_deslash'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove trailing slashes from paths.'),
    '#default_value' => $config->get('global_deslash'),
    '#access' => FALSE,
  );
  $form['globals']['redirect_global_admin_paths'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow redirections on admin paths.'),
    '#default_value' => $config->get('global_admin_paths'),
  );
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['#submit'][] = 'redirect_settings_form_submit';
  return $form;
}

/**
 * Form submit handler; clears the page cache.
 *
 * @see redirect_settings_form()
 */
function redirect_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $config = config('redirect.settings');
  $config->set('auto_redirect', (bool) $values['redirect_auto_redirect']);
  $config->set('passthrough_querystring', (bool) $values['redirect_passthrough_querystring']);
  $config->set('warning', (bool) $values['redirect_warning']);
  $config->set('default_status_code', $values['redirect_default_status_code']);
  $config->set('page_cache', (bool) $values['redirect_page_cache']);
  $config->set('purge_inactive', (bool) $values['redirect_purge_inactive']);
  $config->set('global_home', (bool) $values['redirect_global_home']);
  $config->set('global_clean', (bool) $values['redirect_global_clean']);
  $config->set('global_canonical', (bool) $values['redirect_global_canonical']);
  $config->set('global_deslash', (bool) $values['redirect_global_deslash']);
  $config->set('global_admin_paths', (bool) $values['redirect_global_admin_paths']);
  $config->save();
  redirect_page_cache_clear();
  backdrop_set_message(t('Redirect configuration saved.'));
}

function redirect_404_list($form = NULL) {
  $destination = backdrop_get_destination();

  // Get filter keys and add the filter form.
  $keys = func_get_args();
  //$keys = array_splice($keys, 2); // Offset the $form and $form_state parameters.
  $keys = implode('/', $keys);
  $build['redirect_list_404_filter_form'] = backdrop_get_form('redirect_list_404_filter_form', $keys);

  $header = array(
    array('data' => t('Page'), 'field' => 'message'),
    array('data' => t('Count'), 'field' => 'count', 'sort' => 'desc'),
    array('data' => t('Last accessed'), 'field' => 'timestamp'),
    array('data' => t('Operations')),
  );

  $count_query = db_select('watchdog', 'w');
  $count_query->addExpression('COUNT(DISTINCT(w.message))');
  $count_query->leftJoin('redirect', 'r', 'w.message = r.source');
  $count_query->condition('w.type', 'page not found');
  $count_query->isNull('r.rid');
  redirect_build_filter_query($count_query, array('w.message'), $keys);

  $query = db_select('watchdog', 'w')->extend('PagerDefault')->extend('TableSort');
  $query->fields('w', array('message'));
  $query->addExpression('COUNT(wid)', 'count');
  $query->addExpression('MAX(timestamp)', 'timestamp');
  $query->leftJoin('redirect', 'r', 'w.message = r.source');
  $query->isNull('r.rid');
  $query->condition('w.type', 'page not found');
  $query->groupBy('w.message');
  $query->orderByHeader($header);
  $query->limit(25);
  redirect_build_filter_query($query, array('w.message'), $keys);
  $query->setCountQuery($count_query);
  $results = $query->execute();

  $rows = array();
  foreach ($results as $result) {
    $row = array();
    $row['source'] = l($result->message, $result->message, array('query' => $destination));
    $row['count'] = $result->count;
    $row['timestamp'] = format_date($result->timestamp, 'short');

    $operations = array();
    if (redirect_access('create', 'redirect')) {
      $operations['add'] = array(
        'title' => t('Add redirect'),
        'href' => 'admin/config/search/redirect/add/',
        'query' => array('source' => $result->message) + $destination,
      );
    }
    $row['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
      ),
    );

    $rows[] = $row;
  }

  $build['redirect_404_table']  = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No 404 pages without redirects found.'),
  );
  $build['redirect_404_pager'] = array('#theme' => 'pager');
  return $build;
}

/**
 * Return a form to filter URL redirects.
 *
 * @see redirect_list_filter_form_submit()
 *
 * @ingroup forms
 */
function redirect_list_404_filter_form($form, &$form_state, $keys = '') {
  $form['#attributes'] = array('class' => array('search-form'));
  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter 404s'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['basic']['filter'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => $keys,
    '#maxlength' => 128,
    '#size' => 25,
  );
  $form['basic']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#submit' => array('redirect_list_404_filter_form_submit'),
  );
  if ($keys) {
    $form['basic']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#submit' => array('redirect_list_404_filter_form_reset'),
    );
  }
  return $form;
}

/**
 * Process filter form submission when the Filter button is pressed.
 */
function redirect_list_404_filter_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/redirect/404/' . trim($form_state['values']['filter']);
}

/**
 * Process filter form submission when the Reset button is pressed.
 */
function redirect_list_404_filter_form_reset($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/redirect/404';
}

function redirect_list_table($redirects, $header) {
  $destination = backdrop_get_destination();
  $config = config('redirect.settings');
  $default_status_code = $config->get('default_status_code');

  // Set up the header.
  $header = array_combine($header, $header);
  $header = array_intersect_key(array(
    'source' => array('data' => t('From'), 'field' => 'source', 'sort' => 'asc'),
    'redirect' => array('data' => t('To'), 'field' => 'redirect'),
    'status_code' => array('data' => t('Type'), 'field' => 'status_code'),
    'language' => array('data' => t('Language'), 'field' => 'langcode'),
    'count' => array('data' => t('Count'), 'field' => 'count'),
    'access' => array('data' => t('Last accessed'), 'field' => 'access'),
    'operations' => array('data' => t('Operations')),
  ), $header);

  // Do not include the language column if locale is disabled.
  if (!module_exists('locale')) {
    unset($header['language']);
  }

  $rows = array();
  foreach ($redirects as $rid => $redirect) {
    $row = array();
    $redirect->source_options = array_merge($redirect->source_options, array('alias' => TRUE, 'language' => redirect_language_load($redirect->langcode)));
    $source_url = redirect_url($redirect->source, $redirect->source_options);
    $redirect_url = redirect_url($redirect->redirect, array_merge($redirect->redirect_options, array('alias' => TRUE)));
    $row['data']['source'] = l($source_url, $redirect->source, $redirect->source_options);
    $row['data']['redirect'] = l($redirect_url, $redirect->redirect, $redirect->redirect_options);
    $row['data']['status_code'] = $redirect->status_code ? $redirect->status_code : t('Default (@default)', array('@default' => $default_status_code));
    $row['data']['language'] = language_name($redirect->langcode);
    $row['data']['count'] = $redirect->count;
    if ($redirect->access) {
      $row['data']['access'] = array(
        'data' => t('!interval ago', array('!interval' => format_interval(REQUEST_TIME - $redirect->access))),
        'title' => t('Last accessed on @date', array('@date' => format_date($redirect->access))),
      );
    }
    else {
      $row['data']['access'] = t('Never');
    }

    // Mark redirects that override existing paths with a warning in the table.
    if (backdrop_valid_path($redirect->source)) {
      $row['class'][] = 'warning';
      $row['title'] = t('This redirect overrides an existing internal path.');
    }

    $operations = array();
    if (redirect_access('update', $redirect)) {
      $operations['edit'] = array(
        'title' => t('Edit'),
        'href' => 'admin/config/search/redirect/edit/' . $rid,
        'query' => $destination,
      );
    }
    if (redirect_access('delete', $redirect)) {
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/config/search/redirect/delete/' . $rid,
        'query' => $destination,
      );
    }
    $row['data']['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
      ),
    );

    $row['data'] = array_intersect_key($row['data'], $header);
    $rows[$rid] = $row;
  }

  $build['list'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No URL redirects available.'),
    '#attributes' => array('class' => array('redirect-list')),
  );

  return $build;
}
