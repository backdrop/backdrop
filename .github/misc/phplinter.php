#!/usr/bin/env php
<?php
/**
 * @file
 * Runs the php linter against changed files.
 *
 */

/**
 * Get a list of changed files via git diff and run the linter function.
 */
function lint_changed_files() {
  $descriptorspec = array(
    1 => array('pipe', 'w'),
  );
  $php_extensions = array(
    'php',
    'inc',
    'module',
    'install',
    'test',
    'profile',
  );
  // Note that "origin" is just a naming convention.
  $git_command = 'git diff --name-only --diff-filter=d origin/1.x';
  $git_process = proc_open($git_command, $descriptorspec, $pipes);

  if (is_resource($git_process)) {
    while (!feof($pipes[1])) {
      $file = fgets($pipes[1], 1024);
      $extension = trim(pathinfo($file, PATHINFO_EXTENSION));
      if (!in_array($extension, $php_extensions)) {
        continue;
      }
      phplinter($file);
    }
    fclose($pipes[1]);
    proc_close($git_process);
  }
  else {
    exit(1);
  }
}

/**
 * Run the php lint command and evaluate output.
 *
 * Translates the output to a format, GitHub Actions can use for annotations.
 *
 * @param string $file
 *   Path to php file that changed compared to 1.x branch.
 */
function phplinter($file) {
  $descriptorspec = array(
    1 => array('pipe', 'w'),
    2 => array('pipe', 'w'),
  );
  $lint_command = "php -l $file";
  $lint_process = proc_open($lint_command, $descriptorspec, $lint_pipes);
  if (is_resource($lint_process)) {
    while (!feof($lint_pipes[2])) {
      $line = fgets($lint_pipes[2], 1024);
      if (preg_match('#^(.+) in ([a-z./_-]+) on line (\d+)#', $line, $matches)) {
        print "::error file=$matches[2],line=$matches[3],col=0::$matches[1]\n";
      }
    }
    fclose($lint_pipes[2]);
    proc_close($lint_process);
  }
}

lint_changed_files();
