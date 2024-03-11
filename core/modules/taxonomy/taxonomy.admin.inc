<?php
/**
 * @file
 * Admin page callbacks for the Taxonomy module.
 */

/**
 * Page callback at admin/structure/taxonomy to list all available vocabularies.
 */
function taxonomy_overview_vocabularies() {
  $vocabularies = taxonomy_vocabulary_load_multiple(FALSE);

  $header = array(
    array('data' => t('Vocabulary name'), 'class' => array('vocabulary-name')),
    array('data' => t('Description'), 'class' => array('vocabulary-description', 'priority-low')),
    array('data' => t('Number of terms'), 'class' => array('vocabulary-term-count', 'priority-medium')),
    array('data' => t('Operations'), 'class' => array('vocabulary-operations')),
  );

  $rows = array();

  foreach ($vocabularies as $vocabulary) {
    $term_count = db_query("SELECT count(tid) FROM {taxonomy_term_data} WHERE vocabulary = :vocab", array(':vocab' => $vocabulary->machine_name))->fetchField();
    if (taxonomy_vocabulary_access(FALSE, $vocabulary)) {
      $row = array();
      $row[] = array(
        'data' => theme('label_machine_name__vocabulary', array(
          'label' => $vocabulary->name,
          'machine_name' => $vocabulary->machine_name,
        )),
        'class' => array('vocabulary-name'),
        'data-label' => t('Vocabulary name'),
      );
      $row[] = array(
        'data' => theme('taxonomy_vocabulary_description', array('vocabulary' => $vocabulary)),
        'class' => array('vocabulary-description'),
        'data-label' => t('Description'),
      );
      $row[] = array(
        'data' => $term_count,
        'class' => array('vocabulary-term-count'),
        'data-label' => t('Number of terms'),
      );
      $row[] = array(
        'data' => array(
          '#type' => 'operations',
          '#links' => _vocabulary_get_operations($vocabulary),
        ),
        'class' => array('vocabulary-operations'),
        'data-label' => t('Operations'),
      );

      $rows[] = $row;
    }
  }

  backdrop_add_css(backdrop_get_path('module', 'taxonomy') . '/css/taxonomy.css');

  $table = array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No vocabularies available. <a href="@link">Add vocabulary</a>.', array('@link' => url('admin/structure/taxonomy/add'))),
    'attributes' => array(
      'id' => 'vocabulary-list',
      'class' => array('vocabulary-list'),
    ),
  );

  return theme('table', $table);
}

/**
 * Given a vocabulary, return a list of operations that can be performed on it.
 */
function _vocabulary_get_operations($vocabulary) {
  $links = array();

  // Allow access to the terms listing for the vocabulary if the user has:
  // - any of the create/update (edit)/delete permissions,
  // - or the the 'administer taxonomy' permission.
  if (taxonomy_vocabulary_access(FALSE, $vocabulary)) {
    $links['list'] = array(
      'title' => t('List terms'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name,
    );
  }
  if (taxonomy_vocabulary_access('create', $vocabulary)) {
    $links['add'] = array(
      'title' => t('Add terms'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/add',
    );
  }
  if (user_access('administer taxonomy')) {
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/configure',
    );
  }
  if (module_exists('field_ui') && user_access('administer fields')) {
    $links['fields'] = array(
      'title' => t('Manage fields'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/fields',
      'weight' => 5,
    );
    $links['display'] = array(
      'title' => t('Manage displays'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/display',
      'weight' => 10,
      );
    }
  if (module_exists('config') && user_access('synchronize configuration')) {
    $links['export'] = array(
      'title' => t('Export'),
      'href' => 'admin/config/development/configuration/single/export',
      'query' => array(
        'group' => 'Vocabularies',
        'name' => 'taxonomy.vocabulary.' . $vocabulary->machine_name,
      ),
    );
  }
  if (user_access('administer taxonomy')) {
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/delete',
    );
  }

  return $links;
}

/**
 * Form builder for the vocabulary editing form.
 *
 * @ingroup forms
 * @see taxonomy_form_vocabulary_submit()
 * @see taxonomy_form_vocabulary_validate()
 */
function taxonomy_form_vocabulary($form, &$form_state, TaxonomyVocabulary $vocabulary = NULL) {
  // During initial form build, add the entity to the form state for use
  // during form building and processing. During a rebuild, use what is in the
  // form state.
  if (!isset($form_state['vocabulary'])) {
    // Create a new TaxonomyVocabulary entity for the add form.
    if (!isset($vocabulary)) {
      backdrop_set_title(t('Add vocabulary'));
      $vocabulary = new TaxonomyVocabulary();
    }
    $form_state['vocabulary'] = $vocabulary;
  }
  else {
    $vocabulary = $form_state['vocabulary'];
  }

  $form['#vocabulary'] = $form_state['vocabulary'];

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $vocabulary->name,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#default_value' => $vocabulary->machine_name,
    '#maxlength' => 255,
    '#disabled' => !empty($vocabulary->machine_name),
    '#machine_name' => array(
      'exists' => 'taxonomy_vocabulary_load',
    ),
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => $vocabulary->description,
  );
  // Set the hierarchy to "multiple parents" by default. This simplifies the
  // vocabulary form and standardizes the term form.
  $form['hierarchy'] = array(
    '#type' => 'value',
    '#value' => '0',
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'taxonomy') . '/js/taxonomy.vocabulary.js'),
    ),
    '#weight' => 99,
  );

  // Vocabulary permissions.
  $form['permissions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Permissions'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#access' => user_access('administer permissions'),
  );
  $form['permissions']['permissions'] = taxonomy_vocabulary_form_permissions($vocabulary);

  if (module_exists('language')) {
    $form['multilingual'] = array(
      '#type' => 'fieldset',
      '#title' => t('Multilingual support'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'additional_settings',
    );
    $form['multilingual']['language'] = array(
      '#type' => 'checkbox',
      '#title' => t('Multilingual support'),
      '#default_value' => $vocabulary->language,
      '#description' => t('Add a language selection field to the editing form, allowing you to select from one of the <a href="!languages">enabled languages</a>. If disabled, new terms are saved with no defined language. Existing terms will not be affected by changing this option.', array('!languages' => url('admin/config/regional/language'))),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save vocabulary'),
    '#weight' => 2,
  );

  // Only show 'Save and add terms' button when creating new vocabulary.
  if ($vocabulary->machine_name == NULL) {
    $form['actions']['continue'] = array(
      '#type' => 'submit',
      '#value' => t('Save and add terms'),
      '#weight' => 1,
    );
  }

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/structure/taxonomy',
    '#options' => array('attributes' => array('class' => array('form-cancel'))),
    '#weight' => 15,
  );

  return $form;
}

/**
 * Builds a matrix of node permissions for this node type.
 *
 * @param $vocabulary
 *   Fully loaded TaxonomyVocabulary object.
 *
 * @see taxonomy_form_vocabulary()
 */
function taxonomy_vocabulary_form_permissions($vocabulary) {
  // Retrieve role names for columns.
  $roles = user_roles(FALSE, NULL, TRUE);
  $admin_role = config_get('system.core', 'user_admin_role');

  // Note that in taxonomy_theme() the 'node_type_form_permissions' theme
  // callback reuses theme_user_admin_permissions(), to make it identical to
  // the user module permissions page.
  $form['#theme'] = array('node_type_form_permissions');

  // Wrap the table in a form element to make margins match other form elements.
  $form['#theme_wrappers'] = array('form_element');

  $form['roles'] = array(
    '#type' => 'value',
    '#value' => $roles,
  );

  // Render role/permission overview:
  $options = array();

  $permissions = taxonomy_vocabulary_list_permissions($vocabulary);

  $status = array();
  foreach ($permissions as $perm => $perm_item) {
    // Fill in default values for the permission.
    $perm_item += array(
      'description' => '',
      'restrict access' => FALSE,
      'warning' => '',
    );
    $options[$perm] = '';
    $parts = explode(': ', $perm_item['title']);
    $form['permission'][$perm] = array(
      '#type' => 'item',
      '#markup' => $parts[1],
      '#description' => theme('user_permission_description', array('permission_item' => $perm_item)),
    );
    foreach ($roles as $role_name => $role) {
      // Builds arrays for checked boxes for each role. Always select the Admin
      // role on new forms.
      if (in_array($perm, $role->permissions) || (!$vocabulary->machine_name && $role->name == $admin_role)) {
        $status[$role_name][] = $perm;
      }
    }
  }

  // Have to build checkboxes here after checkbox arrays are built.
  foreach ($roles as $role_name => $role) {
    $form['checkboxes'][$role_name] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => isset($status[$role_name]) ? $status[$role_name] : array(),
      '#attributes' => array('class' => array('role-' . $role_name)),
    );
    $form['role_names'][$role_name] = array(
      '#markup' => check_plain($role->label),
      '#description' => filter_xss_admin($role->description),
      '#tree' => TRUE,
    );
  }

  $form['#attached']['js'][] = backdrop_get_path('module', 'user') . '/js/user.permissions.js';

  return $form;
}

/**
 * Form validation handler for taxonomy_form_vocabulary().
 *
 * Makes sure that the machine name of the vocabulary is not in the
 * disallowed list (names that conflict with menu items, such as 'list'
 * and 'add').
 *
 * @see taxonomy_form_vocabulary()
 * @see taxonomy_form_vocabulary_submit()
 */
function taxonomy_form_vocabulary_validate($form, &$form_state) {
  // During the deletion there is no 'machine_name' key
  if (isset($form_state['values']['machine_name'])) {
    // Do not allow machine names to conflict with taxonomy path arguments.
    $machine_name = $form_state['values']['machine_name'];
    $disallowed = array('add', 'list');
    if (in_array($machine_name, $disallowed)) {
      form_set_error('machine_name', t('The machine-readable name cannot be "add" or "list".'));
    }
  }
}

/**
 * Form submission handler for taxonomy_form_vocabulary().
 *
 * @see taxonomy_form_vocabulary()
 * @see taxonomy_form_vocabulary_validate()
 */
function taxonomy_form_vocabulary_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Delete')) {
    // Rebuild the form to confirm vocabulary deletion.
    $form_state['rebuild'] = TRUE;
    $form_state['confirm_delete'] = TRUE;
    return;
  }

  $vocabulary = $form_state['vocabulary'];
  form_state_values_clean($form_state);
  foreach ($form_state['values'] as $key => $value) {
    if (property_exists($vocabulary, $key)) {
      $vocabulary->$key = $value;
    }
  }

  // Prevent leading and trailing spaces in vocabulary names.
  $vocabulary->name = trim($vocabulary->name);

  switch (taxonomy_vocabulary_save($vocabulary)) {
    case SAVED_NEW:
      backdrop_set_message(t('Created new vocabulary %name.', array('%name' => $vocabulary->name)));
      watchdog('taxonomy', 'Created new vocabulary %name.', array('%name' => $vocabulary->name), WATCHDOG_NOTICE, l(t('configure'), 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/configure'));
      $form_state['redirect'] = 'admin/structure/taxonomy';
      // Save new permissions
      foreach ($form_state['values']['roles'] as $role_name => $role) {
        $valid_perms = array();
        // Create the permissions string in the format 'edit terms in $vocab'.
        foreach ($form_state['values'][$role->name] as $string => $perm) {
          if ($perm) {
            $parts = explode(' terms in ', $string);
            $valid_perms[] = $parts[0] . ' terms in ' . $vocabulary->machine_name;
          }
        }
        user_role_grant_permissions($role->name, $valid_perms);
      }

      if ($form_state['triggering_element']['#value'] == t('Save and add terms')) {
        $form_state['redirect'] = 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/add';
      }
      break;

    case SAVED_UPDATED:
      backdrop_set_message(t('Updated vocabulary %name.', array('%name' => $vocabulary->name)));
      watchdog('taxonomy', 'Updated vocabulary %name.', array('%name' => $vocabulary->name), WATCHDOG_NOTICE, l(t('configure'), 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/configure'));
      $form_state['redirect'] = 'admin/structure/taxonomy';
      // Update permissions.
      foreach ($form_state['values']['roles'] as $role_name => $role) {
        user_role_change_permissions($role->name, $form_state['values'][$role->name]);
      }
      break;
  }
}

/**
 * Form builder for the taxonomy terms overview.
 *
 * Display a tree of all the terms in a vocabulary, with options to edit
 * each one. The form is made drag and drop by the theme function.
 *
 * @param TaxonomyVocabulary $vocabulary
 *   The taxonomy vocabulary entity to list terms for.
 *
 * @ingroup forms
 * @see taxonomy_overview_terms_submit()
 * @see theme_taxonomy_overview_terms()
 */
function taxonomy_overview_terms($form, &$form_state, TaxonomyVocabulary $vocabulary) {
  global $pager_page_array, $pager_total, $pager_total_items, $language;

  // Check for confirmation forms.
  if (isset($form_state['confirm_reset_alphabetical'])) {
    return taxonomy_vocabulary_confirm_reset_alphabetical($form, $form_state, $vocabulary->machine_name);
  }

  $filter_langcode = isset($_GET['langcode_term']) ? $_GET['langcode_term'] : $language->langcode;
  if (module_exists('language') && $vocabulary->language == TAXONOMY_LANGUAGE_ENABLED) {
    $form['langcode_term'] = array(
      '#title' => t('Show terms in language'),
      '#type' => 'select',
      '#options' => language_list(TRUE, TRUE),
      '#empty_value' => LANGUAGE_NONE,
      '#empty_option' => t('All languages'),
      '#default_value' => $filter_langcode,
      '#access' => module_exists('language') && $vocabulary->language == TAXONOMY_LANGUAGE_ENABLED,
      '#weight' => -20,
      '#description' => t('Show terms for a given language. Terms with the "All" language will be shown in every language.'),
    );
    $form['langcode_filter'] = array(
      '#type' => 'submit',
      '#value' => t('Filter'),
      '#attributes' => array('class' => array('button-primary')),
      '#submit' => array('term_overview_form_language_filter_submit'),
    );
  }

  $form['#vocabulary'] = $vocabulary;
  $form['#tree'] = TRUE;
  $form['#parent_fields'] = FALSE;

  $page            = isset($_GET['page']) ? $_GET['page'] : 0;
  $page_increment  = config_get('taxonomy.settings', 'terms_per_page_admin');  // Number of terms per page.
  $page_entries    = 0;   // Elements shown on this page.
  $before_entries  = 0;   // Elements at the root level before this page.
  $after_entries   = 0;   // Elements at the root level after this page.
  $root_entries    = 0;   // Elements at the root level on this page.

  // Terms from previous and next pages are shown if the term tree would have
  // been cut in the middle. Keep track of how many extra terms we show on each
  // page of terms.
  $back_step    = NULL;
  $forward_step = 0;

  // An array of the terms to be displayed on this page.
  $current_page = array();

  $delta = 0;
  $term_deltas = array();
  $tree = taxonomy_get_tree($vocabulary->machine_name);
  $term = current($tree);
  do {
    // In case this tree is completely empty.
    if (empty($term)) {
      break;
    }

    // Skip all terms not in the requested language.
    $tree_langcode = $filter_langcode === LANGUAGE_NONE ? NULL : $filter_langcode;
    if ($tree_langcode && $term->langcode !== $tree_langcode && $term->langcode !== LANGUAGE_NONE) {
      continue;
    }

    $delta++;
    // Count entries before the current page.
    if ($page && ($page * $page_increment) > $before_entries && !isset($back_step)) {
      $before_entries++;
      continue;
    }
    // Count entries after the current page.
    elseif ($page_entries > $page_increment && isset($complete_tree)) {
      $after_entries++;
      continue;
    }

    // Do not let a term start the page that is not at the root.
    if (isset($term->depth) && ($term->depth > 0) && !isset($back_step)) {
      $back_step = 0;
      while ($previous_term = prev($tree)) {
        $before_entries--;
        $back_step++;
        if ($previous_term->depth == 0) {
          prev($tree);
          continue 2; // Jump back to the start of the root level parent.
       }
      }
    }
    $back_step = isset($back_step) ? $back_step : 0;

    // Continue rendering the tree until we reach the a new root item.
    if ($page_entries >= $page_increment + $back_step + 1 && $term->depth == 0 && $root_entries > 1) {
      $complete_tree = TRUE;
      // This new item at the root level is the first item on the next page.
      $after_entries++;
      continue;
    }
    if ($page_entries >= $page_increment + $back_step) {
      $forward_step++;
    }

    // Finally, if we've gotten down this far, we're rendering a term on this page.
    $page_entries++;
    $term_deltas[$term->tid] = isset($term_deltas[$term->tid]) ? $term_deltas[$term->tid] + 1 : 0;
    $key = 'tid:' . $term->tid . ':' . $term_deltas[$term->tid];

    // Keep track of the first term displayed on this page.
    if ($page_entries == 1) {
      $form['#first_tid'] = $term->tid;
    }
    // Keep a variable to make sure at least 2 root elements are displayed.
    if ($term->parents[0] == 0) {
      $root_entries++;
    }
    $current_page[$key] = $term;
  } while ($term = next($tree));

  // Because we didn't use a pager query, set the necessary pager variables.
  $total_entries = $before_entries + $page_entries + $after_entries;
  $pager_total_items[0] = $total_entries;
  $pager_page_array[0] = $page;
  $pager_total[0] = ceil($total_entries / $page_increment);

  // If this form was already submitted once, it's probably hit a validation
  // error. Ensure the form is rebuilt in the same order as the user submitted.
  if (!empty($form_state['input'])) {
    $order = array_flip(array_keys($form_state['input'])); // Get the $_POST order.
    $current_page = array_merge($order, $current_page); // Update our form with the new order.
    foreach ($current_page as $key => $term) {
      // Verify this is a term for the current page and set at the current depth.
      if (is_array($form_state['input'][$key]) && is_numeric($form_state['input'][$key]['tid'])) {
        $current_page[$key]->depth = $form_state['input'][$key]['depth'];
      }
      else {
        unset($current_page[$key]);
      }
    }
  }

  $language_options = language_list(TRUE, TRUE);
  $language_options[LANGUAGE_NONE] = t('All');

  // Build the actual form.
  foreach ($current_page as $key => $term) {
    // Save the term for the current page so we don't have to load it a second time.
    $form[$key]['#term'] = (array) $term;
    if (isset($term->parents)) {
      $form[$key]['#term']['parent'] = $term->parent = $term->parents[0];
      unset($form[$key]['#term']['parents'], $term->parents);
    }

    $form[$key]['view'] = array('#type' => 'link', '#title' => $term->name, '#href' => "taxonomy/term/$term->tid");
    if ($vocabulary->hierarchy != TAXONOMY_HIERARCHY_MULTIPLE && count($tree) > 1) {
      $form['#parent_fields'] = TRUE;
      $form[$key]['tid'] = array(
        '#type' => 'hidden',
        '#value' => $term->tid
      );
      $form[$key]['parent'] = array(
        '#type' => 'hidden',
        // Yes, default_value on a hidden. It needs to be changeable by the javascript.
        '#default_value' => $term->parent,
      );
      $form[$key]['depth'] = array(
        '#type' => 'hidden',
        // Same as above, the depth is modified by javascript, so it's a default_value.
        '#default_value' => $term->depth,
      );
      // Language
      $langcode_label = isset($language_options[$term->langcode]) ? $language_options[$term->langcode] : $term->langcode;
      if (module_exists('language') && $vocabulary->language == TAXONOMY_LANGUAGE_ENABLED) {
        $form[$key]['langcode'] = array(
          '#type' => 'markup',
          '#markup' => check_plain($langcode_label),
          '#access' => module_exists('language') && $vocabulary->language == TAXONOMY_LANGUAGE_ENABLED,
        );
      }
      if (taxonomy_vocabulary_access('update', $vocabulary)) {
        $form[$key]['weight'] = array(
          '#type' => 'weight',
          '#delta' => $delta,
          '#title_display' => 'invisible',
          '#title' => t('Weight for added term'),
          '#default_value' => $term->weight,
        );
      }
    }
    $links = array();
    if (taxonomy_vocabulary_access('update', $vocabulary)) {
      $links['edit'] = array(
        'title' => t('Edit'),
        'href' => 'taxonomy/term/' . $term->tid . '/edit',
        'query' => backdrop_get_destination(),
      );
    }
    if (taxonomy_vocabulary_access('delete', $vocabulary)) {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href' => 'taxonomy/term/' . $term->tid . '/delete',
        'query' => backdrop_get_destination(),
      );
    }
    $form[$key]['operations'] = array(
      '#type' => 'operations',
      '#links' => $links,
    );
  }

  $form['#total_entries'] = $total_entries;
  $form['#page_increment'] = $page_increment;
  $form['#page_entries'] = $page_entries;
  $form['#back_step'] = $back_step;
  $form['#forward_step'] = $forward_step;
  $form['#empty_text'] = t('No terms available.');
  if (taxonomy_vocabulary_access('create', $vocabulary)) {
    $form['#empty_text'] = t('No terms available. <a href="@link">Add term</a>.', array('@link' => url('admin/structure/taxonomy/' . $vocabulary->machine_name . '/add')));
  }

  if ($vocabulary->hierarchy != TAXONOMY_HIERARCHY_MULTIPLE && count($tree) > 1 && taxonomy_vocabulary_access('edit', $vocabulary)) {
    $form['actions'] = array('#type' => 'actions', '#tree' => FALSE);
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save')
    );
    $form['actions']['reset_alphabetical'] = array(
      '#type' => 'submit',
      '#value' => t('Reset to alphabetical')
    );
    $form_state['redirect'] = array($_GET['q'], (isset($_GET['page']) ? array('query' => array('page' => $_GET['page'])) : array()));
  }

  return $form;
}

/**
 * Submit handler for term_overview_form().
 *
 * Filters the displayed form to a particular language.
 */
function term_overview_form_language_filter_submit($form, &$form_state) {
  $query['langcode_term'] = $form_state['values']['langcode_term'];
  if (isset($_GET['destination'])) {
    $query['destination'] = $_GET['destination'];
  }
  backdrop_goto($_GET['q'], array('query' => $query));
}

/**
 * Submit handler for terms overview form.
 *
 * Rather than using a textfield or weight field, this form depends entirely
 * upon the order of form elements on the page to determine new weights.
 *
 * Because there might be hundreds or thousands of taxonomy terms that need to
 * be ordered, terms are weighted from 0 to the number of terms in the
 * vocabulary, rather than the standard -10 to 10 scale. Numbers are sorted
 * lowest to highest, but are not necessarily sequential. Numbers may be skipped
 * when a term has children so that reordering is minimal when a child is
 * added or removed from a term.
 *
 * @see taxonomy_overview_terms()
 */
function taxonomy_overview_terms_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Reset to alphabetical')) {
    // Execute the reset action.
    if ($form_state['values']['reset_alphabetical'] === TRUE) {
      return taxonomy_vocabulary_confirm_reset_alphabetical_submit($form, $form_state);
    }
    // Rebuild the form to confirm the reset action.
    $form_state['rebuild'] = TRUE;
    $form_state['confirm_reset_alphabetical'] = TRUE;
    return;
  }

  // Sort term order based on weight of values submitted.
  form_state_values_clean($form_state);
  backdrop_sort($form_state['values'], array('weight' => SORT_NUMERIC));

  $vocabulary = $form['#vocabulary'];
  // Update the current hierarchy type as we go.
  $hierarchy = TAXONOMY_HIERARCHY_DISABLED;

  $changed_terms = array();
  $tree = taxonomy_get_tree($vocabulary->machine_name);

  if (empty($tree)) {
    return;
  }

  // Build a list of all terms that need to be updated on previous pages.
  $weight = 0;
  $term = (array) $tree[0];
  while ($term['tid'] != $form['#first_tid']) {
    if ($term['parents'][0] == 0 && $term['weight'] != $weight) {
      $term['parent'] = $term['parents'][0];
      $term['weight'] = $weight;
      $changed_terms[$term['tid']] = $term;
    }
    $weight++;
    $hierarchy = $term['parents'][0] != 0 ? TAXONOMY_HIERARCHY_SINGLE : $hierarchy;
    $term = (array) $tree[$weight];
  }

  // Renumber the current page weights and assign any new parents.
  $level_weights = array();
  foreach ($form_state['values'] as $tid => $values) {
    if (isset($form[$tid]['#term'])) {
      $term = $form[$tid]['#term'];
      // Give terms at the root level a weight in sequence with terms on previous pages.
      if ($values['parent'] == 0 && $term['weight'] != $weight) {
        $term['weight'] = $weight;
        $changed_terms[$term['tid']] = $term;
      }
      // Terms not at the root level can safely start from 0 because they're all on this page.
      elseif ($values['parent'] > 0) {
        $level_weights[$values['parent']] = isset($level_weights[$values['parent']]) ? $level_weights[$values['parent']] + 1 : 0;
        if ($level_weights[$values['parent']] != $term['weight']) {
          $term['weight'] = $level_weights[$values['parent']];
          $changed_terms[$term['tid']] = $term;
        }
      }
      // Update any changed parents.
      if ($values['parent'] != $term['parent']) {
        $term['parent'] = $values['parent'];
        $changed_terms[$term['tid']] = $term;
      }
      $hierarchy = $term['parent'] != 0 ? TAXONOMY_HIERARCHY_SINGLE : $hierarchy;
      $weight++;
    }
  }

  // Build a list of all terms that need to be updated on following pages.
  for ($weight; $weight < count($tree); $weight++) {
    $term = (array) $tree[$weight];
    if ($term['parents'][0] == 0 && $term['weight'] != $weight) {
      $term['parent'] = $term['parents'][0];
      $term['weight'] = $weight;
      $changed_terms[$term['tid']] = $term;
    }
    $hierarchy = $term['parents'][0] != 0 ? TAXONOMY_HIERARCHY_SINGLE : $hierarchy;
  }

  // Save all updated terms.
  foreach ($changed_terms as $changed) {
    $term = (object) $changed;
    // Update term_hierarchy and term_data directly since we don't have a
    // fully populated term object to save.
    db_update('taxonomy_term_hierarchy')
      ->fields(array('parent' => $term->parent))
      ->condition('tid', $term->tid, '=')
      ->execute();

    db_update('taxonomy_term_data')
      ->fields(array('weight' => $term->weight))
      ->condition('tid', $term->tid, '=')
      ->execute();
  }

  // Update the vocabulary hierarchy to flat or single hierarchy.
  if ($vocabulary->hierarchy != $hierarchy) {
    $vocabulary->hierarchy = $hierarchy;
    taxonomy_vocabulary_save($vocabulary);
  }
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form function for the term edit form.
 *
 * @param TaxonomyTerm|null $term
 *   (optional) The taxonomy term entity to edit. If NULL or omitted, the form
 *   creates a new term.
 * @param TaxonomyVocabulary|null $vocabulary
 *   (optional) A taxonomy vocabulary entity to create the term in. Required if
 *   the term is omitted.
 *
 * @ingroup forms
 * @see taxonomy_form_term_validate()
 * @see taxonomy_form_term_submit()
 */
function taxonomy_form_term($form, &$form_state, TaxonomyTerm $term = NULL, TaxonomyVocabulary $vocabulary = NULL) {
  // Load the vocabulary from the term, if not already provided.
  if (!isset($vocabulary) && isset($term->vocabulary)) {
    $vocabulary = taxonomy_vocabulary_load($term->vocabulary);
  }

  // During initial form build, add the term entity to the form state for use
  // during form building and processing. During a rebuild, use what is in the
  // form state.
  if (!isset($form_state['term'])) {
    // Create a new TaxonomyTerm entity for the add form.
    if (!isset($term)) {
      $term = entity_create('taxonomy_term', array(
        'vocabulary' => $vocabulary->machine_name,
      ));
      backdrop_set_title(t('@vocab_name: Add term', array('@vocab_name' => $vocabulary->name)), PASS_THROUGH);
    }
    else {
      backdrop_set_title(t('@vocab_name: Edit term %term_name', array('@vocab_name' => $vocabulary->name, '%term_name' => $term->name)), PASS_THROUGH);
    }
    $form_state['term'] = $term;
  }
  else {
    $term = $form_state['term'];
  }

  $parent = array_keys(taxonomy_term_load_parents($term->tid));
  $form['#term'] = (array) $term;
  $form['#term']['parent'] = $parent;
  $form['#vocabulary'] = $vocabulary;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $term->name,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
    '#attributes' => array(
      'autofocus' => TRUE,
    ),
  );
  if (module_exists('language') && $vocabulary->language == TAXONOMY_LANGUAGE_ENABLED) {
    $form['langcode'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => isset($_GET['langcode_term']) ? $_GET['langcode_term'] : $term->langcode,
      '#options' => language_list(TRUE, TRUE),
      '#empty_value' => LANGUAGE_NONE,
      '#empty_option' => t('- All (always shown) -'),
      '#description' => t('Set a language for this term. The term will only be visible in that language.')
    );
  }
  else {
    // Maintain langcode settings if Language module is not present.
    $form['langcode'] = array(
      '#type' => 'value',
      '#value' => $term->langcode,
    );
  }

  $form['description'] = array(
    '#type' => 'text_format',
    '#title' => t('Description'),
    '#default_value' => $term->description,
    '#format' => $term->format,
    '#weight' => 0,
  );

  $form['vocabulary'] = array(
    '#type' => 'value',
    '#value' => isset($term->vocabulary) ? $term->vocabulary : $vocabulary->machine_name,
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'taxonomy') . '/js/taxonomy.term.js'),
    ),
    '#weight' => 99,
  );

  $langcode = entity_language('taxonomy_term', $term);
  field_attach_form('taxonomy_term', $term, $form, $form_state, $langcode);

  $form['relations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Relations'),
    '#collapsible' => TRUE,
    '#collapsed' => ($vocabulary->hierarchy != TAXONOMY_HIERARCHY_MULTIPLE),
    '#weight' => -10,
    '#group' => 'additional_settings',
  );

  $parent = array_keys(taxonomy_term_load_parents($term->tid));
  $children = taxonomy_get_tree($vocabulary->machine_name, $term->tid);

  // A term can't be the child of itself, nor of its children.
  foreach ($children as $child) {
    $exclude[] = $child->tid;
  }
  $exclude[] = $term->tid;

  $tree = taxonomy_get_tree($vocabulary->machine_name);
  $options = array('<' . t('root') . '>');
  if (empty($parent)) {
    $parent = array(0);
  }
  foreach ($tree as $item) {
    if (!in_array($item->tid, $exclude)) {
      $options[$item->tid] = str_repeat('· ', $item->depth) . $item->name;
    }
  }
  $form['relations']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent terms'),
    '#options' => $options,
    '#default_value' => $parent,
    '#multiple' => TRUE,
  );

  $form['relations']['weight'] = array(
    '#type' => 'number',
    '#title' => t('Weight'),
    '#default_value' => $term->weight,
    '#description' => t('Terms are displayed in ascending order by weight.'),
    '#required' => TRUE,
  );

  // Prepare cancel link.
  if (isset($_GET['destination'])) {
    $path = $_GET['destination'];
  }
  elseif (isset($_SERVER['HTTP_REFERER'])) {
    $path = $_SERVER['HTTP_REFERER'];
  }
  elseif (user_access('administer taxonomy')) {
    $path = 'admin/structure/taxonomy/' . $vocabulary->machine_name;
  }
  elseif (isset($term->tid)) {
    $path = 'taxonomy/term/' . $term->tid;
  }
  else {
    $path = '<front>';
  }

  $cancel_options = backdrop_parse_url($path);
  $cancel_options['attributes']['class'][] = 'form-cancel';

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
  );

  if ($term->tid) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#access' => taxonomy_term_access('delete', $term),
      '#weight' => 10,
      '#submit' => array('taxonomy_form_term_delete_submit'),
    );
  }
  else {
    $form_state['redirect'] = $_GET['q'];
  }
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $cancel_options['path'],
    '#options' => $cancel_options,
    '#weight' => 15,
  );

  return $form;
}

/**
 * Validation handler for the term form.
 *
 * @see taxonomy_form_term()
 */
function taxonomy_form_term_validate($form, &$form_state) {
  entity_form_field_validate('taxonomy_term', $form, $form_state);
}

/**
 * Submit handler to insert or update a term.
 *
 * @see taxonomy_form_term()
 */
function taxonomy_form_term_submit($form, &$form_state) {
  $term = taxonomy_form_term_submit_build_taxonomy_term($form, $form_state);

  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_NEW:
      backdrop_set_message(t('Created new term %term.', array('%term' => $term->name)));
      watchdog('taxonomy', 'Created new term %term.', array('%term' => $term->name), WATCHDOG_NOTICE, l(t('edit'), 'taxonomy/term/' . $term->tid . '/edit'));
      break;
    case SAVED_UPDATED:
      backdrop_set_message(t('Updated term %term.', array('%term' => $term->name)));
      watchdog('taxonomy', 'Updated term %term.', array('%term' => $term->name), WATCHDOG_NOTICE, l(t('edit'), 'taxonomy/term/' . $term->tid . '/edit'));
      break;
  }

  $current_parent_count = count($form_state['values']['parent']);
  $previous_parent_count = count($form['#term']['parent']);
  // Root doesn't count if it's the only parent.
  if ($current_parent_count == 1 && isset($form_state['values']['parent'][0])) {
    $current_parent_count = 0;
    $form_state['values']['parent'] = array();
  }

  // If the number of parents has been reduced to one or none, do a check on the
  // parents of every term in the vocabulary value.
  if ($current_parent_count < $previous_parent_count && $current_parent_count < 2) {
    taxonomy_check_vocabulary_hierarchy($form['#vocabulary'], $form_state['values']);
  }
  // If we've increased the number of parents and this is a single or flat
  // hierarchy, update the vocabulary immediately.
  elseif ($current_parent_count > $previous_parent_count && $form['#vocabulary']->hierarchy != TAXONOMY_HIERARCHY_MULTIPLE) {
    $form['#vocabulary']->hierarchy = $current_parent_count == 1 ? TAXONOMY_HIERARCHY_SINGLE : TAXONOMY_HIERARCHY_MULTIPLE;
    taxonomy_vocabulary_save($form['#vocabulary']);
  }

  $form_state['values']['tid'] = $term->tid;
  $form_state['tid'] = $term->tid;
}

/**
 * Updates the form state's term entity by processing this submission's values.
 */
function taxonomy_form_term_submit_build_taxonomy_term($form, &$form_state) {
  $term = $form_state['term'];
  entity_form_submit_build_entity('taxonomy_term', $term, $form, $form_state);

  // Prevent leading and trailing spaces in term names.
  $term->name = trim($term->name);

  // Convert text_format field into values expected by taxonomy_term_save().
  $description = $form_state['values']['description'];
  $term->description = $description['value'];
  $term->format = $description['format'];
  return $term;
}

/**
 * Form submission handler for the 'Delete' button for taxonomy_form_term().
 *
 * @see taxonomy_form_term_validate()
 * @see taxonomy_form_term_submit()
 */
function taxonomy_form_term_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = backdrop_get_destination();
    unset($_GET['destination']);
  }
  $term = $form['#term'];
  $form_state['redirect'] = array('taxonomy/term/' . $term['tid'] . '/delete', array('query' => $destination));
}

/**
 * Form builder for the term delete form.
 *
 * @ingroup forms
 * @see taxonomy_term_confirm_delete_submit()
 */
function taxonomy_term_confirm_delete($form, &$form_state, $term) {
  // Always provide entity id in the same form key as in the entity edit form.
  $form['tid'] = array('#type' => 'value', '#value' => $term->tid);

  $form['#term'] = $term;
  $form['#vocabulary'] = taxonomy_vocabulary_load($term->vocabulary);
  $form['type'] = array('#type' => 'value', '#value' => 'term');
  $form['name'] = array('#type' => 'value', '#value' => $term->name);
  $form['delete'] = array('#type' => 'value', '#value' => TRUE);
  return confirm_form($form,
    t('Are you sure you want to delete the term %title?',
    array('%title' => $term->name)),
    'admin/structure/taxonomy',
    t('Deleting a term will delete all its children if there are any. This action cannot be undone.'),
    t('Delete'),
    t('Cancel'));
}

/**
 * Submit handler to delete a term after confirmation.
 *
 * @see taxonomy_term_confirm_delete()
 */
function taxonomy_term_confirm_delete_submit($form, &$form_state) {
  taxonomy_term_delete($form_state['values']['tid']);
  taxonomy_check_vocabulary_hierarchy($form['#vocabulary'], $form_state['values']);
  backdrop_set_message(t('Deleted term %name.', array('%name' => $form_state['values']['name'])));
  watchdog('taxonomy', 'Deleted term %name.', array('%name' => $form_state['values']['name']), WATCHDOG_NOTICE);
  if (!isset($_GET['destination'])) {
    $form_state['redirect'] = 'admin/structure/taxonomy';
  }
}

/**
 * Form builder for the vocabulary delete confirmation form.
 *
 * @ingroup forms
 * @see taxonomy_vocabulary_confirm_delete_submit()
 */
function taxonomy_vocabulary_confirm_delete($form, &$form_state, $vocabulary) {
  $form['#vocabulary'] = $vocabulary;
  $form['type'] = array('#type' => 'value', '#value' => 'vocabulary');
  $form['name'] = array('#type' => 'value', '#value' => $vocabulary->name);
  $form['#submit'] = array('taxonomy_vocabulary_confirm_delete_submit');
  return confirm_form($form,
    t('Are you sure you want to delete the vocabulary %title?',
    array('%title' => $vocabulary->name)),
    'admin/structure/taxonomy',
    t('Deleting a vocabulary will delete all the terms in it. This action cannot be undone.'),
    t('Delete'),
    t('Cancel'));
}

/**
 * Submit handler to delete a vocabulary after confirmation.
 *
 * @see taxonomy_vocabulary_confirm_delete()
 */
function taxonomy_vocabulary_confirm_delete_submit($form, &$form_state) {
  $vocabulary = $form['#vocabulary'];
  $vocabulary->delete();
  backdrop_set_message(t('Deleted vocabulary %name.', array('%name' => $form_state['values']['name'])));
  watchdog('taxonomy', 'Deleted vocabulary %name.', array('%name' => $form_state['values']['name']), WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/structure/taxonomy';
}

/**
 * Form builder to confirm resetting a vocabulary to alphabetical order.
 *
 * @ingroup forms
 * @see taxonomy_vocabulary_confirm_reset_alphabetical_submit()
 */
function taxonomy_vocabulary_confirm_reset_alphabetical($form, &$form_state, $vocabulary_name) {
  $vocabulary = taxonomy_vocabulary_load($vocabulary_name);

  $form['type'] = array('#type' => 'value', '#value' => 'vocabulary');
  $form['machine_name'] = array('#type' => 'value', '#value' => $vocabulary->machine_name);
  $form['name'] = array('#type' => 'value', '#value' => $vocabulary->name);
  $form['reset_alphabetical'] = array('#type' => 'value', '#value' => TRUE);
  return confirm_form($form,
                  t('Are you sure you want to reset the vocabulary %title to alphabetical order?',
                  array('%title' => $vocabulary->name)),
                  'admin/structure/taxonomy/' . $vocabulary->machine_name,
                  t('Resetting a vocabulary will discard all custom ordering and sort items alphabetically.'),
                  t('Reset to alphabetical'),
                  t('Cancel'));
}

/**
 * Submit handler to reset a vocabulary to alphabetical order after confirmation.
 *
 * @see taxonomy_vocabulary_confirm_reset_alphabetical()
 */
function taxonomy_vocabulary_confirm_reset_alphabetical_submit($form, &$form_state) {
  db_update('taxonomy_term_data')
    ->fields(array('weight' => 0))
    ->condition('vocabulary', $form_state['values']['machine_name'])
    ->execute();
  backdrop_set_message(t('Reset vocabulary %name to alphabetical order.', array('%name' => $form_state['values']['name'])));
  watchdog('taxonomy', 'Reset vocabulary %name to alphabetical order.', array('%name' => $form_state['values']['name']), WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/structure/taxonomy/' . $form_state['values']['machine_name'];
}
