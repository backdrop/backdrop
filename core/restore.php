<?php
/**
 * @file
 * Administrative page for restoring backup database and configuration files.
 *
 * Point your browser to "http://www.example.com/core/restore.php" and follow
 * the instructions.
 *
 * If you are not logged in using either the site maintenance account or an
 * account with the "Restore system backups" permission, you will need to
 * modify the access check statement inside your settings.php file. After
 * finishing the upgrade, be sure to open settings.php again, and change it
 * back to its original state!
 */

/**
 * Defines the root directory of the Backdrop installation.
 */
define('BACKDROP_ROOT', dirname(dirname($_SERVER['SCRIPT_FILENAME'])));

// Change the directory to the Backdrop root.
chdir(BACKDROP_ROOT);

/**
 * Global flag indicating that restore.php is being run.
 *
 * When this flag is set, various operations do not take place, such as invoking
 * hook_init() and hook_exit(), css/js preprocessing, and translation.
 */
const MAINTENANCE_MODE = 'restore';

/**
 * Form constructor for the list of backups that can be restored.
 */
function restore_script_selection_form($form, &$form_state) {
  $existing_backups = array();

  // If there are no backups, display a message.
  if (empty($existing_backups)) {
    $help = st('No backups are currently available to be restored.');
    if (!settings_get('backups_directory')) {
      $help .= ' ' . st('Future backups may be created by specifying the $settings[\'backups_directory\'] variable in settings.php.');
    }

    $form['help'] = array(
      '#type' => 'help',
      '#markup' => $help,
    );

    return $form;
  }

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => 'Select a backup to restore. This will restore the database and configuration to a previous state.',
    '#weight' => -5,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Apply pending updates'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#href' => '<front>',
    '#title' => st('Cancel'),
  );

  return $form;
}

/**
 * Provides links to the homepage and administration pages.
 */
function restore_helpful_links() {
  $links['front'] = array(
    'title' => st('Home page'),
    'href' => '<front>',
  );
  if (module_exists('dashboard') && user_access('access dashboard')) {
    $links['dashboard'] = array(
      'title' => st('Dashboard'),
      'href' => 'admin/dashboard',
    );
  }
  elseif (user_access('access administration pages')) {
    $links['admin-pages'] = array(
      'title' => st('Administration pages'),
      'href' => 'admin',
    );
  }
  if (user_access('administer site configuration')) {
    $links['status-report'] = array(
      'title' => st('Status report'),
      'href' => 'admin/reports/status',
    );
  }
  return $links;
}

/**
 * Displays results of the restore script with any accompanying errors.
 */
function restore_results_page() {
  backdrop_set_title('Restore site backup');

  restore_task_list();

  if ($_SESSION['restore_success']) {
    $output = '<p>The backup was successfully restored. Proceed happily back to your <a href="' . base_path() . '">site</a></p>';
  }
  else {
    $output = '<p>The restore process failed. Check the online documentation or reach out to the Backdrop community for help.</p>';
  }

  if (settings_get('restore_free_access')) {
    backdrop_set_message("Reminder: Don't forget to set the <code>\$settings['restore_free_access']</code> value in your <code>settings.php</code> file back to <code>FALSE</code>.", 'warning');
  }

  $output .= theme('links', array('links' => restore_helpful_links()));

  return $output;
}

/**
 * Provides an overview of the backup restoration process.
 *
 * This page provides cautionary statements before proceeding with a restore.
 *
 * @return string
 *   Rendered HTML form.
 */
function restore_info_page() {
  global $databases;

  // Change query-strings on css/js files to enforce reload for all users.
  _backdrop_flush_css_js();
  // Flush the cache of all data for the update status module.
  if (db_table_exists('cache_update')) {
    cache('update')->flush();
  }

  // Flush the theme cache so we can render this page correctly if the theme
  // registry been updated with new preprocess or template variables.
  backdrop_theme_rebuild();

  restore_task_list('info');
  backdrop_set_title('Restore site backup');
  $token = backdrop_get_token('restore');
  $output = '<p>Use this utility to restore your site\'s database and configuration to a previous version.</p>';
  $output .= '<p>For more detailed information, see the Backdrop CMS <a href="https://docs.backdropcms.org/documentation/restoring-backups">documentation on restoring backups</a>.</p>';

  $form_action = check_url(backdrop_current_script_url(array('op' => 'select', 'token' => $token)));
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
 * Provides a form to create an on-demand backup before updating.
 *
 * @return string
 *   Rendered HTML form.
 */
function restore_select_page() {
  restore_task_list('select');
  backdrop_set_title('Select to backup restore');

  $elements = backdrop_get_form('restore_backup_form');
  return backdrop_render($elements);
}

/**
 * Form constructor for the list of available database module updates.
 */
function restore_backup_form($form, &$form_state) {
  // This function uses st() because it needs to function even when there is
  // no database available.
  $help = st('The restore process may take several minutes, depending on the size of your database.');
  $form['help'] = array(
    '#type' => 'help',
    '#markup' => $help,
    '#weight' => -5,
  );

  $form['backups'] = array(
    '#tree' => TRUE,
  );

  $form['backup'] = array(
    '#type' => 'radios',
    '#title' => st('Select backup'),
    '#options' => array(),
  );

  $backups = backup_directory_list();
  foreach ($backups as $backup_directory => $backup_info) {
    $form['backup']['#options'][$backup_directory] = $backup_info['label'];
    if (!$backup_info['valid']) {
      $form['backup'][$backup_directory]['#disabled'] = TRUE;
      $form['backup'][$backup_directory]['#description'] = st('This backup is missing a backup information file and cannot be restored.');
    }
    else {
      $form['backup'][$backup_directory]['#description'] = st('Contains backup files: @list', array(
        '@list' => implode(', ', array_keys($backup_info['backups'])),
      ));
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Restore backup'),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#href' => base_path() . 'core/restore.php',
    '#title' => st('Cancel'),
  );

  return $form;
}

/**
 * Renders a 403 access denied page for restore.php.
 *
 * @return string
 *   Rendered HTML warning with 403 status.
 */
function restore_access_denied_page() {
  backdrop_add_http_header('Status', '403 Forbidden');
  watchdog('access denied', 'restore.php', NULL, WATCHDOG_WARNING);
  backdrop_set_title(st('Access denied'));

  $output = '';
  $steps = array();

  $output .= st('You are not authorized to access this page. Log in using either an account with the <em>restore system backups</em> permission, or the site maintenance account (the account you created during installation). If you cannot log in, you will have to edit <code>settings.php</code> to bypass this access check. To do this:');
  $output = '<p>' . $output . '</p>';

  $steps[] = st('Find the <code>settings.php</code> file on your system, and open it with a text editor.');
  $steps[] = st('There is a line inside your <code>settings.php</code> file that says <code>$settings[\'restore_free_access\'] = FALSE</code>. Change it to <code>$settings[\'restore_free_access\'] = TRUE</code>.');
  $steps[] = st('Reload this page. The site restore script should be able to run now.');
  $steps[] = st('As soon as restoring a backup is complete, you must change the <code>restore_free_access</code> setting in the <code>settings.php</code> file back to <code>FALSE</code>: <code>$settings[\'restore_free_access\'] = FALSE;</code>.');

  $output .= theme('item_list', array('items' => $steps, 'type' => 'ol'));

  return $output;
}

/**
 * Determines if the current user is allowed to access restore.php.
 *
 * @return boolean
 *   TRUE if the current user should be granted access, or FALSE otherwise.
 */
function restore_access_allowed() {
  global $user;

  // Allow the global variable in settings.php to override the access check.
  if (settings_get('restore_free_access')) {
    return TRUE;
  }
  // Calls to user_access() might not be available if the site is not in a
  // working state (or the database is completely empty). The user #1 fallback
  // may not work either, in which case "restore_free_access" is the only
  // available way to grant access.
  try {
    require_once BACKDROP_ROOT . '/' . backdrop_get_path('module', 'user') . '/user.module';
    return user_access('restore system backups');
  }
  catch (Exception $e) {
    return ($user->uid == 1);
  }
}

/**
 * Adds the restore task list to the current page.
 */
function restore_task_list($set_active = NULL) {
  static $active;
  if ($set_active) {
    $active = $set_active;
  }

  // Default list of tasks.
  $tasks = array(
    'info' => 'Overview',
    'select' => 'Select backup',
    'restore' => 'Restore',
    'results' => 'Review',
  );

  // Only show the task list on the left sidebar if the logged-in user has
  // permission to restore backups, or if the "restore_free_access" setting in
  // settings.php has been set to TRUE.
  if (settings_get('restore_free_access') || user_access('restore system backups')) {
    return theme('task_list', array('items' => $tasks, 'active' => $active));
  }
}

// Some unavoidable errors happen because the database is not yet up-to-date.
// Our custom error handler is not yet installed, so we just suppress them.
//ini_set('display_errors', FALSE);

// Determine if the current user has access to run restore.php.
include_once BACKDROP_ROOT . '/core/includes/install.inc';
include_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(BACKDROP_BOOTSTRAP_SESSION);
backdrop_maintenance_theme();

// Only allow the requirements check to proceed if the current user has access
// to run restoration (since it may expose sensitive information about the
// site's configuration).
$op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
if (empty($op) && restore_access_allowed()) {

  // Load module basics.
  include_once BACKDROP_ROOT . '/core/includes/module.inc';
  $module_list['system']['filename'] = 'core/modules/system/system.module';
  module_list(TRUE, FALSE, FALSE, $module_list);
  backdrop_load('module', 'system');

  // Set up $language, since the installer components require it.
  backdrop_language_initialize();

  // Redirect to the restore information page if all requirements were met.
  install_goto('core/restore.php?op=info');
}

backdrop_bootstrap(BACKDROP_BOOTSTRAP_LANGUAGE);
include_once BACKDROP_ROOT . '/core/includes/unicode.inc';

// Now proceed with a full bootstrap.
backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);

// Turn error reporting back on. From now on, only fatal errors (which are
// not passed through the error handler) will cause a message to be printed.
ini_set('display_errors', TRUE);

// Only proceed if the user is allowed to restore backups.
if (restore_access_allowed()) {
  include_once BACKDROP_ROOT . '/core/includes/backup.inc';
  include_once BACKDROP_ROOT . '/core/includes/batch.inc';

  $op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
  $valid_token = isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'restore');
  switch ($op) {
    // Main restore.php operations.
    case 'info':
      $output = restore_info_page();
      break;

    case 'select':
      if ($valid_token) {
        $output = restore_select_page();
        break;
      }

    case st('Restore backup'):
      if ($valid_token) {
        // Generate absolute URLs for the batch processing (using $base_root),
        // since the batch API will pass them to url() which does not handle
        // update.php correctly by default.
        $batch_url = $base_root . backdrop_current_script_url();
        $redirect_url = $base_root . backdrop_current_script_url(array('op' => 'results'));

        // Check that a backup directory is specified.
        $backup_directory = $_POST['backup'];
        $backups = backup_directory_list();
        $errors = array();
        $ready = FALSE;
        if (!isset($backups[$backup_directory])) {
          $errors[] = st('Backup directory does not exist.');
        }
        else {
          $ready = backup_restore_batch_prepare($backup_directory, $backups[$backup_directory], $errors);
        }
        if ($ready) {
          $backups[$backup_directory]['backup_directory'] = $backup_directory;
          backup_restore_batch($backups[$backup_directory], $redirect_url, $batch_url);
          break;
        }
        else {
          foreach ($errors as $error) {
            backdrop_set_message($error, 'error');
          }
        }
      }

    case 'results':
      $output = restore_results_page();
      break;

    // Regular batch ops: defer to batch processing API.
    default:
      restore_task_list('run');
      $output = _batch_page();
      break;
  }
}
else {
  $output = restore_access_denied_page();
}
if (isset($output) && $output) {
  // Explicitly start a session so that the restore.php token will be accepted.
  backdrop_session_start();
  // We defer the display of messages until all updates are done.
  $progress_page = ($batch = batch_get()) && isset($batch['running']);
  $task_list = restore_task_list();
  print theme('restore_page', array('content' => $output, 'sidebar' => $task_list, 'show_messages' => !$progress_page));
}
