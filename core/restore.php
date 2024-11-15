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
 * Displays results of the restore script with any accompanying errors.
 */
function restore_results_page() {

  restore_task_list();

  if ($_GET['success']) {
    backdrop_set_title('Restore success');
    $output = '<p>' . st('The backup was successfully restored.') . '</p>';

    if (settings_get('restore_free_access')) {
      // Note this does not use backdrop_set_message() because session handling
      // is not initialized on restore.php.
      $output .= '<p>' . st("Reminder: Don't forget to set the !variable value in your !file file back to !value.", array(
        '!variable' => "<code>\$settings['restore_free_access']</code>",
        '!file' => '<code>settings.php</code>',
        '!value' => '<code>FALSE</code>',
      )) . '</p>';
    }

    $output .= '<p>' . l(st('Return to your site'), '<front>') . '</p>';
  }
  else {
    backdrop_set_title('Restore failure');
    $output = '<p>' . st('The restore process failed. Check the online documentation or reach out to the Backdrop community for help.') . '</p>';
  }

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
  restore_task_list('info');
  backdrop_set_title('Restore site backup');
  $token = settings_get('restore_free_access') ? '1' : backdrop_get_token('restore');
  $output = '<p>' . st('Use this utility to restore your site\'s database and configuration to a previous version.') . '</p>';
  $output .= '<p>' . st('For more detailed information, see the Backdrop CMS <a href="!url">documentation on restoring backups</a>.', array(
    '!url' => 'https://docs.backdropcms.org/documentation/restoring-backups',
  )) . '</p>';

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
  backdrop_sort($backups, array('label' => SORT_STRING), SORT_DESC);
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

  // Set the default to the most recent backup.
  $first_backup = key($form['backup']['#options']);
  $form['backup']['#default_value'] = $first_backup;
  $form['backup']['#options'][$first_backup] .= ' ' . st('(most recent)');

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Restore backup'),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#href' => '<front>',
    '#title' => st('Cancel'),
  );

  return $form;
}

/**
 * Show a progress message while processing a restore.
 *
 * Note this page has an HTTP "Refresh" header set, so it's immediately
 * attempting to load the restore page while being shown.
 */
function restore_progress_page($one_time_token) {
  restore_task_list('restore');
  backdrop_set_title('Restoring backup');

  // Redirect to restore process page. This uses a one-time token that is
  // only valid for a few seconds.

  $redirect_path = backdrop_current_script_url(array(
    'op' => 'restore',
    'token' => $one_time_token,
    'time' => REQUEST_TIME,
    'backup' => $_POST['backup'],
  ));
  backdrop_add_http_header('Refresh', '0; ' . $redirect_path);

  $output = '<p>';
  $output .= st('This process may take a while. Please wait...');
  $output .= '<span class="restore-progress">&nbsp;</span>';
  $output .= '</p>';
  return $output;
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

/**
 * Light-weight version of backdrop_goto() that guarantees no database usage.
 */
function restore_goto($url) {
  header('Location: ' . $url, TRUE, 302);
  exit();
}

// Determine if the current user has access to run restore.php.
require_once BACKDROP_ROOT . '/core/includes/install.inc';
require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
backdrop_bootstrap(BACKDROP_BOOTSTRAP_DATABASE);
backdrop_maintenance_theme();

// Only proceed if the user is allowed to restore backups.
if (restore_access_allowed()) {
  require_once BACKDROP_ROOT . '/core/includes/unicode.inc';
  require_once BACKDROP_ROOT . '/core/includes/form.inc';
  require_once BACKDROP_ROOT . '/core/includes/ajax.inc';
  require_once BACKDROP_ROOT . '/core/includes/backup.inc';
  foreach (backup_get_handlers() as $class_name => $include_path) {
    require_once BACKDROP_ROOT . '/' . $include_path;
  }

  $op = isset($_REQUEST['op']) ? $_REQUEST['op'] : 'info';

  // If free access is set, skip using any kind of token (which is session and
  // thus database dependent). This allows restore.php to be used with an
  // entirely empty or corrupted database.
  if (settings_get('restore_free_access')) {
    $valid_token = TRUE;
    $one_time_token = 1;
    $valid_one_time_token = TRUE;
  }
  else {
    $valid_token = isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'restore');
    $one_time_token = backdrop_get_token('restore_' . REQUEST_TIME);
    $valid_one_time_token = isset($_GET['time']) && isset($_GET['token']) && backdrop_valid_token($_GET['token'], 'restore_' . $_GET['time']);
  }
  switch ($op) {
    // Main restore.php operations.
    case 'select':
      if ($valid_token) {
        $output = restore_select_page();
      }
      break;

    case st('Restore backup'):
      if ($valid_token) {
        $output = restore_progress_page($one_time_token);
      }
      else {
        restore_goto($_SERVER['SCRIPT_NAME']);
      }
      break;

    case 'restore':
      if (!$valid_one_time_token || $_GET['time'] < REQUEST_TIME - 10) {
        restore_goto($_SERVER['SCRIPT_NAME']);
      }
      // Check that a backup directory is specified.
      $backup_directory = $_GET['backup'];
      $backups = backup_directory_list();
      $errors = array();
      $ready = FALSE;
      if (!isset($backups[$backup_directory])) {
        $errors[] = st('Backup directory does not exist.');
      }
      else {
        $backup_info = $backups[$backup_directory];

        foreach ($backup_info['backups'] as $backup_name => $backup) {
          backup_restore_prepare($backup_name, $backup['target'], $backup['settings'], $errors);
        }
        if (empty($errors)) {
          foreach ($backup_info['backups'] as $backup) {
            $settings = isset($backup['settings']) ? $backup['settings'] : array();
            // @todo: Better determine which class should be used.
            $class = NULL;
            if (strpos($backup['target'], 'db:') === 0) {
              $class = 'BackupMySql';
            }
            if (strpos($backup['target'], 'config:') === 0) {
              $class = 'BackupConfig';
            }
            backup_restore_execute($class, $backup['name'], $backup_directory, $backup['target'], $settings);
          }
        }
      }

      foreach ($errors as $error) {
        backdrop_set_message($error, 'error');
      }

      // Redirect to results page.
      $success = empty($errors) ? '1' : '0';
      header('Location: ' . $_SERVER['SCRIPT_NAME'] . '?op=results&success=' . $success, TRUE, 302);
      exit();

    case 'results':
      $output = restore_results_page();
      break;

    default:
      $output = restore_info_page();
      break;
  }
}
else {
  $output = restore_access_denied_page();
}

if (isset($output) && $output) {
  // Explicitly start a session so that the restore.php token will be accepted.
  if (backdrop_bootstrap() >= BACKDROP_BOOTSTRAP_SESSION) {
    backdrop_session_start();
  }
  $task_list = restore_task_list();
  print theme('restore_page', array('content' => $output, 'sidebar' => $task_list, 'show_messages' => TRUE));
}
