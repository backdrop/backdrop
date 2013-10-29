<?php

/**
 * @file
 * PHP page for handling incoming XML-RPC requests from clients.
 */

/**
 * Root directory of Drupal installation.
 */
define('DRUPAL_ROOT', getcwd());

include_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$output = '';
$output .= '<?php' . "\n";
$output .= '// $Id$' . "\n\n";

$modules = array(
  'blog',
  'book',
  'comment',
  'node',
  'user',
  'taxonomy',
  'poll',
);

$setup = var_export($setup, 1);

$classes = db_query("SELECT name FROM {registry} WHERE module IN(:modules) AND type = :type AND filename LIKE :name", array(':modules' => $modules, ':type' => 'class', ':name' => '%.test'))->fetchCol();

// Exclude tests that fail due to core bugs.
$exclude = array(
  // @see http://drupal.org/node/1008198
  'CommentActionsTestCase',
  // Removing 'CommentInterfaceTest' for now since the test is not caching
  // friendly.
  // @todo: figure out why it fails, and likely file core patch to fix it, then
  // re-enable that test.
  'CommentInterfaceTest',
);
foreach ($exclude as $class) {
  $key = array_search($class, $classes);
  unset($classes[$key]);
}

foreach ($classes as $class) {
  $output .= "/**\n";
  $output .= " * Copy of $class.\n";
  $output .= " */\n";
  $output .= "class EntityCache$class extends $class {\n";
  $output .= "  public static function getInfo() {\n";
  $output .= "    return array(\n";
  $output .= "      'name' => 'Copy of $class',\n";
  $output .= "      'description' => 'Copy of $class',\n";
  $output .= "      'group' => 'Entity cache',\n";
  $output .= "    );\n";
  $output .= "  }\n";
  $output .= "  function setUp() {\n";
  $output .= "    parent::setup();\n";
  $output .= "    module_enable(array('entitycache'));\n";
  $output .= "  }\n";
  $output .= "}\n";
  $output .= "\n";
}

file_put_contents('/tmp/entitycache.test', $output);
