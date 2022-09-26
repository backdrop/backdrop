<?php
/**
 * @file
 * Admin page callbacks for the Search module.
 */

/**
 * Menu callback: confirm rebuilding of the index.
 */
function search_reindex_confirm() {
  $form = confirm_form(array(),
  t('Are you sure you want to rebuild the search index?'),
  'admin/config/search/settings',
  t('This action will rebuild the whole search index. It may be a lengthy process.'),
  t('Rebuild search index'),
  t('Cancel'));

  // Change the css class to something less scary than button-danger.
  $form['actions']['submit']['#attributes']['class'][0] = 'button-primary';
  return $form;
}

/**
 * Handler for rebuild confirmation
 */
function search_reindex_confirm_submit(&$form, &$form_state) {
  if ($form['confirm']) {
    $config = config('search.settings');
    $limit_per_pass = $config->get('search_cron_limit');  

    // Wipe out all indexes.
    module_invoke_all('search_reset');

    // Find out how many items need to be reindexed.
    $remaining = 0;
    foreach ($config->get('search_active_modules') as $module) {
      if ($status = module_invoke($module, 'search_status')) {
        $remaining += $status['remaining'];
      }
    }

    // Determine how many passes are needed to reindex.
    $passes_needed = ceil($remaining / $limit_per_pass);
    $search_active_modules = config_get('search.settings', 'search_active_modules');

    // Create the batch if needed.
    if ($passes_needed && isset($search_active_modules)) {
      $batch = array(
        'title' => t('Rebuilding search index'),
        'operations' => array(
          array('search_reindex_batch', array($passes_needed, $search_active_modules)),
        ),
        'progress_message' => '',
        'finished' => 'search_reindex_batch_finished',
      );
      batch_set($batch);
    }

    $form_state['redirect'] = 'admin/config/search/settings';
  }
}

/**
 * Helper function to get real module names.
 */
function _search_get_module_names() {

  $search_info = search_get_info(TRUE);
  $system_info = system_get_info('module');
  $names = array();
  foreach ($search_info as $module => $info) {
    $names[$module] = $system_info[$module]['name'];
  }
  asort($names, SORT_STRING);
  return $names;
}

/**
 * Menu callback: displays the search module settings page.
 *
 * @ingroup forms
 *
 * @see search_admin_settings_validate()
 * @see search_admin_settings_submit()
 * @see search_admin_reindex_submit()
 */
function search_admin_settings($form, &$form_state) {
  $config = config('search.settings');

  $form['active'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search items')
  );
  $search_info = search_get_info(TRUE);
  $module_options = array();
  foreach ($search_info as $module => $info) {
    $module_options[$module] = $info['title'];
  }
  $form['active']['search_active_modules'] = array(
    '#title' => t('Available search items'),
    '#type' => 'checkboxes',
    '#default_value' => $config->get('search_active_modules'),
    '#options' => $module_options,
    '#description' => t('Several modules expose items to the search system. Use this setting to limit what can be searched.')
  );
  $form['active']['search_default_module'] = array(
    '#title' => t('Default search'),
    '#type' => 'radios',
    '#default_value' => $config->get('search_default_module'),
    '#options' => $module_options,
    '#description' => t('Only one type of item will be searched by default. This selection will determine what appears at <a href="@search">@search</a>.', array('@search' => url('search', array('absolute' => TRUE)))),
  );

  $form['logging'] = array(
    '#type' => 'fieldset',
    '#title' => t('Logging')
  );
  $form['logging']['search_logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log all searches'),
    '#default_value' => $config->get('search_logging'),
    '#description' => t('This setting may affect performance. Enable only if actively inspecting logs of search queries.'),
  );
  $form['#validate'][] = 'search_admin_settings_validate';
  $form['#submit'][] = 'search_admin_settings_submit';

  // Per module settings
  foreach ($config->get('search_active_modules') as $module) {
    $added_form = module_invoke($module, 'search_admin');
    if (is_array($added_form)) {
      $form = array_merge($form, $added_form);
    }
  }
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  // Indexing settings:
  $form['indexing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Indexing'),
  );

  // Collect some stats
  $remaining = 0;
  $total = 0;
  foreach ($config->get('search_active_modules') as $module) {
    if ($status = module_invoke($module, 'search_status')) {
      $remaining += $status['remaining'];
      $total += $status['total'];
    }
  }

  $count = format_plural($remaining, 'There is 1 item left to index.', 'There are @count items left to index.');
  $percentage = ((int)min(100, 100 * ($total - $remaining) / max(1, $total))) . '%';
  $status = '<p><strong>' . t('%percentage of the site has been indexed.', array('%percentage' => $percentage)) . ' ' . $count . '</strong></p>';

  $form['indexing']['status'] = array('#markup' => $status);
  $form['indexing']['wipe'] = array(
    '#type' => 'submit',
    '#value' => t('Rebuild search index'),
    '#submit' => array('search_admin_reindex_submit'),
    '#attributes' => array('class' => array('button-secondary')),
  );

  $items = backdrop_map_assoc(array(10, 20, 50, 100, 200, 500));
  $form['indexing']['search_cron_limit'] = array(
    '#type' => 'select',
    '#title' => t('Number of items to index during cron runs:'),
    '#default_value' => $config->get('search_cron_limit'),
    '#options' => $items,
    '#description' => t('The maximum number of items indexed in each pass of a <a href="@cron">cron maintenance task</a>. If necessary, reduce this number to prevent timeouts and memory errors while indexing.', array('@cron' => url('admin/reports/status')))
  );

  $form['indexing']['search_minimum_word_size'] = array(
    '#type' => 'number',
    '#title' => t('Minimum word length to index'),
    '#default_value' => $config->get('search_minimum_word_size'),
    '#min' => 1,
    '#max' => 10000,
    '#description' => t('The number of characters a word has to be to be indexed. A lower setting means better search result ranking, but also a larger database. Each search query must contain at least one keyword that is this size (or longer). <strong>Changing this setting will invalidate the search index.</strong>'),
  );
  $form['indexing']['transliteration'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transliterate search index and searched strings.'),
    '#description' => t('Enable to allow searching and indexing using US-ASCII character set, i.e. to treat accented and unaccented letters the same.'),
    '#default_value' => $config->get('transliteration'),
  );
  $form['indexing']['search_overlap_cjk'] = array(
    '#type' => 'checkbox',
    '#title' => t('Simple CJK handling'),
    '#default_value' => $config->get('search_overlap_cjk'),
    '#description' => t('Whether to apply a simple Chinese/Japanese/Korean tokenizer based on overlapping sequences. Turn this off if you want to use an external preprocessor for this instead. Does not affect other languages. <strong>Changing this setting will invalidate the search index.</strong>')
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

/**
 * Form validation handler for search_admin_settings().
 */
function search_admin_settings_validate($form, &$form_state) {
  // Check whether we selected a valid default.
  if ($form_state['triggering_element']['#value'] != t('Reset to defaults')) {
    $new_modules = array_filter($form_state['values']['search_active_modules']);
    $default = $form_state['values']['search_default_module'];
    if (!in_array($default, $new_modules, TRUE)) {
      form_set_error('search_default_module', t('Your default search module is not selected as an active module.'));
    }
  }
}

/**
 * Form submission handler for search_admin_settings().
 */
function search_admin_settings_submit($form, &$form_state) {
  $config = config('search.settings');
  // If these settings change, the index needs to be rebuilt.
  if (($config->get('search_minimum_word_size') != $form_state['values']['search_minimum_word_size']) || ($config->get('search_overlap_cjk') != $form_state['values']['search_overlap_cjk'])) {
    $config->set('search_minimum_word_size', $form_state['values']['search_minimum_word_size']);
    $config->set('search_overlap_cjk', $form_state['values']['search_overlap_cjk']);
    backdrop_set_message(t('The index will be rebuilt.'));
    search_reindex();
  }
  $config->set('search_cron_limit', $form_state['values']['search_cron_limit']);
  $config->set('search_default_module', $form_state['values']['search_default_module']);
  $config->set('search_logging', (bool) $form_state['values']['search_logging']);

  // Check whether we are resetting the values.
  if ($form_state['triggering_element']['#value'] == t('Reset to defaults')) {
    $new_modules = array('node', 'user');
  }
  else {
    $new_modules = array_filter($form_state['values']['search_active_modules']);
  }
  if ($config->get('search_active_modules') != $new_modules) {
    $config->set('search_active_modules', $new_modules);
    backdrop_set_message(t('The active search modules have been changed.'));
    state_set('menu_rebuild_needed', TRUE);
  }

  // Save all other settings provided by any other modules.
  form_state_values_clean($form_state);
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'search_') !== 0) {
      $config->set($key, $value);
    }
  }

  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form submission handler for reindex button on search_admin_settings_form().
 */
function search_admin_reindex_submit($form, &$form_state) {
  // send the user to the confirmation page
  $form_state['redirect'] = 'admin/config/search/settings/reindex';
}
