<?php
/**
 * @file
 * Path integration.
 */

/**
 * Implements hook_path_info().
 */
function user_path_info() {
  $info['user'] = array(
    'entity type' => 'user',
    'label' => t('User account'),
    'pattern label' => t('URL alias pattern for user accounts'),
    'pattern default' => 'users/[user:name]',
    'batch update callback' => 'user_path_bulk_update_batch_process',
    'batch file' => 'user.path.inc',
    'batch file path' => backdrop_get_path('module', 'user'),
    'source prefix' => 'user/',
  );

  return $info;
}

/**
 * Batch processing callback; Generate URL aliases for users.
 */
function user_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('users', 'u');
  $query->addField('u', 'uid');
  $query->condition('u.uid', $context['sandbox']['current'], '>');
  $query->orderBy('u.uid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'user');
  $query->leftJoin('url_alias', 'ua', "CONCAT('user/', u.uid) = ua.source");
  if ($context['op'] == 'update') {
    $query->isNotNull('ua.source');
  }
  if ($context['op'] == 'generate') {
    $query->isNull('ua.source');
  }

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
    if (path_save_automatic_entity_alias($account)) {
      $context['results']['total']['user'] += 1;
    }
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($uids);
  $context['sandbox']['current'] = max($uids);
  $context['message'] = t('Updated alias for user @uid.', array('@uid' => end($uids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
