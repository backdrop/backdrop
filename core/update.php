<?php

/**
 * @file
 * Administrative page for handling updates from one Backdrop version to another.
 *
 * Point your browser to "http://www.example.com/core/update.php" and follow the
 * instructions.
 *
 * If you are not logged in using either the site maintenance account or an
 * account with the "Administer software updates" permission, you will need to
 * modify the access check statement inside your settings.php file. After
 * finishing the upgrade, be sure to open settings.php again, and change it
 * back to its original state!
 */

/**
 * Defines the root directory of the Backdrop installation.
 *
 * The dirname() function is used to get path to Backdrop root folder, which
 * avoids resolving of symlinks. This allows the code repository to be a symlink
 * and hosted outside of the web root. See issue #1297.
 */
define('BACKDROP_ROOT', dirname(dirname($_SERVER['SCRIPT_FILENAME'])));

// Change the directory to the Backdrop root.
chdir(BACKDROP_ROOT);

// Exit early if running an incompatible PHP version to avoid fatal errors.
// The minimum version is specified explicitly, as BACKDROP_MINIMUM_PHP is not
// yet available. It is defined in bootstrap.inc, but it is not possible to
// load that file yet as it would cause a fatal error on older versions of PHP.
if (version_compare(PHP_VERSION, '5.3.2') < 0) {
  print 'Your PHP installation is too old. Backdrop CMS requires at least PHP 5.3.2. See the <a href="https://backdropcms.org/guide/requirements">System Requirements</a> page for more information.';
  exit;
}

/**
 * Global flag indicating that update.php is being run.
 *
 * When this flag is set, various operations do not take place, such as invoking
 * hook_init() and hook_exit(), css/js preprocessing, and translation.
 *
 * This constant is defined using define() instead of const so that PHP
 * versions older than 5.3 can display the proper PHP requirements instead of
 * causing a fatal error.
 */
define('MAINTENANCE_MODE', 'update');

/**
 * Renders form with a list of available database updates.
 */
function update_selection_page() {
  backdrop_set_title('Backdrop database update');
  $elements = backdrop_get_form('update_script_selection_form');
  $output = backdrop_render($elements);

  update_task_list('select');

  return $output;
}

/**
 * Form constructor for the list of available database module updates.
 */
function update_script_selection_form($form, &$form_state) {
  $count = 0;
  $incompatible_count = 0;
  $form['start'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );

  // Ensure system.module's updates appear first.
  $form['start']['system'] = array();

  $updates = update_get_update_list();
  $starting_updates = array();
  $incompatible_updates_exist = FALSE;
  foreach ($updates as $module => $update) {
    if (!isset($update['start'])) {
      $form['start'][$module] = array(
        '#type' => 'item',
        '#title' => $module . ' module',
        '#markup'  => $update['warning'],
        '#prefix' => '<div class="messages warning">',
        '#suffix' => '</div>',
      );
      $incompatible_updates_exist = TRUE;
      continue;
    }
    if (!empty($update['pending'])) {
      $starting_updates[$module] = $update['start'];
      $form['start'][$module] = array(
        '#type' => 'hidden',
        '#value' => $update['start'],
      );
      $form['start'][$module . '_updates'] = array(
        '#theme' => 'item_list',
        '#items' => $update['pending'],
        '#title' => $module . ' module',
      );
    }
    if (isset($update['pending'])) {
      $count = $count + count($update['pending']);
    }
  }

  // Find and label any incompatible updates.
  foreach (update_resolve_dependencies($starting_updates) as $function => $data) {
    if (!$data['allowed']) {
      $incompatible_updates_exist = TRUE;
      $incompatible_count++;
      $module_update_key = $data['module'] . '_updates';
      if (isset($form['start'][$module_update_key]['#items'][$data['number']])) {
        $text = $data['missing_dependencies'] ? 'This update will been skipped due to the following missing dependencies: <em>' . implode(', ', $data['missing_dependencies']) . '</em>' : "This update will be skipped due to an error in the module's code.";
        $form['start'][$module_update_key]['#items'][$data['number']] .= '<div class="warning">' . $text . '</div>';
      }
      // Move the module containing this update to the top of the list.
      $form['start'] = array($module_update_key => $form['start'][$module_update_key]) + $form['start'];
    }
  }

  // Warn the user if any updates were incompatible.
  if ($incompatible_updates_exist) {
    backdrop_set_message('Some of the pending updates cannot be applied because their dependencies were not met.', 'warning');
  }

  if (empty($count)) {
    backdrop_set_message(t('No pending updates.'));
    unset($form);
    $form['links'] = array(
      '#theme' => 'links',
      '#links' => update_helpful_links(),
    );

    // No updates to run, so caches won't get flushed later.  Clear them now.
    backdrop_flush_all_caches();
  }
  else {
    $form['help'] = array(
      '#type' => 'help',
      '#markup' => 'Updates have been found that need to be applied. You may review the updates below before executing them.',
      '#weight' => -5,
    );
    if ($incompatible_count) {
      $form['start']['#title'] = format_plural(
        $count,
        '1 pending update (@number_applied to be applied, @number_incompatible skipped)',
        '@count pending updates (@number_applied to be applied, @number_incompatible skipped)',
        array('@number_applied' => $count - $incompatible_count, '@number_incompatible' => $incompatible_count)
      );
    }
    else {
      $form['start']['#title'] = format_plural($count, '1 pending update', '@count pending updates');
    }
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Apply pending updates'),
    );
    $form['actions']['cancel'] = array(
      '#type' => 'link',
      '#href' => '<front>',
      '#title' => t('Cancel'),
    );
  }
  return $form;
}

/**
 * Provides links to the homepage and administration pages.
 */
function update_helpful_links() {
  $links['front'] = array(
    'title' => t('Front page'),
    'href' => '<front>',
  );
  if (user_access('access administration pages')) {
    $links['admin-pages'] = array(
      'title' => t('Administration pages'),
      'href' => 'admin',
    );
  }
  if (user_access('administer site configuration')) {
    $links['status-report'] = array(
      'title' => t('Status report'),
      'href' => 'admin/reports/status',
    );
  }
  return $links;
}

/**
 * Displays results of the update script with any accompanying errors.
 */
function update_results_page() {
  backdrop_set_title('Backdrop database update');

  update_task_list();
  // Report end result.
  if (module_exists('dblog') && user_access('access site reports')) {
    $log_message = ' All errors have been <a href="' . base_path() . '?q=admin/reports/dblog">logged</a>.';
  }
  else {
    $log_message = ' All errors have been logged.';
  }

  if ($_SESSION['update_success']) {
    $output = '<p>Updates were attempted. If you see no failures below, you may proceed happily back to your <a href="' . base_path() . '">site</a>. Otherwise, you may need to update your database manually.' . $log_message . '</p>';
  }
  else {
    $updates_remaining = reset($_SESSION['updates_remaining']);
    list($module, $version) = array_pop($updates_remaining);
    $output = '<p class="error">The update process was aborted prematurely while running <strong>update #' . $version . ' in ' . $module . '.module</strong>.' . $log_message;
    if (module_exists('dblog')) {
      $output .= ' You may need to check the <code>watchdog</code> database table manually.';
    }
    $output .= '</p>';
  }

  if (settings_get('update_free_access')) {
    $output .= "<p><strong>Reminder: Don't forget to set the <code>\$settings[&#39;update_free_access&#39;]</code> value in your <code>settings.php</code> file back to <code>FALSE</code>.</strong></p>";
  }

  $output .= theme('links', array('links' => update_helpful_links()));

  // Output a list of queries executed.
  if (!empty($_SESSION['update_results'])) {
    $all_messages = '';
    foreach ($_SESSION['update_results'] as $module => $updates) {
      if ($module != '#abort') {
        $module_has_message = FALSE;
        $query_messages = '';
        foreach ($updates as $number => $queries) {
          $messages = array();
          foreach ($queries as $query) {
            // If there is no message for this update, don't show anything.
            if (empty($query['query'])) {
              continue;
            }

            if ($query['success']) {
              $messages[] = '<li class="success">' . $query['query'] . '</li>';
            }
            else {
              $messages[] = '<li class="failure"><strong>Failed:</strong> ' . $query['query'] . '</li>';
            }
          }

          if ($messages) {
            $module_has_message = TRUE;
            $query_messages .= '<h4>Update #' . $number . "</h4>\n";
            $query_messages .= '<ul>' . implode("\n", $messages) . "</ul>\n";
          }
        }

        // If there were any messages in the queries then prefix them with the
        // module name and add it to the global message list.
        if ($module_has_message) {
          $all_messages .= '<h3>' . $module . " module</h3>\n" . $query_messages;
        }
      }
    }
    if ($all_messages) {
      $output .= '<div class="update-results"><h2>The following updates returned messages</h2>';
      $output .= $all_messages;
      $output .= '</div>';
    }
  }
  unset($_SESSION['update_results']);
  unset($_SESSION['update_success']);

  return $output;
}

/**
 * Provides an overview of the Backdrop database update.
 *
 * This page provides cautionary suggestions that should happen before
 * proceeding with the update to ensure data integrity.
 *
 * @return
 *   Rendered HTML form.
 */
function update_info_page() {
  global $databases;

  // Change query-strings on css/js files to enforce reload for all users.
  _backdrop_flush_css_js();
  // Flush the cache of all data for the update status module.
  if (db_table_exists('cache_update')) {
    cache('update')->flush();
  }

  // Get database name
  $db_name = $databases['default']['default']['database'];

  // Get the config path
  $config_dir = config_get_config_directory('active');

  update_task_list('info');
  backdrop_set_title('Backdrop database update');
  $token = backdrop_get_token('update');
  $output = '<p>Use this utility to update your database whenever you install a new version of Backdrop CMS or one of the site\'s modules.</p>';
  $output .= '<p>For more detailed information, see the <a href="https://backdropcms.org/upgrade">Upgrading Backdrop CMS</a> page. If you are unsure of what these terms mean, contact your hosting provider.</p>';
  $output .= '<p>Before running updates, the following steps are recommended.</p>';
  $output .= "<ol>\n";
  $output .= "<li><strong>Create backups.</strong> This update utility will alter your database and config files. In case of an emergency you may need to revert to a recent backup; make sure you have one.\n";
  $output .= "<ul>\n";
  $output .= "<li><strong>Database:</strong> Create a database dump of the '" . $db_name . "' database.</li>\n";
  $output .= "<li><strong>Config files:</strong> Back up the entire directory at '" . $config_dir . "'.</li>\n";
  $output .= "</ul>\n";
  $output .= '<li>Put your site into <a href="' . base_path() . '?q=admin/config/development/maintenance">maintenance mode</a>.</li>' . "\n";
  $output .= "<li>Install your new files into the appropriate location, as described in <a href=\"https://backdropcms.org/upgrade\">the handbook</a>.</li>\n";
  $output .= "</ol>\n";
  $output .= "<p>After performing the above steps proceed using the continue button.</p>\n";
  $form_action = check_url(backdrop_current_script_url(array('op' => 'selection', 'token' => $token)));
  $output .= '<form method="post" action="' . $form_action . '">
  <div class="form-actions">
    <input type="submit" value="Continue" class="form-submit button-primary" />
    <a href="' . base_path() . '">Cancel</a>
  </div>
  </form>';
  $output .= "\n";
  return $output;
}

/**
 * Renders a 403 access denied page for update.php.
 *
 * @return
 *   Rendered HTML warning with 403 status.
 */
function update_access_denied_page() {
  backdrop_add_http_header('Status', '403 Forbidden');
  watchdog('access denied', 'update.php', NULL, WATCHDOG_WARNING);
  backdrop_set_title('Access denied');
  return '<p>Access denied. You are not authorized to access this page. Log in using either an account with the <em>administer software updates</em> permission or the site maintenance account (the account you created during installation). If you cannot log in, you will have to edit <code>settings.php</code> to bypass this access check. To do this:</p>
<ol>
 <li>With a text editor find the settings.php file on your system and open it.</li>
 <li>There is a line inside your settings.php file that says <code>$settings[&#39;update_free_access&#39;] = FALSE;</code>. Change it to <code>$settings[&#39;update_free_access&#39;] = TRUE;</code>.</li>
 <li>As soon as the update.php script is done, you must change the settings.php file back to its original form with <code>$settings[&#39;update_free_access&#39;] = FALSE;</code>.</li>
 <li>To avoid having this problem in the future, remember to log in to your website using either an account with the <em>administer software updates</em> permission or the site maintenance account (the account you created during installation) before you backup your database at the beginning of the update process.</li>
</ol>';
}

/**
 * Determines if the current user is allowed to run update.php.
 *
 * @return
 *   TRUE if the current user should be granted access, or FALSE otherwise.
 */
function update_access_allowed() {
  global $user;

  // Allow the global variable in settings.php to override the access check.
  if (settings_get('update_free_access')) {
    return TRUE;
  }
  // Calls to user_access() might fail during the update process,
  // so we fall back on requiring that the user be logged in as user #1.
  try {
    require_once BACKDROP_ROOT . '/' . backdrop_get_path('module', 'user') . '/user.module';
    return user_access('administer software updates');
  }
  catch (Exception $e) {
    return ($user->uid == 1);
  }
}

/**
 * Adds the update task list to the current page.
 */
function update_task_list($set_active = NULL) {
  static $active;
  if ($set_active) {
    $active = $set_active;
  }

  // Default list of tasks.
  $tasks = array(
    'requirements' => 'Verify requirements',
    'info' => 'Overview',
    'select' => 'Review updates',
    'run' => 'Run updates',
    'finished' => 'Review log',
  );

  // Only show the task list on the left sidebar if the logged-in user is has
  // permission to perform updates, or if the update_free_access' setting in
  // settings.php has been set to TRUE.
  if (settings_get('update_free_access') || user_access('administer software updates')) {
    return theme('task_list', array('items' => $tasks, 'active' => $active));
  }
}

/**
 * Returns and stores extra requirements that apply during the update process.
 */
function update_extra_requirements($requirements = NULL) {
  static $extra_requirements = array();
  if (isset($requirements)) {
    $extra_requirements += $requirements;
  }
  return $extra_requirements;
}

/**
 * Checks update requirements and reports errors and (optionally) warnings.
 *
 * @param $skip_warnings
 *   (optional) If set to TRUE, requirement warnings will be ignored, and a
 *   report will only be issued if there are requirement errors. Defaults to
 *   FALSE.
 */
function update_check_requirements($skip_warnings = FALSE) {
  // Check requirements of all loaded modules.
  $requirements = module_invoke_all('requirements', 'update');
  $requirements += update_extra_requirements();
  $severity = backdrop_requirements_severity($requirements);

  // If there are errors, always display them. If there are only warnings, skip
  // them if the caller has indicated they should be skipped.
  if ($severity == REQUIREMENT_ERROR || ($severity == REQUIREMENT_WARNING && !$skip_warnings)) {
    backdrop_set_title('Requirements problem');
    $task_list = update_task_list('requirements');
    $status_report = theme('status_report', array('requirements' => $requirements));
    $status_report .= 'Check the messages and <a href="' . check_url(backdrop_requirements_url($severity)) . '">try again</a>.';
    print theme('update_page', array('content' => $status_report, 'sidebar' => $task_list));
    exit();
  }
}

// Some unavoidable errors happen because the database is not yet up-to-date.
// Our custom error handler is not yet installed, so we just suppress them.
ini_set('display_errors', FALSE);

// We prepare a minimal bootstrap for the update requirements check to avoid
// reaching the PHP memory limit.
require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
require_once BACKDROP_ROOT . '/core/includes/update.inc';
require_once BACKDROP_ROOT . '/core/includes/common.inc';
require_once BACKDROP_ROOT . '/core/includes/file.inc';
require_once BACKDROP_ROOT . '/core/includes/unicode.inc';
update_prepare_bootstrap();

// Determine if the current user has access to run update.php.
backdrop_bootstrap(BACKDROP_BOOTSTRAP_SESSION);

// The interface language global has been renamed in Backdrop, we must ensure
// that it contains a valid value while language settings are upgraded.
$GLOBALS[LANGUAGE_TYPE_INTERFACE] = language_default();

// Only allow the requirements check to proceed if the current user has access
// to run updates (since it may expose sensitive information about the site's
// configuration).
$op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
if (empty($op) && update_access_allowed()) {
  require_once BACKDROP_ROOT . '/core/includes/install.inc';
  require_once BACKDROP_ROOT . '/core/modules/system/system.install';

  // Load module basics.
  include_once BACKDROP_ROOT . '/core/includes/module.inc';
  $module_list['system']['filename'] = 'core/modules/system/system.module';
  module_list(TRUE, FALSE, FALSE, $module_list);
  backdrop_load('module', 'system');

  // Reset the module_implements() cache so that any new hook implementations
  // in updated code are picked up.
  module_implements_reset();

  // Set up $language, since the installer components require it.
  backdrop_language_initialize();

  // Set up theme system for the maintenance page.
  backdrop_maintenance_theme();

  // Check the update requirements for Backdrop. Only report on errors at this
  // stage, since the real requirements check happens further down.
  update_check_requirements(TRUE);

  // Redirect to the update information page if all requirements were met.
  install_goto('core/update.php?op=info');
}

// update_fix_requirements() needs to run before bootstrapping beyond path.
// So bootstrap to BACKDROP_BOOTSTRAP_LANGUAGE then include unicode.inc.

backdrop_bootstrap(BACKDROP_BOOTSTRAP_LANGUAGE);
include_once BACKDROP_ROOT . '/core/includes/unicode.inc';

update_fix_requirements();

// Now proceed with a full bootstrap.

backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);
backdrop_maintenance_theme();

// Turn error reporting back on. From now on, only fatal errors (which are
// not passed through the error handler) will cause a message to be printed.
ini_set('display_errors', TRUE);

// Only proceed with updates if the user is allowed to run them.
if (update_access_allowed()) {

  include_once BACKDROP_ROOT . '/core/includes/install.inc';
  include_once BACKDROP_ROOT . '/core/includes/batch.inc';
  backdrop_load_updates();

  update_fix_compatibility();

  // Check the update requirements for all modules. If there are warnings, but
  // no errors, skip reporting them if the user has provided a URL parameter
  // acknowledging the warnings and indicating a desire to continue anyway. See
  // backdrop_requirements_url().
  $skip_warnings = !empty($_GET['continue']);
  update_check_requirements($skip_warnings);

  $op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
  switch ($op) {
    // update.php ops.

    case 'selection':
      if (isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'update')) {
        $output = update_selection_page();
        break;
      }

    case 'Apply pending updates':
      if (isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'update')) {
        // Generate absolute URLs for the batch processing (using $base_root),
        // since the batch API will pass them to url() which does not handle
        // update.php correctly by default.
        $batch_url = $base_root . backdrop_current_script_url();
        $redirect_url = $base_root . backdrop_current_script_url(array('op' => 'results'));
        update_batch($_POST['start'], $redirect_url, $batch_url);
        break;
      }

    case 'info':
      $output = update_info_page();
      break;

    case 'results':
      $output = update_results_page();
      break;

    // Regular batch ops : defer to batch processing API.
    default:
      update_task_list('run');
      $output = _batch_page();
      break;
  }
}
else {
  $output = update_access_denied_page();
}
if (isset($output) && $output) {
  // Explicitly start a session so that the update.php token will be accepted.
  backdrop_session_start();
  // We defer the display of messages until all updates are done.
  $progress_page = ($batch = batch_get()) && isset($batch['running']);
  $task_list = update_task_list();
  print theme('update_page', array('content' => $output, 'sidebar' => $task_list, 'show_messages' => !$progress_page));
}
