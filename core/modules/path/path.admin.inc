<?php
/**
 * @file
 * Administrative page callbacks for the path module.
 */

/**
 * Returns a listing of all defined URL aliases.
 *
 * When filter key passed, perform a standard search on the given key,
 * and return the list of matching URL aliases.
 */
function path_admin_overview($keys = NULL) {
  // Add the filter form above the overview table.
  $build['path_admin_filter_form'] = backdrop_get_form('path_admin_filter_form', $keys);
  // Enable language column if language.module is enabled or if we have any
  // alias with a language.
  $alias_exists = (bool) db_query_range('SELECT 1 FROM {url_alias} WHERE langcode <> :langcode', 0, 1, array(':langcode' => LANGUAGE_NONE))->fetchField();
  $multilanguage = (module_exists('language') || $alias_exists);

  $header = array();
  $header[] = array('data' => t('URL alias'), 'field' => 'alias', 'sort' => 'asc');
  $header[] = array('data' => t('System path'), 'field' => 'source');
  if ($multilanguage) {
    $header[] = array('data' => t('Language'), 'field' => 'langcode');
  }
  $header[] = t('Operations');

  $query = db_select('url_alias')->extend('PagerDefault')->extend('TableSort');
  if ($keys) {
    // Replace wildcards with PDO wildcards.
    $query->condition('alias', '%' . preg_replace('!\*+!', '%', $keys) . '%', 'LIKE');
  }
  $result = $query
    ->fields('url_alias')
    ->orderByHeader($header)
    ->limit(50)
    ->execute();

  $rows = array();
  $destination = backdrop_get_destination();
  foreach ($result as $data) {
    $row = array();
    $row['data']['alias'] = l($data->alias, $data->source);
    $row['data']['source'] = l($data->source, $data->source, array('alias' => TRUE));
    if ($multilanguage) {
      $row['data']['language_name'] = language_name($data->langcode);
    }

    $operations = array();
    $operations['edit'] = array(
      'title' => t('Edit'),
      'href' => "admin/config/search/path/edit/$data->pid",
      'query' => $destination,
    );
    $operations['delete'] = array(
      'title' => t('Delete'),
      'href' => "admin/config/search/path/delete/$data->pid",
      'query' => $destination,
    );
    $row['data']['operations'] = array(
      'data' => array(
        '#type' => 'operations',
        '#links' => $operations,
      ),
    );

    // If the system path maps to a different URL alias, highlight this table
    // row to let the user know of old aliases.
    if ($data->alias != backdrop_get_path_alias($data->source, $data->langcode)) {
      $row['class'] = array('warning');
    }

    $rows[] = $row;
  }

  $build['path_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No URL aliases available. <a href="@link">Add URL alias</a>.', array('@link' => url('admin/config/search/path/add'))),
  );
  $build['path_pager'] = array('#theme' => 'pager');

  return $build;
}

/**
 * Page callback: Returns a form creating or editing an alias.
 *
 * @param $path
 *   An array containing the path ID, source, alias, and language code.
 *
 * @return
 *   A form for adding or editing a URL alias.
 *
 * @see path_menu()
 */
function path_admin_edit($path = array()) {
  if ($path) {
    backdrop_set_title($path['alias']);
    $output = backdrop_get_form('path_admin_form', $path);
  }
  else {
    $output = backdrop_get_form('path_admin_form');
  }

  return $output;
}

/**
 * Form builder; Main URL pattern administration form.
 *
 * @param $path
 *   An array containing the path ID, source, alias, and language code.
 *
 * @ingroup forms
 * @see path_admin_form_validate()
 * @see path_admin_form_submit()
 * @see path_admin_form_delete_submit()
 */
function path_admin_form($form, &$form_state, $path = array('source' => '', 'alias' => '', 'langcode' => LANGUAGE_NONE, 'pid' => NULL)) {
  $form['source'] = array(
    '#type' => 'textfield',
    '#title' => t('Existing system path'),
    '#default_value' => $path['source'],
    '#maxlength' => 255,
    '#size' => 45,
    '#description' => t('Specify the existing path you wish to alias. For example: node/28 or taxonomy/term/1.'),
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (config_get('system.core', 'clean_url') ? '' : '?q='),
    '#required' => TRUE,
  );
  $form['alias'] = array(
    '#type' => 'textfield',
    '#title' => t('URL alias'),
    '#default_value' => $path['alias'],
    '#maxlength' => 255,
    '#size' => 45,
    '#description' => t('Specify an alternative path where this page will appear. Do not include a trailing slash.'),
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (config_get('system.core', 'clean_url') ? '' : '?q='),
    '#required' => TRUE,
  );

  // A hidden value unless language.module is enabled.
  if (module_exists('language')) {
    $languages = language_list(TRUE);
    foreach ($languages as $langcode => $language) {
      $language_options[$langcode] = $language->name;
    }

    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#options' => $language_options,
      '#empty_value' => LANGUAGE_NONE,
      '#empty_option' => t('- None -'),
      '#default_value' => $path['langcode'],
      '#weight' => -10,
      '#description' => t('A path alias set for a specific language will always be used when displaying this page in that language, and takes precedence over path aliases set as <em>- None -</em>.'),
    );
  }
  else {
    $form['langcode'] = array(
      '#type' => 'value',
      '#value' => $path['langcode']
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save URL alias'),
  );
  if ($path['pid']) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $path['pid'],
    );
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('path_admin_form_delete_submit'),
    );
  }

  return $form;
}

/**
 * Form submission handler for the 'Delete' button on path_admin_form().
 *
 * @see path_admin_form_validate()
 * @see path_admin_form_submit()
 */
function path_admin_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = backdrop_get_destination();
    unset($_GET['destination']);
  }
  $form_state['redirect'] = array('admin/config/search/path/delete/' . $form_state['values']['pid'], array('query' => $destination));
}

/**
 * Form validation handler for path_admin_form().
 *
 * @see path_admin_form_submit()
 * @see path_admin_form_delete_submit()
 */
function path_admin_form_validate($form, &$form_state) {
  $source = &$form_state['values']['source'];
  $source = backdrop_get_normal_path($source);
  $alias = $form_state['values']['alias'];
  $pid = isset($form_state['values']['pid']) ? $form_state['values']['pid'] : 0;
  // Language is only set if language.module is enabled, otherwise save for all
  // languages.
  $langcode = isset($form_state['values']['langcode']) ? $form_state['values']['langcode'] : LANGUAGE_NONE;

  $has_alias = db_query("SELECT COUNT(alias) FROM {url_alias} WHERE pid <> :pid AND alias = :alias AND langcode = :langcode", array(
      ':pid' => $pid,
      ':alias' => $alias,
      ':langcode' => $langcode,
    ))
    ->fetchField();

  if ($has_alias) {
    form_set_error('alias', t('The alias %alias is already in use in this language.', array('%alias' => $alias)));
  }
  if (!backdrop_valid_path($source)) {
    form_set_error('source', t("The path '@link_path' is either invalid or you do not have access to it.", array('@link_path' => $source)));
  }
}

/**
 * Form submission handler for path_admin_form().
 *
 * @see path_admin_form_validate()
 * @see path_admin_form_delete_submit()
 */
function path_admin_form_submit($form, &$form_state) {
  // Remove unnecessary values.
  form_state_values_clean($form_state);

  path_save($form_state['values']);

  backdrop_set_message(t('The alias has been saved.'));
  $form_state['redirect'] = 'admin/config/search/path';
}

/**
 * Form constructor for the path deletion form.
 *
 * @param $path
 *   The path alias that will be deleted.
 *
 * @see path_admin_delete_confirm_submit()
 */
function path_admin_delete_confirm($form, &$form_state, $path) {
  if (user_access('administer url aliases')) {
    $form_state['path'] = $path;
    return confirm_form(
      $form,
      t('Are you sure you want to delete path alias %title?',
      array('%title' => $path['alias'])),
      'admin/config/search/path'
    );
  }
  return array();
}

/**
 * Form submission handler for path_admin_delete_confirm().
 */
function path_admin_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    path_delete($form_state['path']['pid']);
    $form_state['redirect'] = 'admin/config/search/path';
  }
}

/**
 * Form constructor for the path admin overview filter form.
 *
 * @ingroup forms
 * @see path_admin_filter_form_submit_filter()
 * @see path_admin_filter_form_submit_reset()
 */
function path_admin_filter_form($form, &$form_state, $keys = '') {
  $form['#attributes'] = array('class' => array('search-form'));
  $form['basic'] = array('#type' => 'fieldset',
    '#title' => t('Filter aliases'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['basic']['filter'] = array(
    '#type' => 'search',
    '#title' => 'URL alias',
    '#title_display' => 'invisible',
    '#default_value' => $keys,
    '#maxlength' => 128,
    '#size' => 25,
  );
  $form['basic']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#submit' => array('path_admin_filter_form_submit_filter'),
    );
  if ($keys) {
    $form['basic']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#submit' => array('path_admin_filter_form_submit_reset'),
    );
  }
  return $form;
}

/**
 * Form submission handler for the path_admin_filter_form() Filter button.
 *
 * @see path_admin_filter_form_submit_reset()
 */
function path_admin_filter_form_submit_filter($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/path/list/' . trim($form_state['values']['filter']);
}

/**
 * Form submission handler for the path_admin_filter_form() Reset button.
 *
 * @see path_admin_filter_form_submit_filter()
 */
function path_admin_filter_form_submit_reset($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/search/path/list';
}

/**
 * Form builder; Configure the URL alias patterns.
 *
 * @see path_patterns_form_submit()
 * @ingroup forms
 */
function path_patterns_form($form, $form_state) {
  $config = config('path.settings');

  $form['help'] = array(
    '#markup' => '<p>' . t('URL patterns automatically alias new content based on wildcards called "tokens". For example the URL "node/10" might be automatically aliased to "blog/my-first-post" using the pattern "blog/[node:title]".') . '</p>',
  );

  $all_path_info = path_get_info();
  foreach ($all_path_info as $path_type => $path_info) {
    $pattern_description = $path_info['pattern description'];
    $group_header = t('!label URL patterns', array('!label' => $path_info['label']));
    $token_types = array();
    if (isset($path_info['entity type'])) {
      $entity_type = entity_get_info($path_info['entity type']);
      if (isset($entity_type['token type'])) {
        $token_types[] = $entity_type['token type'];
      }
    }

    $form[$path_type] = array(
      '#type' => 'fieldset',
      '#title' => $group_header,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    // Prompt for the default pattern for this path type.
    $variable = $path_type . '_pattern';
    $form[$path_type][$variable] = array(
      '#type' => 'textfield',
      '#title' => $pattern_description,
      '#default_value' => $config->get($variable),
      '#size' => 65,
      '#maxlength' => 1280,
      '#element_validate' => array('token_element_validate'),
      '#after_build' => array('token_element_validate'),
      '#token_types' => $token_types,
      '#min_tokens' => 1,
      '#parents' => array($variable),
    );

    // Set up specialized patterns for this path type (e.g. node types).
    if (isset($path_info['pattern items'])) {
      foreach ($path_info['pattern items'] as $item_name => $item_label) {
        $variable = $path_type . '_' . $item_name . '_pattern';
        $form[$path_type][$variable] = array(
          '#type' => 'textfield',
          '#title' => $item_label,
          '#default_value' => $config->get($variable),
          '#size' => 65,
          '#maxlength' => 1280,
          '#element_validate' => array('token_element_validate'),
          '#after_build' => array('token_element_validate'),
          '#token_types' => $token_types,
          '#min_tokens' => 1,
          '#parents' => array($variable),
        );
      }
    }

    // Display the user documentation of placeholders supported by this path
    // type as a description on the last pattern.
    $variables = array(
      'token_types' => $token_types,
      'text' => t('Browse available tokens for @type.', array('@type' => $path_info['label'])),
    );
    $form[$path_type]['#description'] = '<p>' . t('Each of the fields below support tokens.') . ' ' . theme('token_tree_link', $variables) . '</p>';
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Submit handler for path_patterns_form().
 */
function path_patterns_form_submit($form, &$form_state) {
  $config = config('path.settings');
  $values = $form_state['values'];

  $all_path_info = path_get_info();
  foreach ($all_path_info as $path_type => $path_info) {
    // Save default pattern per path type.
    $variable = $path_type . '_pattern';
    $config->set($variable, $values[$variable]);

    // Save specialized patterns per path type.
    if (isset($path_info['pattern items'])) {
      foreach ($path_info['pattern items'] as $item_name => $item_label) {
        $variable = $path_type . '_' . $item_name . '_pattern';
        $config->set($variable, $values[$variable]);
      }
    }
  }

  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form builder; Configure the URL pattern settings.
 *
 * @see path_patterns_settings_form_validate()
 * @see path_patterns_settings_form_submit()
 * @ingroup forms
 */
function path_patterns_settings_form($form) {
  module_load_include('inc', 'path');
  $config = config('path.settings');

  $form['verbose'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verbose'),
    '#default_value' => $config->get('verbose'),
    '#description' => t('Display alias changes (except during bulk updates).'),
  );

  $form['separator'] = array(
    '#type' => 'textfield',
    '#title' => t('Separator'),
    '#size' => 1,
    '#maxlength' => 1,
    '#default_value' => $config->get('separator'),
    '#description' => t('Character used to separate words in titles. This will replace any spaces and punctuation characters. Using a space or + character can cause unexpected results.'),
  );

  $form['case'] = array(
    '#type' => 'radios',
    '#title' => t('Character case'),
    '#default_value' => $config->get('case'),
    '#options' => array(
      PATH_CASE_LEAVE_ASIS => t('Leave case the same as source token values.'),
      PATH_CASE_LOWER => t('Change to lower case'),
    ),
  );

  $max_length = _path_get_schema_alias_maxlength();

  $form['max_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum alias length'),
    '#default_value' => $config->get('max_length'),
    '#step' => 1,
    '#min' => 1,
    '#max' => $max_length,
    '#description' => t('Maximum length of aliases to generate. 100 is the recommended length. @max is the maximum possible length.', array('@max' => $max_length)),
  );
  $form['max_component_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum component length'),
    '#default_value' => $config->get('max_component_length'),
    '#step' => 1,
    '#min' => 1,
    '#max' => $max_length,
    '#description' => t('Maximum text length of any component in the alias (e.g., [title]). 100 is the recommended length. @max is the maximum possible length.', array('@max' => $max_length)),
  );

  $description = t('What should be done when updating an existing content item which already has an alias?');
  if (module_exists('redirect')) {
    $description .= ' ' . t('The <a href="!url">Redirect module settings</a> affect whether a redirect is created when an alias is deleted.', array('!url' => url('admin/config/search/redirect/settings')));
  }
  else {
    $description .= ' ' . t('Considering installing the <a href="!url">Redirect module</a> to get redirects when your aliases change.', array('!url' => 'https://github.com/backdrop-contrib/redirect'));
  }
  $form['update_action'] = array(
    '#type' => 'radios',
    '#title' => t('Update action'),
    '#default_value' => $config->get('update_action'),
    '#options' => array(
      PATH_UPDATE_ACTION_NO_NEW => t('Do nothing. Leave the old alias intact.'),
      PATH_UPDATE_ACTION_LEAVE => t('Create a new alias. Leave the existing alias functioning.'),
      PATH_UPDATE_ACTION_DELETE => t('Create a new alias. Delete the old alias.'),
    ),
    '#description' => $description,
  );

  $form['transliterate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transliterate prior to creating alias'),
    '#default_value' => $config->get('transliterate'),
    '#description' => t('When a pattern includes certain characters (such as those with accents) should these characters be converted into the US-ASCII alphabet?'),
  );

  $form['reduce_ascii'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reduce strings to letters and numbers'),
    '#default_value' => $config->get('reduce_ascii'),
    '#description' => t('Filters the new alias to only letters and numbers found in the ASCII-96 set.'),
  );

  $form['ignore_words'] = array(
    '#type' => 'textarea',
    '#title' => t('Strings to Remove'),
    '#default_value' => $config->get('ignore_words'),
    '#description' => t('Words to strip out of the URL alias, separated by commas. Do not use this to remove punctuation.'),
    '#wysiwyg' => FALSE,
  );

  $form['punctuation'] = array(
    '#type' => 'fieldset',
    '#title' => t('Punctuation'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $punctuation = path_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $details['default'] = PATH_PUNCTUATION_REMOVE;
    if ($details['value'] == $config->get('separator')) {
      $details['default'] = PATH_PUNCTUATION_REPLACE;
    }
    $form['punctuation']['punctuation_' . $name] = array(
      '#type' => 'select',
      '#title' => $details['name'] . ' (<code>' . check_plain($details['value']) . '</code>)',
      '#default_value' => ($config->get('punctuation_' . $name)) ? $config->get('punctuation_' . $name): $details['default'],
      '#options' => array(
        PATH_PUNCTUATION_REMOVE => t('Remove'),
        PATH_PUNCTUATION_REPLACE => t('Replace by separator'),
        PATH_PUNCTUATION_DO_NOTHING => t('No action (do not replace)'),
      ),
    );
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validate handler for path_patterns_settings_form().
 */
function path_patterns_settings_form_validate($form, &$form_state) {
  module_load_include('inc', 'path');

  // Perform a basic check for HTML characters in the strings to remove field.
  if (strip_tags($form_state['values']['ignore_words']) != $form_state['values']['ignore_words']) {
    form_set_error('ignore_words', t('The <em>Strings to remove</em> field must not contain HTML. Make sure to disable any WYSIWYG editors for this field.'));
  }

  // Validate that the separator is not set to be removed per http://drupal.org/node/184119
  // This isn't really all that bad so warn, but still allow them to save the value.
  $separator = $form_state['values']['separator'];
  $punctuation = path_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    if ($details['value'] == $separator) {
      $action = $form_state['values']['punctuation_' . $name];
      if ($action == PATH_PUNCTUATION_REMOVE) {
        backdrop_set_message(t('You have configured the @name to be the separator and to be removed when encountered in strings. This can cause problems with your patterns and especially with the term:path token. You should probably set the action for @name to be "replace by separator".', array('@name' => $details['name'])), 'error');
      }
    }
  }
}

/**
 * Submit handler for path_patterns_settings_form().
 */
function path_patterns_settings_form_submit($form, &$form_state) {
  $config = config('path.settings');
  $values = $form_state['values'];
  $config->set('verbose', $values['verbose']);
  $config->set('separator', $values['separator']);
  $config->set('case', $values['case']);
  $config->set('max_length', $values['max_length']);
  $config->set('max_component_length', $values['max_component_length']);
  $config->set('update_action', $values['update_action']);
  $config->set('transliterate', $values['transliterate']);
  $config->set('reduce_ascii', $values['reduce_ascii']);
  $config->set('ignore_words', $values['ignore_words']);

  $punctuation = path_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $config->set('punctuation_' . $name, $values['punctuation_' . $name]);
  }

  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form builder; Bulk path alias update form.
 *
 * @see path_bulk_update_form_submit()
 * @ingroup forms
 */
function path_bulk_update_form() {
  $form['#update_callbacks'] = array();

  $form['generate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bulk generate'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  $form['generate']['update'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Generate URL aliases for the following types of un-aliased paths:'),
    '#options' => array(),
    '#default_value' => array(),
  );

  $all_path_info = path_get_info();
  foreach ($all_path_info as $path_info) {
    if (!empty($path_info['batch update callback'])) {
      $form['#update_callbacks'][$path_info['batch update callback']] = $path_info;
      $form['generate']['update']['#options'][$path_info['batch update callback']] = t('!label URL aliases', array('!label' => $path_info['label']));
    }
  }

  $form['generate']['help'] = array(
    '#markup' => '<p>' . t('URL aliases will only be created for items that do not already have URL aliases.') . '</p>',
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate URL aliases'),
  );

  return $form;
}

/**
 * Form submit handler for path alias bulk update form.
 *
 * @see path_bulk_update_form()
 * @see path_bulk_update_batch_finished()
 */
function path_bulk_update_form_submit($form, &$form_state) {
  $batch = array(
    'title' => t('Bulk updating URL aliases'),
    'operations' => array(
      array('path_bulk_update_batch_start', array()),
    ),
    'finished' => 'path_bulk_update_batch_finished',
    'file' => backdrop_get_path('module', 'path') . '/path.admin.inc',
  );

  foreach ($form_state['values']['generate']['update'] as $callback) {
    if (!empty($callback)) {
      $path_info = $form['#update_callbacks'][$callback];
      if (!empty($path_info['batch file'])) {
        $batch['operations'][] = array('path_bulk_update_batch_process', array($callback, $path_info));
      }
      else {
        $batch['operations'][] = array($callback, array());
      }
    }
  }

  batch_set($batch);
}

/**
 * Batch callback; count the current number of URL aliases for comparison later.
 */
function path_bulk_update_batch_start(&$context) {
  $context['results']['count_before'] = db_select('url_alias')->countQuery()->execute()->fetchField();
}

/**
 * Common batch processing callback for all operations.
 *
 * Required to load our include the proper batch file.
 */
function path_bulk_update_batch_process($callback, $path_info, &$context) {
  if (!empty($path_info['batch file'])) {
    require_once BACKDROP_ROOT . '/' . $path_info['batch file path'] . '/' . $path_info['batch file'];
  }
  return $callback($context);
}

/**
 * Batch finished callback.
 */
function path_bulk_update_batch_finished($success, $results, $operations) {
  if ($success) {
    // Count the current number of URL aliases after the batch is completed
    // and compare to the count before the batch started.
    $results['count_after'] = db_select('url_alias')->countQuery()->execute()->fetchField();
    $results['count_changed'] = max($results['count_after'] - $results['count_before'], 0);
    if ($results['count_changed']) {
      backdrop_set_message(format_plural($results['count_changed'], 'Generated 1 URL alias.', 'Generated @count URL aliases.'));
    }
    else {
      backdrop_set_message(t('No new URL aliases to generate.'));
    }
  }
  else {
    $error_operation = reset($operations);
    backdrop_set_message(t('An error occurred while processing @operation with arguments : @args', array('@operation' => $error_operation[0], '@args' => print_r($error_operation[0], TRUE))));
  }
}

/**
 * Menu callback; select certain alias types to delete.
 */
function path_admin_bulk_delete($form, $form_state) {
  $all_path_info = path_get_info();

  // Confirmation form action.
  if (isset($form_state['values']['delete'])) {
    $delete_list = array();
    foreach ($form_state['values']['delete'] as $key => $value) {
      if ($value) {
        $label = $key === 'all_aliases' ? t('All URL aliases') : $all_path_info[$key]['label'];
        $delete_list[] = $label;
      }
    }
    $form['delete'] = array(
      '#type' => 'value',
      '#value' => $form_state['values']['delete'],
    );
    $form['delete_list'] = array(
      '#markup' => theme('item_list', array('items' => $delete_list)),
    );

    $question = t('Delete all the following aliases?');
    $path = 'admin/config/search/path/bulk-delete';
    $description = t('Deleting these aliases cannot be undone.');
    $yes = t('Delete aliases');
    return confirm_form($form, $question, $path, $description, $yes);
  }

  /* TODO:
   1) all - DONE
   2) all node aliases - DONE
   4) all user aliases - DONE
   5) all taxonomy aliases - DONE
   6) by node type
   7) by taxonomy vocabulary
   8) no longer existing aliases (see http://drupal.org/node/128366)
   9) where src like 'pattern' - DON'T DO
   10) where dst like 'pattern' - DON'T DO
  */

  $form['delete'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose aliases to bulk delete'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  // First we do the "all" case.
  $total_count = db_query('SELECT count(1) FROM {url_alias}')->fetchField();
  $form['delete']['all_aliases'] = array(
    '#type' => 'checkbox',
    '#title' => t('All URL aliases'),
    '#default_value' => FALSE,
    '#description' => t('Number of aliases which will be deleted: %count.', array('%count' => $total_count)),
  );

  // Next, provide checkboxes for each alias type.
  foreach ($all_path_info as $internal_name => $path_info) {
    $count = db_query("SELECT count(1) FROM {url_alias} WHERE source LIKE :src", array(':src' => "$internal_name%"))->fetchField();
    $form['delete'][$internal_name] = array(
      '#type' => 'checkbox',
      '#title' => t('!label URL aliases', array('!label' => $path_info['label'])),
      '#default_value' => FALSE,
      '#description' => t('Number of aliases which will be deleted: %count.', array('@label' => $path_info['label'], '%count' => $count)),
    );
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete URL aliases'),
    '#submit' => array('path_admin_bulk_delete_confirm_submit'),
  );

  return $form;
}

/**
 * Submit handler for path_admin_bulk_delete().
 */
function path_admin_bulk_delete_confirm_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for path_admin_bulk_delete().
 */
function path_admin_bulk_delete_submit($form, &$form_state) {
  foreach ($form_state['values']['delete'] as $key => $value) {
    if ($value) {
      if ($key === 'all_aliases') {
        db_delete('url_alias')
          ->execute();
        backdrop_set_message(t('All of your path aliases have been deleted.'));
      }
      $all_path_info = path_get_info();
      if (array_key_exists($key, $all_path_info)) {
        $path_prefix = isset($all_path_info[$key]['source prefix']) ? $all_path_info[$key]['source prefix'] : $key . '/';
        db_delete('url_alias')
          ->condition('source', db_like($path_prefix) . '%', 'LIKE')
          ->execute();
        backdrop_set_message(t('All aliases for %type have been deleted.', array('%type' => $all_path_info[$key]['label'])));
      }
    }
  }
  $form_state['redirect'] = 'admin/config/search/path/bulk-delete';
}
