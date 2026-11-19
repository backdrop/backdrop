<?php

/**
 * @file
 * Drush integration for the devel module.
 */

/**
 * Implements hook_drush_command().
 */
function devel_drush_command() {
  $items['devel-reinstall'] = array(
    'description' => dt('Disable, Uninstall, and Install a list of projects.'),
    'arguments' => array(
      'projects' => dt('A space-separated list of project names.'),
    ),
    'aliases' => array('dre'),
  );
  $items['fn-hook'] = array(
    'description' => 'List implementations of a given hook and explore source of specified one.',
    'arguments' => array(
      'hook' => 'The name of the hook to explore.'
    ),
    'aliases' => array('fnh', 'hook'),
  );
  $items['fn-view'] = array(
    'description' => 'Show the source of specified function or method.',
    'arguments' => array(
      'function' => 'The name of the function or method to view.',
    ),
    'options' => array(
      'pipe' => 'Output just the filename of the function',
      'format' => 'Specify how the filename should be printed. Available placeholders are !startline, !endline and !file',
    ),
    'examples' => array(
      'fn-view backdrop_set_breadcrumb' => 'View the source code for function "backdrop_set_breadcrumb"',
      'vi `drush --pipe fn-view user_access --format=\'+!startline !file\'`' => 'Edit the file that contains the function "user_access"',
      'fn-view NodeController::load' => 'View the source code for method load in the class NodeController'
    ),
    'aliases' => array('fnv'),
  );
  $items['devel-token'] = array(
    'description' => dt('List available tokens'),
    'aliases' => array('token'),
    'core' => array(7), // Remove once 3.0 is released.
  );
  return $items;
}

/**
 * A command callback for reinstalling one or more projects.
 *
 * This is faster than 3 separate bootstraps.
 */
function drush_devel_reinstall() {
  $projects = func_get_args();

  $args = array_merge(array('pm-disable'), $projects);
  call_user_func_array('drush_invoke', $args);

  $args = array_merge(array('pm-uninstall'), $projects);
  call_user_func_array('drush_invoke', $args);

  $args = array_merge(array('pm-enable'), $projects);
  call_user_func_array('drush_invoke', $args);
}

/**
 * A command handler for showing hook implementations.
 */
function drush_devel_fn_hook($hook) {
  // Get implementations in the .install files as well.
  include_once './includes/install.inc';
  backdrop_load_updates();

  if ($hook_implementations = module_implements($hook)) {
    if ($choice = drush_choice(array_combine($hook_implementations, $hook_implementations), 'Enter the number of the hook implementation you wish to view.')) {
      return drush_devel_fn_view($choice . "_$hook");
    }
  }
  else {
    drush_log(dt('No implementations.'), 'ok');
  }
}

/**
 * A command handler for showing source code of a function or method.
 */
function drush_devel_fn_view($function_name) {
  // Get implementations in the .install files as well.
  include_once './core/includes/install.inc';
  backdrop_load_updates();

  if (strpos($function_name, '::') === FALSE) {
    if (!function_exists($function_name)) {
      return drush_set_error(dt('Function not found'));
    }
    $reflect = new ReflectionFunction($function_name);
  }
  else {
    list($class, $method) = explode('::', $function_name);
    if (!method_exists($class, $method)) {
      return drush_set_error(dt('Method not found'));
    }
    $reflect = new ReflectionMethod($class, $method);
  }
  $func_info = array('!file' => $reflect->getFileName(), '!startline' => $reflect->getStartLine(), '!endline' => $reflect->getEndLine());
  $format = drush_get_option('format', '!file');
  drush_print_pipe(dt($format, $func_info));
  drush_print(dt("// file: !file, lines !startline-!endline", $func_info));

  _drush_devel_print_function($reflect->getFileName(), $reflect->getStartLine(), $reflect->getEndLine());
}

/**
 * A command callback for listing available tokens.
 */
function drush_devel_token() {
  $rows[] = array(dt('Group'), dt('Token'), dt('Name'));
  $all = token_info();
  foreach ($all['tokens'] as $group => $tokens) {
    foreach ($tokens as $key => $token) {
      $rows[] = array($group, $key, $token['name']);
    }
  }
  drush_print_table($rows, TRUE);
}


/**
 * Prints a function including any Doxygen-style comments preceding it.
 */
function _drush_devel_print_function($file, $start_line, $end_line) {
  $line_num = 0;
  $doxygen = NULL;
  $fp = fopen( $file, 'r' );

  while (!feof($fp) && ($line_num < ($start_line - 1))) {
    $line = fgets($fp);
    ++$line_num;

    if (substr($line,0,3) == '/**') {
      $doxygen = $line;
    }
    elseif (isset($doxygen)) {
      $doxygen .= $line;
      if ($line_num + 1 == $start_line) {
        drush_print(rtrim($doxygen));
      }
      if (strstr($line, '*/') !== FALSE) {
        $doxygen = NULL;
      }
    }
  }
  while (!feof($fp) && ($line_num < $end_line)) {
    $line = fgets($fp);
    ++$line_num;
    drush_print(rtrim($line));
  }
}
