<?php
/**
 * @file
 * Database additions for Layout upgrade tests. Used in layout.test.
 *
 * This dump only contains data and schema components relevant for Layout module
 * functionality. The drupal-7.filled.database.php file is imported before
 * this dump, so the two form the database structure expected in tests
 * altogether.
 */

// Add various core blocks to test that their positioning is upgraded.
// Note that the search block is already positioned in the sidebar_first.
db_update('block')
  ->fields([
    'region' => 'triptych_middle',
    'status' => '1',
  ])
  ->condition('module', 'node')
  ->condition('delta', 'syndicate')
  ->condition('theme', 'bartik')
  ->execute();

db_update('block')
  ->fields([
    'region' => 'triptych_last',
    'status' => '1',
    'visibility' => '1',
    'pages' => "<front>\nnode*",
  ])
  ->condition('module', 'comment')
  ->condition('delta', 'recent')
  ->condition('theme', 'bartik')
  ->execute();

db_update('block')
  ->fields([
    'region' => 'footer_firstcolumn',
    'status' => '1',
  ])
  ->condition('module', 'system')
  ->condition('delta', 'main-menu')
  ->condition('theme', 'bartik')
  ->execute();

db_update('block')
  ->fields([
    'region' => 'footer_secondcolumn',
    'status' => '1',
  ])
  ->condition('module', 'system')
  ->condition('delta', 'management')
  ->condition('theme', 'bartik')
  ->execute();

db_update('block')
  ->fields([
    'region' => 'footer_thirdcolumn',
    'status' => '1',
  ])
  ->condition('module', 'system')
  ->condition('delta', 'user-menu')
  ->condition('theme', 'bartik')
  ->execute();

// The navigation block will not exist in Backdrop 1.x, add it to test the
// handling of a non-existent block (using the BlockBroken class).
db_update('block')
  ->fields([
    'region' => 'footer_fourthcolumn',
    'status' => '1',
  ])
  ->condition('module', 'system')
  ->condition('delta', 'navigation')
  ->condition('theme', 'bartik')
  ->execute();

// Node-specific blocks.
db_update('block')
  ->fields([
    'region' => 'sidebar_second',
    'status' => '1',
  ])
  ->condition('module', 'node')
  ->condition('delta', 'recent')
  ->condition('theme', 'bartik')
  ->execute();

db_insert('block_node_type')
  ->fields([
    'module',
    'delta',
    'type',
  ])
  ->values([
    'module' => 'node',
    'delta' => 'recent',
    'type' => 'post',
  ])
  ->execute();

// Role-specific blocks.
db_insert('block_role')
  ->fields([
    'module',
    'delta',
    'rid',
  ])
  ->values([
    'module' => 'system',
    'delta' => 'user-menu',
    'rid' => 2,
  ])
  ->execute();
