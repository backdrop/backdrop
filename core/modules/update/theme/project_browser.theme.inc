<?php
/**
 * @file
 * Project Browser theme pages.
 */

/**
 * Add some variables for the projects install theme.
 *
 * @param $variables
 *   An associative array containing:
 *     - current_task : the current task
 */
function update_preprocess_project_browser_install(&$variables) {
  module_load_include('inc', 'update', 'update.browser');
  // Add the themed list.
  $variables['task_list'] = project_browser_installation_task_list($variables['current_task']);
}

/**
 * Add some variables for the projects install queue theme.
 */
function update_preprocess_project_browser_install_queue(&$variables) {
  $build = array();
  if (empty($variables['projects'])) {
    $build['empty_text'] = array(
      '#markup' => t('Install queue is empty.'),
    );
  }
  else {
    foreach ($variables['projects'] as $project) {
      $build['queued-item-' . $project['name']] = array(
        '#prefix' => "<div class='project-browser-install-queue-item'>",
        '#markup' => project_browser_add_remove_queue_link($project['name'], $project['title'], 'remove-queue-link'),
        '#suffix' => "</div>",
      );
    }
    $build['install-link'] = backdrop_get_form('project_browser_install_button_form');
  }

  // Add the install button.
  $variables['queue_html'] = backdrop_render($build);
}

/**
 * Add some variables for the project browser block theme.
 *
 * @param $variables
 */
function update_preprocess_project_browser_block(&$variables) {
  // Add the title and content variables.
  $variables['title'] = $variables['element']['#title'];
  $variables['content'] = $variables['element']['#content'];
}

/**
 * Add some variables for the projects list theme.
 *
 * @param $variables
 *   An associative array containing:
 *     - projects_list : array of all projects.
 */
function update_preprocess_project_browser_list(&$variables) {
  module_load_include('inc', 'update', 'update.browser');
  backdrop_add_css(backdrop_get_path('module', 'update') . '/css/project_browser.css');

  if (is_array($variables['projects_list']) AND !empty($variables['projects_list'])) {
    $content = '';
    $first = TRUE;
    $rows = array();
    switch ($variables['type']) {
      case 'module':
        $title = t('Modules');
        break;
      case 'theme':
        $title = t('Themes');
        break;
      case 'layout':
        $title = t('Layouts');
        break;
      default:
        $title = t('Projects');
        break;
    }
    $headers = array(
      $title,
    );
    // Theme each individual project and add to the list.
    foreach ($variables['projects_list'] as $project) {
      $row = array();
      $row[] = theme('project_browser_project', array('project' => $project, 'first' => NULL));
      $rows[] = $row;
    }
    $content .= theme('table', array('header' => $headers, 'rows' => $rows));
  }
  else {
    $content = t('No results found.');
  }

  $variables['main_content'] = render($content);

  // Add the pager.
  $variables['pager'] = theme('pager', array('tags' => NULL));

  // Add the filters.
  $filter_content = backdrop_get_form('project_browser_filters_form', $variables['type']);
  $filters['project_browser_filters_block'] = array(
    '#theme' => 'project_browser_block',
    '#title' => t('Filter search'),
    '#content' => backdrop_render($filter_content),
  );
  $variables['filters'] = render($filters);

  // Add the install list.
  $install_list['project_browser_filters_block'] = array(
    '#theme' => 'project_browser_block',
    '#title' => t('Install queue'),
    '#content' => project_browser_get_install_list(),
  );
  $variables['install_list'] = render($install_list);

  // Add the advanced options block.
  $advanced_content = backdrop_get_form('project_browser_advanced_block');
  $variables['advanced'] = backdrop_render($advanced_content);
}

/**
 * Add some variables for the project details dialog.
 *
 * @param $variables
 *   An associative array containing:
 *     - project : associative array of project variables.
 */
function update_preprocess_project_browser_project_dialog(&$variables) {
  update_preprocess_project_browser_project($variables);
  $project = $variables['project'];
  $variables['project_page'] = l(t('View on BackdropCMS.org'), $project['project url']);
  $variables['description'] = _filter_htmlcorrector(filter_xss($project['description'], array('h2', 'p', 'a', 'ul', 'li')));
}

/**
 * Add some variables for the project theme.
 *
 * @param $variables
 *   An associative array containing:
 *     - project : associative array of project variables.
 */
function update_preprocess_project_browser_project(&$variables) {
  module_load_include('inc', 'update', 'update.browser');
  $project = $variables['project'];

  $variables['title'] = l(
    $project['title'], 
    'admin/modules/project-browser/project/' . $project['name'],
    array(
      'attributes' => array(
        'class' => array('use-ajax'),
        'id' => 'title-ajax',
        'data-dialog' => 'true',
        'data-dialog-options' => json_encode(array('width' => 700, 'dialogClass' => 'project-dialog')),
      ), 
    )
  ); 
  $variables['description'] = theme('project_browser_project_description', array('project' => $project));
  $variables['last_updated'] = ($project['last updated']) ? t('Last Updated: @date', array('@date' => format_date($project['last updated'], 'long'))) : '';

  $extras = array();

  if ($project['maintenance status']) {
    $extras[] = check_plain($project['maintenance status']);
  }
  if ($project['development status']) {
    // We are not showing this because it isn't a good indicator right now.
    // $extras[] = check_plain($project['development status']);
  }
  if ($project['usage'] AND is_numeric($project['usage'])) {
    $extras[] = format_plural($project['usage'], '1 Install', '@count Installs');
  }
  if ($project['rating']) {
    $extras[] = check_plain($project['rating']);
  }

  $variables['extras'] = implode(' | ', $extras);

  // Check if the project is installed.
  $found = backdrop_get_filename($project['type'], $project['name']);
  if (_project_browser_is_project_enabled($project['type'], $project['name'])) {
    $variables['status'] = '<div class="install-disabled">Already installed</div>';
    $variables['install'] = '';
  }
  // Todo: this is to mitigate a bug in bootstrap.inc. 
  // See https://github.com/backdrop/backdrop-issues/issues/1709
  elseif ($found && $found != '/.info') {
    $variables['status'] = '<div class="install-disabled">Already downloaded</div>';
    $variables['install'] = '';
  }
  else {
    $variables['status'] = '';
    $variables['install'] = project_browser_add_remove_queue_link($project['name']);
  }
}

/**
 * Builds the truncated description on the project list.
 */
function theme_project_browser_project_description($variables) {
  backdrop_add_library('system', 'backdrop.ajax');

  $project = $variables['project'];
  $form['project_description'] = array(
    '#markup' => truncate_utf8(strip_tags(_filter_htmlcorrector(filter_xss($project['description']))), 280, TRUE, TRUE, 150),
  );
  $form['project_dialog_link'] = array(
    '#type' => 'link',
    '#href' => 'admin/modules/project-browser/project/' . $project['name'],
    '#title' => t('  details'),
    '#value' => t('details'),
    '#attributes' => array(
      'class' => array('use-ajax more-link'),
      'id' => array('title-link'),
      'data-dialog' => 'true',
      'data-dialog-options' => json_encode(array('width' => 700, 'dialogClass' => 'project-dialog')),
    ),
  );
  return backdrop_render($form);
}
