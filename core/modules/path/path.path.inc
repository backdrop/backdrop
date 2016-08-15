<?php
/**
 * @file
 * Path integration for core modules.
 *
 * Path is an optional module in Backdrop, providing essentially a UI for the
 * core path functionality. Because this module may be disabled, it's up to this
 * module to provide implementations for other core modules.
 */

/**
 * Implements hook_path_info().
 */
function node_path_info() {
  $info['node'] = array(
    'entity type' => 'node',
    'label' => t('Content'),
    'pattern description' => t('Default URL alias pattern (applies to all content types with blank URL alias patterns below)'),
    'pattern default' => 'content/[node:title]',
    'batch update callback' => 'node_path_bulk_update_batch_process',
    'batch file path' => backdrop_get_path('module', 'path'),
    'batch file' => 'path.path.inc',
    'source prefix' => 'node/',
  );

  $languages = array();
  if (module_exists('locale')) {
    $languages = array(LANGUAGE_NONE => (object) array(
      'name' => t('Language neutral'),
      'langcode' => LANGUAGE_NONE,
    )) + language_list();
  }

  foreach (node_type_get_types() as $node_type_name => $node_type) {
    if (count($languages) && !empty($node_type->settings['language'])) {
      $info['node']['pattern items'][$node_type_name] = t('Default URL alias pattern for @node_type (applies to all @node_type content types with blank URL alias patterns below)', array('@node_type' => $node_type->name));
      foreach ($languages as $langcode => $language) {
        $info['node']['pattern items'][$node_type_name . '_' . $langcode] = t('URL alias pattern for all @language @node_type URLs', array('@node_type' => $node_type->name, '@language' => $language->name));
      }
    }
    else {
      $info['node']['pattern items'][$node_type_name] = t('URL alias pattern for all @node_type URLs', array('@node_type' => $node_type->name));
    }
  }

  return $info;
}

/**
 * Batch processing callback; Generate aliases for nodes.
 */
function node_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('node', 'n');
  $query->leftJoin('url_alias', 'ua', "CONCAT('node/', n.nid) = ua.source");
  $query->addField('n', 'nid');
  $query->isNull('ua.source');
  $query->condition('n.nid', $context['sandbox']['current'], '>');
  $query->orderBy('n.nid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'node');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $nids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();
  $nodes = node_load_multiple($nids);
  foreach ($nodes as $node) {
    path_save_automatic_entity_alias($node);
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($nids);
  $context['sandbox']['current'] = max($nids);
  $context['message'] = t('Updated alias for node @nid.', array('@nid' => end($nids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_path_info().
 */
function taxonomy_path_info() {
  $info['taxonomy_term'] = array(
    'entity type' => 'taxonomy_term',
    'label' => t('Taxonomy term'),
    'pattern description' => t('Default URL alias pattern (applies to all vocabularies with blank URL alias patterns below)'),
    'pattern default' => '[term:vocabulary]/[term:name]',
    'batch update callback' => 'taxonomy_path_bulk_update_batch_process',
    'batch file' => 'path.path.inc',
    'batch file path' => backdrop_get_path('module', 'path'),
    'source prefix' => 'taxonomy/term/',
  );
  $vocabularies = taxonomy_get_vocabularies();
  if (count($vocabularies)) {
    $info['taxonomy_term']['pattern items'] = array();
    foreach ($vocabularies as $vocabulary_name => $vocabulary) {
      $info['taxonomy_term']['pattern items'][$vocabulary_name] = t('URL alias pattern for all %vocab-name URLs', array('%vocab-name' => $vocabulary->name));
    }
  }

  return $info;
}

/**
 * Batch processing callback; Generate aliases for taxonomy terms.
 */
function taxonomy_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('taxonomy_term_data', 'td');
  $query->leftJoin('url_alias', 'ua', "CONCAT('taxonomy/term/', td.tid) = ua.source");
  $query->addField('td', 'tid');
  $query->isNull('ua.source');
  $query->condition('td.tid', $context['sandbox']['current'], '>');
  $query->orderBy('td.tid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'taxonomy_term');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $tids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();
  $terms = taxonomy_term_load_multiple($tids);
  foreach ($terms as $term) {
    path_save_automatic_entity_alias($term);
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($tids);
  $context['sandbox']['current'] = max($tids);
  $context['message'] = t('Updated alias for term @tid.', array('@tid' => end($tids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_path_info().
 */
function user_path_info() {
  $info['user'] = array(
    'entity type' => 'user',
    'label' => t('User account'),
    'pattern description' => t('URL alias pattern for user account URLs'),
    'pattern default' => 'users/[user:name]',
    'batch update callback' => 'user_path_bulk_update_batch_process',
    'batch file' => 'path.path.inc',
    'batch file path' => backdrop_get_path('module', 'path'),
    'source prefix' => 'user/',
  );

  return $info;
}

/**
 * Batch processing callback; Generate aliases for users.
 */
function user_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('users', 'u');
  $query->leftJoin('url_alias', 'ua', "CONCAT('user/', u.uid) = ua.source");
  $query->addField('u', 'uid');
  $query->isNull('ua.source');
  $query->condition('u.uid', $context['sandbox']['current'], '>');
  $query->orderBy('u.uid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'user');

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no nodes to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $uids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();
  $accounts = user_load_multiple($uids);
  foreach ($accounts as $account) {
    path_save_automatic_entity_alias($account);
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($uids);
  $context['sandbox']['current'] = max($uids);
  $context['message'] = t('Updated alias for user @uid.', array('@uid' => end($uids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
