<?php
/**
 * @file
 * Various functions that are required by project_browser.
 */

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
function _installer_browser_is_project_enabled($type, $name) {
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
function installer_browser_get_listed_projects() {
  if (isset($_SESSION['installer_browser_listed_projects'])) {
    return $_SESSION['installer_browser_listed_projects'];
  }

  return array();
}

/**
 * Gets the currently queued projects from the session.
 */
function installer_browser_get_queued_projects($type = NULL) {
  $projects = array();

  if (isset($_SESSION['installer_browser_install_list'])) {
    foreach ($_SESSION['installer_browser_install_list'] as $project) {
      if (is_array($project) && !empty($project)) {
        if (isset($type) && $type != $project['type']) {
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
function installer_browser_get_release($release_name, $project) {
  $release_data = installer_browser_get_project_release_data($project);

  return isset($release_data['releases'][$release_name]) ? $release_data['releases'][$release_name] : FALSE;
}

/**
 * Gets the newly installed projects from the session.
 */
function installer_browser_get_installed_projects() {
  $projects = array();

  if (isset($_SESSION['installer_browser_installed_projects'])) {
    foreach ($_SESSION['installer_browser_installed_projects'] as $project) {
      if (is_array($project) && !empty($project)) {
        $projects[$project['name']] = $project;
      }
    }
  }

  return $projects;
}

/**
 * Adds a project to the install queue.
 */
function installer_browser_install_queue_add($project) {
  $_SESSION['installer_browser_install_list'][$project['name']] = $project;
}

/**
 * Removes a project from the install queue.
 */
function installer_browser_install_queue_remove($project_name) {
  if (isset($_SESSION['installer_browser_install_list'][$project_name])) {
    unset($_SESSION['installer_browser_install_list'][$project_name]);
  }
}

/**
 * Clears the install queue.
 */
function installer_browser_install_queue_clear() {
  $_SESSION['installer_browser_install_list'] = array();
}

/**
 * Gets the currently queued releases from the session.
 */
function installer_browser_get_queued_releases() {
  $releases = array();

  if (isset($_SESSION['installer_browser_install_releases_list'])) {
    foreach ($_SESSION['installer_browser_install_releases_list'] as $release_name => $project) {
      if (is_array($project) && !empty($project)) {
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
 *  - version: The Major Version of Backdrop that is running on the Client.
 *  - text: The text that was entered as the search query, or '' if none.
 *  - type: The type of project being searched.
 *  - page: The zero-based page number.
 *  - items_per_page: How many results are requested per page.
 *
 * For example:
 * @code
 * array(
 *   'version' => '1',
 *   'text' => 'views',
 *   'type' => 'module',
 *   'page' => 3,
 *   'items_per_page' => 12,
 * )
 * @endcode
 *
 * The installer_browser_fetch_results($filters) call returns an array with the
 * following keys:
 *  - total: The total number of results found for the filters.
 *  - projects: An array of projects returned for this page request keyed by
 *    machine name. Each project array may contain the following key-value
 *    pairs:
 *     - type: The type of project this is. Can be 'module' or 'theme'.
 *     - title: The title of the project.
 *     - name: The machine name of the project.
 *     - author: The author's name.
 *     - description: The project description.
 *     - image: Absolute url to the image, if any.
 *     - usage: Number of reported active installs.
 *     - project url: Absolute url to the project page, if any.
 *     - project status url: The absolute url of the update checker.
 *     - last updated: UNIX Timestamp of when the project was last updated.
 *     - maintenance status: The project maintenance status.
 *     - development status: The project development status.
 *     - rating: A rating on a scale of 1 to 10 of the project, if available.
 *     - dependencies: An array of the dependencies of this module.
 *
 *    The following is an example of a valid return array:
 *    @code
 *    array(
 *      'total' = 5,
 *      'projects' => array(
 *        'views' => array(
 *          'type' => 'module',
 *          'title' => 'Views',
 *          'name' => 'views',
 *          'author' => 'merlinofchaos',
 *          'description' => 'Long project description',
 *          'image' => 'http://www.example.com/image.jpg',
 *          'usage' => '542312',
 *          'project url' => 'http://www.example.org/projects/views',
 *          'project status url' => 'http://updates.example.org/release-history/views/1.x',
 *          'last updated' => '12342523',
 *          'maintenance status' => 'Actively maintained',
 *          'development status' => 'Under active development',
 *          'rating' => '9.6',
 *          'dependencies' => array(
 *            'entity',
 *          ),
 *        ),
 *        'another_project => array(
 *          'type' => 'module',
 *          ...
 *        ),
 *      ),
 *    );
 *    @endcode
 *
 * @param array $filters
 *   An associative array of queries to use to filter results.
 *
 * @return
 *   Returns an array of results.
 */
function installer_browser_fetch_results($filters) {
  $server = installer_browser_get_server();
  // Attempt to retrieve the cached version of this page.
  $cid = 'installer:results:' . md5(serialize(array_merge($filters, $server)));
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
      'installer_browser_server.fetch_results' => array($local_filters),
    ));

    // Check for errors.
    if ($error = xmlrpc_error() && $error->is_error) {
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
function installer_browser_get_server() {
  $server = config_get('installer.settings', 'installer_browser_server');
  backdrop_alter('installer_browser_server', $server);

  return $server;
}

/**
 * Uses the project status url to get the available releases for a project.
 *
 * @param $project
 *   Associative array of information about the project to fetch release data
 *   for, as returned by the remote server.
 *
 * @return
 *   An array of releases for this project.
 */
function installer_browser_get_project_release_data($project) {
  $project['project_type'] = $project['type'];
  // Build the releases cache for this project
  module_load_include('inc', 'update', 'update.fetch');
  if (_update_process_fetch_task($project)) {
    $data = _update_cache_get('available_releases::' . $project['name']);
    if (isset($data->data) && isset($data->data['releases']) && is_array($data->data['releases'])) {
      return $data->data;
    }
  }

  return FALSE;
}

/**
 * Batch API callback: Installs a single release of a project during batch.
 *
 * @param $release_name
 *   The short name of the selected release of the project being installed.
 * @param $project
 *   Associative array of information about the project to be installed.
 */
function _installer_browser_batch_install_release($release_name, $project, &$context) {
  $release = installer_browser_get_release($release_name, $project);

  module_load_include('inc', 'installer', 'installer.manager');
  $url = file_create_url($release['download_link']);
  $result = installer_manager_download_project($url, TRUE);

  if ($result['success']) {
    $context['results']['successes'][] = t('Successfully installed %project.', array('%project' => $project['title']));
    $context['message'] = t('Installed %project...', array('%project' => $project['title']));

    // Add this to the session variable and remove it from the install_queue
    // variable.
    $_SESSION['installer_browser_installed_projects'][$project['name']] = $project;
    unset($_SESSION['installer_browser_install_list'][$project['name']]);
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
 * Batch API callback: shows a message and finishes up the batch.
 */
function _installer_browser_batch_install_releases_finished($success, $results, $operations) {
  backdrop_get_messages();

  // Restore the maintenance mode to what it was at the start.
  state_set('maintenance_mode', $_SESSION['maintenance_mode']);
  unset($_SESSION['maintenance_mode']);

  unset($_SESSION['installer_browser_install_releases_list']);
  if ($success) {
    if (!empty($results)) {
      if (!empty($results['failures'])) {
        backdrop_set_message(format_plural(count($results['failures']), 'Failed to install one project.', 'Failed to install @count projects.'), 'error');
      }
    }
  }
  else {
    backdrop_set_message(t('Error installing projects.'), 'error');
    backdrop_goto('admin/modules/installer-browser/install/select_versions');
  }

  $projects = installer_browser_get_installed_projects();
  $missing = installer_browser_get_missing_dependencies($projects);
  // If there are missing dependencies, go to install dependencies.
  if (count($missing) > 0) {
    backdrop_goto('admin/modules/installer-browser/install/install_dependencies');
  }
  else {
    installer_browser_get_destination_after_install();
  }
}

/**
 * Determines the types of installed projects.
 */
function installer_browser_get_installed_types() {
  $types = array();
  $projects = installer_browser_get_installed_projects();
  foreach ($projects as $project) {
    $types[$project['type']] = $project['type'];
  }

  return $types;
}



/**
 * Determines the form destination after installed projects.
 */
function installer_browser_get_destination_after_install() {
  $types = installer_browser_get_installed_types();

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
        backdrop_goto('admin/modules/installer-browser/install/enable');
      }
      else {
        backdrop_set_message($layout_message);
        backdrop_goto('admin/modules/installer-browser/install/theme');
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
    backdrop_goto('admin/modules/installer-browser/install/theme');
  }
  // Only a theme, go to theme list.
  elseif ($theme && !$module) {
    backdrop_set_message(t('One or more themes were downloaded. You may enable them now.'));
    backdrop_goto('admin/appearance');
  }
  // Only a module, jump to the module enable stage of the wizard.
  else {
    backdrop_goto('admin/modules/installer-browser/install/enable');
  }
}

/**
 * Gets the dependencies for installed projects.
 */
function installer_browser_get_missing_dependencies($projects) {
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
 * Builds the add/remove project to install queue link.
 */
function installer_browser_add_remove_queue_link($project_name, $title = NULL, $id_prefix = 'add-to-queue-link') {
  $queued_projects = installer_browser_get_queued_projects();
  if (!$title) {
    $title = isset($queued_projects[$project_name]) ? t('Remove from Install queue') : t('Add to Install queue');
  }
  $op = isset($queued_projects[$project_name]) ? 'remove' : 'add';

  $build['ajax_link'] = array(
    '#type' => 'link',
    '#title' => $title,
    '#href' => 'admin/installer-browser/nojs/install-queue/' . $op . '/'. $project_name,
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
function installer_browser_check_zip_loaded() {
  $zip_loaded = array_key_exists('zip', archiver_get_info());
  if(!$zip_loaded) {
    backdrop_set_message(t('Sorry, it seems that the PHP .zip extension is not loaded on your server. You will not be able to download any projects using Project Browser until this is fixed. Please contact your website administrator.'), 'warning');
  }
  return $zip_loaded;
}
