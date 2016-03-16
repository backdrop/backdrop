<?php
/**
 * @file
 * Page callbacks and various functions that are required by project_browser.
 */

/**
 * Page to display the projects.
 *
 * @param $type
 *   Type of project to view, ie 'module', 'theme' or 'layout'.
 *
 * @return
 *   Themed list of the projects.
 */
function project_browser_page($type) {
  // Show a warning if no zip extension available, but allow to browse and
  // select to view available releases. Install blocked on select versions page.
  $zip_loaded = project_browser_check_zip_loaded();

  // Save needed filters to session.
  if (isset($_GET['repository'])) {
    $_SESSION['project_browser_sever_filter'] = $_GET['repository'];
  }
  if (isset($_GET['order'])) {
    $_SESSION['project_browser_order_filter_' . $type] = $_GET['order'];
  }
  if (isset($_GET['sort'])) {
    $_SESSION['project_browser_sort_filter_' . $type] = $_GET['sort'];
  }

  // Build the filters.
  $backdrop_version = explode('.', BACKDROP_CORE_COMPATIBILITY);
  $filters = array(
    'version' => $backdrop_version[0],
    'type' => $type,
  );

  // Add filters.
  if (isset($_SESSION['project_browser_text_filter_' . $type])) {
    $filters['text'] = $_SESSION['project_browser_text_filter_' . $type];
  }
  if (isset($_SESSION['project_browser_order_filter_' . $type])) {
    $filters['order'] = $_SESSION['project_browser_order_filter_' . $type];
  }
  if (isset($_SESSION['project_browser_sort_filter_' . $type])) {
    $filters['sort'] = $_SESSION['project_browser_sort_filter_' . $type];
  }

  $filters['items_per_page'] = 10;
  $filters['page'] = isset($_GET['page']) ? $_GET['page'] : 0;

  // Get the projects to display here based on the filters.
  $results = project_browser_fetch_results($filters);

  // Save the listed projects in the session so it can be used.
  $_SESSION['project_browser_listed_projects'] = $results['projects'];

  $list = array();
  foreach ($results['projects'] as $project) {
    $list[] = $project;
  }

  // Add the pager.
  $total = $results['total'];
  $num_per_page = 10;
  $page = pager_default_initialize($total, $num_per_page);
  $offset = $num_per_page * $page;
  $start = ($total) ? $offset + 1 : 0;
  $finish = $offset + $num_per_page;
  if ($finish > $total) {
    $finish = $total;
  }

  $sort_options = project_browser_get_sort_options();
  $current_order = isset($_SESSION['project_browser_order_filter_' . $type]) ? $_SESSION['project_browser_order_filter_' . $type] : 'score';
  $current_sort = isset($_SESSION['project_browser_sort_filter_' . $type]) ? $_SESSION['project_browser_sort_filter_' . $type] : 'desc';;

  $build = array();
  $build['content'] = array(
    'project_browser_header' => array(
      '#markup' => t('Showing @start to @finish of @total.', array(
        '@start' => $start, '@finish' => $finish, '@total' => $total)),
      '#weight' => 0,
    ),
    'project_browser_sort_header' => array(
      '#type' => 'item',
      '#weight' => 2,
      '#markup' => project_browser_get_sort_widget($sort_options, $current_order, $current_sort),
    ),
    'project_browser_list' => array(
      '#markup' => theme('project_browser_list', array('projects_list' => $list, 'type' => $type)),
      '#weight' => 3,
    ),
    'pager' => array(
      '#theme' => 'pager',
      '#weight' => 99,
    ),
  );

  return $build;
}

/**
 * Page to display individual projects in a modal.
 *
 * @param $project
 *   The project name.
 *
 * @return
 *   Render array of a themed project.
 */
function project_browser_project_display($project) {
  $projects = $_SESSION['project_browser_listed_projects'];
  $form['project_dialog'] = array(
    '#markup' => theme('project_browser_project_dialog', array('project' => $projects[$project])),
  );
  return $form;
}

/**
 * Page to install projects.
 *
 * @param $op
 *    Operation to preform.
 */
function project_browser_installation_page($op) {
  backdrop_add_css(backdrop_get_path('module', 'update') . '/css/project_browser.css', array('preprocess' => FALSE));

  switch ($op) {
    case 'select_versions':
      backdrop_set_title(t("Select versions"));
      $content = project_browser_installation_select_versions_page();
      break;
    case 'install_dependencies':
      backdrop_set_title(t("Install Dependencies"));
      $content = project_browser_installation_install_dependencies_page();
      break;
    case 'theme':
      backdrop_set_title(t("Themes were installed"));
      $content = project_browser_installation_enable_page('theme');
      break;
    case 'enable':
      backdrop_set_title(t("Enable modules"));
      $content = project_browser_installation_enable_page();
      break;
  }
  return theme('project_browser_install', array('current_task' => $op, 'main_content' => backdrop_render($content)));
}

/**
 * Task page for the Select versions installation task.
 *
 * Shows a form where the user can select which versions to install for each
 * project.
 */
function project_browser_installation_select_versions_page() {
  // Show a form that lets the user select which version of the projects to install
  $queued_projects = project_browser_get_queued_projects();
  unset($_SESSION['project_browser_installed_projects']);

  return backdrop_get_form('project_browser_installation_select_versions_form', $queued_projects);
}

/**
 * Form builder for the select versions form.
 *
 * @param $projects
 *   An array of projects to get the releases for.
 */
function project_browser_installation_select_versions_form($form, &$form_state, $projects) {
  backdrop_add_js(backdrop_get_path('module', 'update') . '/js/select_releases.js');

  $zip_loaded = project_browser_check_zip_loaded();

  $form = array();

  // First unset any old data.
  unset($_SESSION['project_browser_install_releases_list']);

  $form['#tree'] = TRUE;

  $form['releases-header'] = array(
    '#type' => 'item',
    '#markup' => t("You're about to install:"),
  );

  $form['releases'] = array();

  foreach ($projects as $project) {
    // Get the available releases for this project.
    if (!$release_data = project_browser_get_project_release_data($project)) {
      backdrop_set_message(t('Could not fetch releases for project %project.',
        array('%project' => $project['title'])), 'warning');
      watchdog('project_browser', 'Could not fetch releases for project %project.',
        array('%project' => $project['title']), WATCHDOG_ERROR);
      project_browser_install_queue_remove($project['name']);
      continue;
    }

    // We use the update module to calculate the recommended version.
    $project_data = array(
      'existing_major' => 0,
      'existing_version' => 0,
      'install_type' => '',
    );
    module_load_include('inc', 'update', 'update.compare');
    update_calculate_project_update_status($project_data, $release_data);

    $releases_list = array();

    foreach ($release_data['releases'] as $version => $release) {
      $release_title = t("@project @version - @date", array(
        '@project' => $project['title'],
        '@version' => $release['version'],
        '@date' => format_date($release['date'], 'custom', 'M j, Y'),
        ));
      if (isset($release['terms']['Release type']) AND !empty($release['terms']['Release type'])) {
        $release_title .= " (" . implode(', ', $release['terms']['Release type']) . ")";
      }
      if (isset($release['release_link'])) {
        $releases_list[$version] = l($release_title, $release['release_link']);
      }
      else {
        $releases_list[$version] = $release_title;
      }
    }

    $form['releases'][$project['name']]['project'] = array(
      '#type' => 'value',
      '#value' => $project,
    );

    $form['releases'][$project['name']]['release_name'] = array(
      '#type' => 'radios',
      '#title' => t('Select release for @project', array('@project' => $project['title'])),
      '#options' => $releases_list,
      '#default_value' => key($releases_list),
      '#prefix' => '<div class="project-browser-releases-wrapper project-browser-release-' . $project['name'] . '">',
      '#suffix' => '</div>',
      '#attributes' => array(
        'class' => array('project-browser-releases-radios'),
      ),
      '#required' => TRUE,
    );
    $form['releases'][$project['name']]['selected_text'] = array(
      '#type' => 'item',
      '#prefix' => '<div class="project-browser-selected-release project-browser-selected-release-' . $project['name'] . '">',
      '#suffix' => '</div>',
      '#markup' => reset($releases_list),
    );
    if (isset($project_data['recommended'])) {
      // If there is a recommended release set, then only show it and show the
      // jQuery link.
      $recommended_releases = array();
      $recommended_releases[$project_data['recommended']] = $releases_list[$project_data['recommended']];
      $form['releases'][$project['name']]['release_name']['#default_value'] = $project_data['recommended'];
      $form['releases'][$project['name']]['selected_text']['#markup'] = $releases_list[$project_data['recommended']];
    }
    if (count($releases_list) > 1) {
      $form['releases'][$project['name']]['selected_text']['#markup'] .=
        " (<span class='project-browser-show-releases-link' rel='" . $project['name'] . "'>" . t('change release') . "</span>)";
    }
  }

  // If there is nothing to install, go to the enable page.
  if (empty($form['releases'])) {
    backdrop_set_message(t('No releases data found for any of the selected projects.'), 'warning');
    backdrop_goto('admin/modules/project-browser/install/enable');
  }

  $form['backup_warning'] = array(
    '#type' => 'markup',
    '#markup' => t('Back up your database and site before you continue. !link.',
      array('!link' => l(t('Learn how'), 'http://<MAINSITE_URL>/node/22281'))),
  );
  $form['maintenance_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Perform updates with site in maintenance mode (strongly recommended)'),
    '#default_value' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Install'),
    '#disabled' => !$zip_loaded,
  );
  $form['cancel'] = array(
    '#type' => 'link',
    '#name' => 'cancel',
    '#title' => t('Cancel'),
    '#href' => 'admin/modules/project-browser/reset',
    '#options' => array(),
  );
  return $form;
}

/**
 * Submit handler for the reset/cancel link.
 */
function project_browser_installation_select_versions_form_reset() {
  project_browser_install_queue_clear();
  backdrop_goto('admin/modules/project-browser');
}

/**
 * Submit handler for the select versions form.
 */
function project_browser_installation_select_versions_form_submit($form, &$form_state) {
  // Store maintenance_mode setting so we can restore it when done.
  $_SESSION['maintenance_mode'] = state_get('maintenance_mode', FALSE);
  if ($form_state['values']['maintenance_mode'] == TRUE) {
    state_set('maintenance_mode', TRUE);
  }

  foreach ($form_state['values']['releases'] as $item) {
    // Load the selected release.
    if ($release = project_browser_get_release($item['release_name'], $item['project'])) {
      // Add the release to a session variable.
      $_SESSION['project_browser_install_releases_list'][$item['release_name']] = $item['project'];
    }
  }

  // Install the projects with batch.
  module_load_include('inc', 'update', 'update.manager');

  $queued_releases = project_browser_get_queued_releases();
   $operations = array();
  foreach ($queued_releases as $release_name => $project) {
    $operations[] = array('_project_browser_batch_install_release', array($release_name, $project));
  }
  $batch = array(
    'operations' => $operations,
    'finished' => '_project_browser_batch_install_releases_finished',
    'title' => t('Installing projects'),
    'init_message' => t('Installing projects...'),
    'progress_message' => t('Installed @current out of @total.'),
    'error_message' => t('Installation has encountered an error.'),
    'file' => backdrop_get_path('module', 'update') . '/update.browser.inc',
  );
  batch_set($batch);
}

/**
 * Task page for the Install Dependencies installation task.
 *
 * Show a form which lets the user select which version of dependencies to
 * install.
 */
function project_browser_installation_install_dependencies_page() {
  $projects = project_browser_get_installed_projects();
  $missing = project_browser_get_missing_dependencies($projects);

  if (count($missing) > 0) {
    $missing_projects = array();
    // Add the project data in the array as best we can.
    foreach ($missing as $project_shortname => $dependencies) {
      foreach ($dependencies as $shortname) {
        $missing_projects[$shortname] = array(
          'name' => $shortname,
          'type' => 'module',
          'title' => $shortname,
          'includes' => array(),
        );
        // Make an assumption that the dependencies will have the same update
        // status URL as the project. This may not be true in the future if
        // we have decide to have more than one update server, but for now this
        // is necessary to test dependencies installation.
        if (isset($projects[$project_shortname]['info']['project status url'])) {
          $status_url = $projects[$project_shortname]['info']['project status url'];
        }
        elseif (isset($projects[$project_shortname]['project status url'])) {
          $status_url = $projects[$project_shortname]['project status url'];
        }
        if ($status_url) {
          $missing_projects[$shortname]['info']['project status url'] = $status_url;
          $missing_projects[$shortname]['project status url'] = $status_url;
        }
      }
    }

    return backdrop_get_form('project_browser_installation_select_versions_form', $missing_projects);
  }
  else {
    backdrop_goto('admin/modules/project-browser/install/enable');
  }
}

/**
 * Task page for the Enable projects installation task.
 *
 * Show a form which lets the user enable the newly installed projects.
 */
function project_browser_installation_enable_page($type = 'enable') {
  $installed_projects = project_browser_get_installed_projects();

  if (count($installed_projects) > 0) {
    return backdrop_get_form('project_browser_installation_' . $type . '_form', $installed_projects);
  }
  else {
    backdrop_goto('admin/modules/project-browser');
  }
}

/**
 * Form builder for theme install.
 *
 * @param $projects
 *   An array of projects to get the releases for.
 */
function project_browser_theme_installation_form($form, &$form_state, $projects) {
  $form['instructions'] = array(
  '#type' => 'item',
  '#markup' => t('The themes you selected have been successfully installed.
    You may enable installed themes here or on the main !themes page. ', array('!themes' => l(t('Themes'), 'admin/appearance')))
  );

  $theme_default = config_get('system.core', 'theme_default');
  $themes = system_rebuild_theme_data();

  foreach ($projects as $project) {
    if ($project['type'] == 'theme' && isset($themes[$project['name']])) {
      $theme = $project['name'];
      $form[$theme] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('browser-installed-theme'),
        ),
      );
      if (file_exists($themes[$theme]->info['screenshot'])) {
        $form[$theme]['image'] = array(
          '#type' => 'item',
          '#markup' => theme('image', array('uri' => $themes[$theme]->info['screenshot'])),
          '#attributes' => array(
            'class' => array('browser-installed-theme-image'),
          ),
        );
      }
      $form[$theme]['name'] = array(
        '#type' => 'item',
        '#markup' => $themes[$theme]->info['name'],
      );
      if ($theme == $theme_default) {
        $form[$theme]['is_default'] = array(
          '#type' => 'markup',
          '#markup' => t('Default theme'),
        );
      }
      else {
        $form[$theme]['set'] = array(
          '#type' => 'submit',
          '#value' => 'Set as default theme',
          '#name' => $theme . '-submit',
          '#submit' => array('project_browser_installation_theme_set'),
        );
      }
    }
  }

  if (in_array('module', project_browser_get_installed_types())) {
    $form['next_link'] = array(
      '#type' => 'link',
      '#title' => 'Next: Install downloaded modules',
      '#href' => 'admin/modules/project-browser/install/enable',
    );
  }
  else {
    $form['next_link'] = array(
      '#type' => 'submit',
      '#value' => t('Finish'),
      '#submit' => array('project_browser_installation_theme_finish'),
    );
  }
  
  return $form;
}


/**
 * Submit callback for project_browser_theme_installation_form().
 *
 * Ends the installation wizard.
 */
function project_browser_installation_theme_finish($form, &$form_state) {
  project_browser_installation_select_versions_form_reset();
}


/**
 * Sets a selected theme as the default site theme.
 */
function project_browser_installation_theme_set($form, &$form_state) {
  $theme = $form_state['triggering_element']['#array_parents'][0];
  // Get current list of themes.
  $themes = list_themes();

  // Check if the specified theme is one recognized by the system.
  if (!empty($themes[$theme])) {
    // Enable the theme if it is currently disabled.
    if (empty($themes[$theme]->status)) {
     theme_enable(array($theme));
    }
    // Set the default theme.
    config_set('system.core', 'theme_default', $theme);
    menu_rebuild();
  }
  dpm($theme);
}


/**
 * Form builder for the select versions form.
 *
 * @param $projects
 *   An array of projects to get the releases for.
 */
function project_browser_installation_enable_form($form, &$form_state, $projects) {
  $modules = system_rebuild_module_data();

  $options = array();
  $missing = array();

  foreach ($projects as $project) {
    if ($project['type'] == 'module') {
      $dependency_check = TRUE;
      $dependencies = array();
      if (isset($modules[$project['name']])) {
        foreach ($modules[$project['name']]->info['dependencies'] as $dependency) {
          if (isset($modules[$dependency])) {
            $dependencies[] = $modules[$dependency]->info['name'] . ' (' . t('Installed') . ')';
          }
          else {
            $dependency_check = FALSE;
            $dependencies[] = $dependency . ' (' . t('Missing') . ')';
          }
        }
        if ($dependency_check) {
          $options[$project['name']] = array(
            array('data' => $modules[$project['name']]->info['name']),
            array('data' => $modules[$project['name']]->info['version']),
            array('data' => implode(', ', $dependencies)),
          );
        }
        else {
          $missing[$project['name']] = array(
            array('data' => $modules[$project['name']]->info['name']),
            array('data' => $modules[$project['name']]->info['version']),
            array('data' => implode(', ', $dependencies)),
          );
        }
      }
      else {
        backdrop_set_message(t('There was an error getting information for @module',
          array('@module' => $project['name'])), 'error');
      }
    }
  }

  $headers = array(
    array('data' => t('Title')),
    array('data' => t('Version')),
    array('data' => t('Dependencies')),
  );

  if(!empty($options) || !empty($missing)) {
    $form['module_instructions'] = array(
    '#type' => 'item',
    '#markup' => t('You may enable modules using the form below
      or on the main !link page.', array('!link' => l(t('Modules'), 'admin/modules'))),
    );
    if (!empty($options)) {
      $form['modules'] = array(
        '#type' => 'tableselect',
        '#title' => t('Enable modules'),
        '#description' => t('Select which modules you would like to enable.'),
        '#header' => $headers,
        '#options' => $options,
        '#empty' => t('No new modules installed.'),
        '#multiple' => TRUE,
        '#js_select' => TRUE,
        '#weight' => 1,
      );

      $form['actions'] = array('#type' => 'actions');
      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#submit' => array('project_browser_installation_enable_form_submit'),
        '#value' => t('Enable modules'),
      );
      $form['actions']['cancel'] = array(
        '#type' => 'link',
        '#name' => 'cancel',
        '#title' => t('Cancel'),
        '#href' => 'admin/modules/project-browser/reset',
        '#options' => array(),
      );
    }

    if (!empty($missing)) {
      $form['missing'] = array(
        '#type' => 'item',
        '#title' => t('Missing Dependencies'),
        '#description' => t('These modules are missing one or more dependencies,
          and so cannot be enabled.'),
        '#markup' => theme('table', array('header' => $headers, 'rows' => $missing)),
        '#weight' => 2,
      );
    }
  }

  return $form;
}

/**
 * Form submit handler for the enable modules form.
 */
function project_browser_installation_enable_form_submit($form, &$form_state) {
  $enable_queue = array_filter($form_state['values']['modules']);
  // Enable these all at once so that dependencies are handled properly.
  module_enable($enable_queue);
  backdrop_flush_all_caches();
  backdrop_set_message(t('Installation is complete.'));
  backdrop_goto('admin/modules/project-browser');
}

/**
 * Get a task list to the sidebar area when installing projects.
 *
 * This will need to be called from every page of the install process.
 *
 * @param $active
 *   (Optional) Set the active task by key.
 */
function project_browser_installation_task_list($active = NULL) {
  // Default list of tasks.
  $tasks = array(
    'select_versions' => t('Select versions'),
    'install_dependencies' => t('Install Dependencies'),
    'theme' => t('Enable themes'),
    'enable' => t('Enable projects'),
  );

  require_once BACKDROP_ROOT . '/core/includes/theme.maintenance.inc';

  return theme_task_list(array('items' => $tasks, 'active' => $active));
}

/**
 * Returns the themed install list form.
 */
function project_browser_get_install_list() {
  $build = array(
    '#theme' => 'project_browser_install_queue',
    '#projects' => project_browser_get_queued_projects(),
  );
  return backdrop_render($build);
}

/**
 * Shows an install button for the Install Queue block.
 */
function project_browser_install_button_form($form, &$form_state) {
  $form['#attributes']['id'] = 'project-browser-install-button-form';
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Install',
  );
  $form['#action'] = url('admin/modules/project-browser/install/select_versions');

  return $form;
}

/**
 * Builds the filters form.
 */
function project_browser_filters_form($form, &$form_state, $type) {
  $form['search_text'] = array(
    '#type' => 'textfield',
    '#size' => '25',
    '#title' => t('Search String'),
    '#default_value' => isset($_SESSION['project_browser_text_filter_' . $type]) ? $_SESSION['project_browser_text_filter_' . $type] : '',
  );

  $form['project_type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );

  if (isset($_SESSION['project_browser_text_filter_' . $type])) {
    $form['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Clear'),
    );
  }

  return $form;
}

/**
 * Returns a themed sort widget for the filters.
 */
function project_browser_get_sort_widget($sort_options, $current_order, $current_sort) {
  $sort_list = array();
  $sort_list[] = array('data' => t('Sort by:'), 'class' => array('sort-header'));
  $current_path = backdrop_get_path_alias($_GET['q']);

  foreach ($sort_options as $sort_option) {
    $classes = array();
    $query = array(
      'order' => $sort_option['method'],
      'sort' => $sort_option['default_sort'],
    );

    // If the sort option is currently active, handle it differently.
    if ($current_order == $sort_option['method']) {
      $classes[] = 'sort-active';
      $classes[] = 'sort-' . $current_sort;

      // Set the direction of the sort link to the opposite of what it currently
      // is.
      if ($current_sort == $query['sort']) {
        if ($query['sort'] == 'desc') {
          $query['sort'] = 'asc';
        }
        else {
          $query['sort'] = 'desc';
        }
      }
    }
    else {
      $classes[] = 'sort-inactive';
    }

    $sort_list[] = array(
      'data' => l($sort_option['name'], $current_path, array('query' => $query, 'class' => array())),
      'class' => $classes,
    );
  }

  return theme('item_list', array(
      'items' => $sort_list,
      'type' => 'ul',
      'attributes' => array('class' => array('project-browser-sort-list', 'clearfix'))));
}


/**
 * Returns a list of sort options.
 *
 * @param $full
 *   Set this to TRUE if you want to get all of the supported sort methods.
 */
function project_browser_get_sort_options($full = FALSE) {
  $sort_options = array(
    'score' => array('method' => 'score', 'name' => t('Relevancy'), 'default_sort' => 'desc'),
    'usage' => array('method' => 'usage', 'name' => t('Most installed'), 'default_sort' => 'desc'),
    'title' => array('method' => 'title', 'name' => t('Title'), 'default_sort' => 'asc'),
    'latest_release' => array('method' => 'latest_release', 'name' => t('Latest release'), 'default_sort' => 'desc'),
  );

  if ($full) {
    $sort_options['type'] = array('method' => 'type', 'name' => t('Type'), 'default_sort' => 'asc');
    $sort_options['created'] = array('method' => 'created', 'name' => t('Date created'), 'default_sort' => 'asc');
    $sort_options['latest_activity'] = array('method' => 'latest_activity', 'name' => t('Latest build'), 'default_sort' => 'desc');
  }

  return $sort_options;
}

/**
 * Handles the filters form submit.
 */
function project_browser_filters_form_submit($form, &$form_state) {
  $type = $form_state['values']['project_type'];
  if ($form_state['clicked_button']['#id'] == 'edit-clear') {
    $_SESSION['project_browser_text_filter_' . $type] = array();
  }
  else {
    $_SESSION['project_browser_text_filter_' . $type] = $form_state['values']['search_text'];
  }
}

/**
 * Checks if a project is enabled.
 *
 * @param string $type
 *   The type of project. Could be 'theme' or 'module' or 'layout'.
 * @param string $name
 *   The short name of the project.
 *
 * @return
 *   TRUE if the project is enabled, FALSE otherwise.
 */
function _project_browser_is_project_enabled($type, $name) {
  switch ($type) {
    case 'module':
      return module_exists($name);
      break;
    case 'theme':
      $themes = list_themes();
      return isset($themes[$name]);
      break;
    case 'layout':
      $layout = layout_load($name);
      return !empty($layout);
      break;
  }
  return FALSE;
}

/**
 * Gets the currently listed projects from the session.
 */
function project_browser_get_listed_projects() {
  if (isset($_SESSION['project_browser_listed_projects'])) {
    return $_SESSION['project_browser_listed_projects'];
  }

  return array();
}

/**
 * Gets the currently queued projects from the session.
 */
function project_browser_get_queued_projects($type = NULL) {
  $projects = array();

  if (isset($_SESSION['project_browser_install_list'])) {
    foreach ($_SESSION['project_browser_install_list'] as $project) {
      if (is_array($project) AND !empty($project)) {
        if (isset($type) AND $type != $project['type']) {
          continue;
        }
        else {
          $projects[$project['name']] = $project;
        }
      }
    }
  }

  return $projects;
}

/**
 * Gets a release from a project and a release_name.
 */
function project_browser_get_release($release_name, $project) {
  $release_data = project_browser_get_project_release_data($project);

  return isset($release_data['releases'][$release_name]) ? $release_data['releases'][$release_name] : FALSE;
}

/**
 * Gets the newly installed projects from the session.
 */
function project_browser_get_installed_projects() {
  $projects = array();

  if (isset($_SESSION['project_browser_installed_projects'])) {
    foreach ($_SESSION['project_browser_installed_projects'] as $project) {
      if (is_array($project) AND !empty($project)) {
        $projects[$project['name']] = $project;
      }
    }
  }

  return $projects;
}

/**
 * Adds a project to the install queue.
 */
function project_browser_install_queue_add($project) {
  $_SESSION['project_browser_install_list'][$project['name']] = $project;
}

/**
 * Removes a project from the install queue.
 */
function project_browser_install_queue_remove($project_name) {
  if (isset($_SESSION['project_browser_install_list'][$project_name])) {
    unset($_SESSION['project_browser_install_list'][$project_name]);
  }
}

/**
 * Clears the install queue.
 */
function project_browser_install_queue_clear() {
  $_SESSION['project_browser_install_list'] = array();
}

/**
 * Gets the currently queued releases from the session.
 */
function project_browser_get_queued_releases() {
  $releases = array();

  if (isset($_SESSION['project_browser_install_releases_list'])) {
    foreach ($_SESSION['project_browser_install_releases_list'] as $release_name => $project) {
      if (is_array($project) AND !empty($project)) {
        $releases[$release_name] = $project;
      }
    }
  }

  return $releases;
}

/**
 * Fetches results from the server based on the parameters passed in.
 *
 * $filters should be an associative array with the following keys:
 * array(
 *   'version' => '1', // The Major Version of Backdrop that is running on the Client
 *   'text' => 'views', // The text that was entered as the search query, or '' if none
 *   'type' => 'module', // The type of project being searched
 *   'page' => 3, // The zero-based page number
 *   'items_per_page' => 12, // How many results are requested per page
 * )
 *
 * The project_browser_fetch_results($filters) call returns an array like this:
 *
 * array(
 *   'total' = 5, // The total number of results found for the filters
 *   'projects' => array( // An array of projects returned for this page request
 *     'views' => array( // A project array keyed by the machine name
 *       'type' => 'module', // The type of project this is. Can be 'module' or 'theme'
 *       'title' => 'Views', // The title of the project
 *       'name' => 'views', // The machine name of the project
 *       'author' => 'merlinofchaos', // The author's name
 *       'description' => "Long project description ...",
 *       'image' => 'http://www.example.com/image.jpg', // Absolute url to the image, if any
 *       'usage' => '542312', // How many Downloads the module has had
 *       'project url' => 'http://www.<MAINSITE_URL>/projects/views', // Absolute url to the project page, if any
 *       'project status url' => 'http://updates.<MAINSITE_URL>/release-history/views/1.x', // The absolute url of the update checker, formatted like how <MAINSITE_URL> Update Status does it
 *       'last updated' => '12342523', // UNIX Timestamp of when the project was last updated
 *       'maintenance status' => 'Actively maintained', // Maintenance status
 *       'development status' => 'Under active development', // Development status
 *       'rating' => '9.6', // A rating on a scale of 1 to 10 of the project, if available
 *       'dependencies' => array( // An array of the dependencies of this module
 *         'entity',
 *       ),
 *     ),
 *   'another_project => array( ... ),
 *   ),
 * );
 *
 * @param array $filters
 *   An associative array of queries to use to filter results.
 *
 * @return
 *   Returns an array of results.
 */
function project_browser_fetch_results($filters) {
  $server = project_browser_get_server();
  // Attempt to retrieve the cached version of this page.
  $cid = md5(serialize(array_merge($filters, $server)));
  if ($cache = cache_get($cid)) {
    return $cache->data;
  }

  $results = array(
    'projects' => array(),
    'total' => 0,
  );

  $local_filters = $filters;

  // Use XMLRPC if it is set.
  if ($server['method'] == 'xmlrpc') {
    $results = xmlrpc($server['url'], array(
      'project_browser_server.fetch_results' => array($local_filters),
    ));

    // Check for errors.
    if ($error = xmlrpc_error() AND $error->is_error) {
      backdrop_set_message(t("Encountered an error when trying to fetch results from @name. Error @code : @message",
        array('@name' => $server['name'], '@code' => $error->code, '@message' => $error->message)));
      return array();
    }
  }

  // Use json if it is set.
  if ($server['method'] == 'json') {
    $local_filters['method'] = 'query';

    $query_url = $server['url'] . '/query/' . $local_filters['type'] . '/1?' . http_build_query($local_filters, FALSE, '&');

    $response = backdrop_http_request($query_url);

    if ($response->code == '200') {
      $results = backdrop_json_decode($response->data);
    }
    else {
      backdrop_set_message(t("Encountered an error when trying to fetch results from @name. Error @code : @message",
        array('@name' => $server['name'], '@code' => $response->code, '@message' => $response->error)));
      return array();
    }
  }

  // Set the cached version of the results.
  cache_set($cid, $results, 'cache', strtotime("+24 hours"));

  return $results;
}

/**
 * Gets the server to use for fetching results.
 *
 * @return
 *   Returns the active server, in the format:
 *   array(
 *     "url" => "https://projects.backdropcms.org",
 *     "name" => "Backdrop",
 *     "method" => "json"
 *   )
 */
function project_browser_get_server() {
  $server = config_get('update.settings', 'project_browser_server');
  backdrop_alter('project_browser_server', $server);

  return $server;
}

/**
 * Uses the project status url to get the available releases for a project.
 *
 * @param $project
 *   The project to get the releases for.
 *
 * @return
 *   An array of releases for this project.
 */
function project_browser_get_project_release_data($project) {
  $project['project_type'] = $project['type'];
  // Build the releases cache for this project
  module_load_include('inc', 'update', 'update.fetch');
  if (_update_process_fetch_task($project)) {
    $data = _update_cache_get('available_releases::' . $project['name']);
    if (isset($data->data) AND isset($data->data['releases']) AND is_array($data->data['releases'])) {
      return $data->data;
    }
  }

  return FALSE;
}

/**
 * Installs a single release of a project during batch.
 */
function _project_browser_batch_install_release($release_name, $project, &$context) {
  $release = project_browser_get_release($release_name, $project);

  module_load_include('inc', 'update', 'update.manager');
  $url = file_create_url($release['download_link']);
  $result = update_manager_download_project($url, TRUE);

  if ($result['success']) {
    $context['results']['successes'][] = t('Successfully installed %project.', array('%project' => $project['title']));
    $context['message'] = t('Installed %project...', array('%project' => $project['title']));

    // Add this to the session variable and remove it from the install_queue
    // variable.
    $_SESSION['project_browser_installed_projects'][$project['name']] = $project;
    unset($_SESSION['project_browser_install_list'][$project['name']]);
  }
  else {
    watchdog('project_browser', 'There was an error while installing %project.
      !message',
      array('%project' => $project['title'], '!message' => $result['message']), WATCHDOG_ERROR);
    $context['results']['failures'][] = t('Error installing %project. Errors have been logged.',
      array('%project' => $project['title']));
    $context['message'] = t('Error installing %project. !message',
      array('%project' => $project['title'], '!message' => $result['message']));
  }
}

/**
 * Shows a message and finish up the batch.
 */
function _project_browser_batch_install_releases_finished($success, $results, $operations) {
  backdrop_get_messages();

  // Restore the maintenance mode to what it was at the start.
  state_set('maintenance_mode', $_SESSION['maintenance_mode']);
  unset($_SESSION['maintenance_mode']);

  unset($_SESSION['project_browser_install_releases_list']);
  if ($success) {
    if (!empty($results)) {
      if (!empty($results['failures'])) {
        backdrop_set_message(format_plural(count($results['failures']), 'Failed to install one project.', 'Failed to install @count projects.'), 'error');
      }
    }
  }
  else {
    backdrop_set_message(t('Error installing projects.'), 'error');
    backdrop_goto('admin/modules/project-browser/install/select_versions');
  }

  $projects = project_browser_get_installed_projects();
  $missing = project_browser_get_missing_dependencies($projects);
  // If there are missing dependencies, go to install dependencies.
  if (count($missing) > 0) {
    backdrop_goto('admin/modules/project-browser/install/install_dependencies');
  }
  else {
    project_browser_get_destination_after_install();
  }
}

/**
 * Determines the types of installed projects.
 */
function project_browser_get_installed_types() {
  $types = array();
  $projects = project_browser_get_installed_projects();
  foreach ($projects as $project) {
    $types[$project['type']] = $project['type'];
  }

  return $types;
}



/**
 * Determines the form destination after installed projects.
 */
function project_browser_get_destination_after_install() {
  $types = project_browser_get_installed_types();

  $layout = in_array('layout', $types);
  $theme = in_array('theme', $types);
  $module = in_array('module', $types);

  $layout_message = t('One or more layouts were installed. You may enable layouts on the !link page.', array('!link' => l(t('Layout'), 'admin/structure/layouts')));

  if ($layout) {
    // Installed a layout and other project(s), continue the wizard. There's no
    // step for layouts in the wizard, since there's nothing we could do there.
    if ($theme || $module) {
      if (!$theme) {
        backdrop_set_message($layout_message);
        backdrop_goto('admin/modules/project-browser/install/enable');
      }
      else {
        backdrop_set_message($layout_message);
        backdrop_goto('admin/modules/project-browser/install/theme');
      }
    }
    // Installed only a layout, go straight to layout list.
    else {
      backdrop_set_message($layout_message);
      backdrop_goto('admin/structure/layouts');
    }
  }
  // Theme and module, go to theme stage of wizard.
  elseif ($theme && $module) {
    backdrop_goto('admin/modules/project-browser/install/theme');
  }
  // Only a theme, go to theme list.
  elseif ($theme && !$module) {
    backdrop_set_message(t('One or more themes were downloaded. You may enable them now.'));
    backdrop_goto('admin/appearance');
  }
  // Only a module, jump to the module enable stage of the wizard.
  else {
    backdrop_goto('admin/modules/project-browser/install/enable');
  }
}

/**
 * Gets the dependencies for installed projects.
 */
function project_browser_get_missing_dependencies($projects) {
  $modules = system_rebuild_module_data();

  $missing = array();

  foreach ($projects as $project) {
    if ($project['type'] == 'module') {
      $dependencies = array();
      if (isset($modules[$project['name']])) {
        foreach ($modules[$project['name']]->info['dependencies'] as $dependency) {
          if (!isset($modules[$dependency])) {
            $dependencies[] = $dependency;
          }
        }
        if (count($dependencies) > 0) {
          $missing[$project['name']] = $dependencies;
        }
      }
      else {
        backdrop_set_message(t('There was an error getting information for @module',
          array('@module' => $project['name'])), 'error');
      }
    }
  }

  return $missing;
}

/**
 * Page callback: Allows for adding to and removing from the install queue.
 */
function project_browser_install_queue_callback($method, $op, $project_name) {
  switch ($op) {
    case 'add':
      $projects = project_browser_get_listed_projects();

      if (isset($projects[$project_name])) {
        $project = $projects[$project_name];
        project_browser_install_queue_add($project);
      }
      else {
        backdrop_set_message(t('Error: The project was not found.'), 'error');
      }
      break;

    case 'remove':
      project_browser_install_queue_remove($project_name);
      break;
  }

  switch ($method) {
    case 'nojs':
      // Redirect to the page it came from.
      $redirect = (isset($_GET['destination'])) ? $_GET['destination'] : 'admin/modules/project-browser';

      backdrop_goto($redirect);
      break;

    case 'ajax':
      $commands = array();
      // Refresh the install queue.
      $commands[] = ajax_command_replace('#project-browser-install-queue', project_browser_get_install_list());
      // Refresh the add to queue link.
      $commands[] = ajax_command_replace('#add-to-queue-link-' . $project_name, project_browser_add_remove_queue_link($project_name));
      return array('#type' => 'ajax', '#commands' => $commands);
      break;
  }
}

/**
 * Builds the add/remove project to install queue link.
 */
function project_browser_add_remove_queue_link($project_name, $title = NULL, $id_prefix = 'add-to-queue-link') {
  $queued_projects = project_browser_get_queued_projects();
  if (!$title) {
    $title = isset($queued_projects[$project_name]) ? t('Remove from Install queue') : t('Add to Install queue');
  }
  $op = isset($queued_projects[$project_name]) ? 'remove' : 'add';

  $build['ajax_link'] = array(
    '#type' => 'link',
    '#title' => $title,
    '#href' => 'admin/project-browser/nojs/install-queue/' . $op . '/'. $project_name,
    '#options' => array(
      'query' => backdrop_get_destination(),
    ),
    '#id' => $id_prefix . '-' . $project_name,
    '#ajax' => array(
      'effect' => 'fade',
      'speed' => 1000,
      'progress' => array(
        'type' => 'throbber',
      ),
    ),
  );

  return backdrop_render($build);
}

/**
 *
 * Checks if PHP zip extension loaded on webserver.
 */
function project_browser_check_zip_loaded() {
  $zip_loaded = array_key_exists('zip', archiver_get_info());
  if(!$zip_loaded) {
    backdrop_set_message(t('Sorry, it seems that the PHP .zip extension is not loaded on your server. You will not be able
    to download any projects using Project Browser until this is fixed. Please contact your website administrator.'), 'warning');
  }
  return $zip_loaded;
}
