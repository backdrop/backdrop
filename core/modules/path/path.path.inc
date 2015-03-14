<?php

/**
 * @file
 * Pathauto integration for core modules.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_path_alias_types().
 *
 * Used primarily by the bulk delete form.
 */
function path_path_alias_types() {
  $objects['user/'] = t('Users');
  $objects['node/'] = t('Content');
  if (module_exists('taxonomy')) {
    $objects['taxonomy/term/'] = t('Taxonomy terms');
  }
  return $objects;
}

/**
 * Implements hook_path().
 */
function path_pathauto($op) {
  $return = array();
  $modules = array('node', 'taxonomy', 'user');
  foreach ($modules as $module) {
    if (module_exists($module)) {
      $return[] = module_invoke($module, 'path', $op);
    }
  }

  return $return;
}

/**
 * Implements hook_path().
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['group_header'] = t('Content paths');
      $settings['pattern_description'] = t('Default path pattern (applies to all content types with blank patterns below)');
      $settings['pattern_default'] = 'content/[node:title]';
      $settings['batch_update_callback'] = 'node_path_bulk_update_batch_process';
      $settings['batch_file'] = backdrop_get_path('module', 'path') . '/path.path.inc';

      $languages = array();
      if (module_exists('locale')) {
        $languages = array(LANGUAGE_NONE => (object) array(
            'name' => t('Language neutral'),
            'langcode' => LANGUAGE_NONE,
          )) + language_list();
      }

      foreach (node_type_get_types() as $node_type_name => $node_type) {
        if (count($languages) && !empty($node_type->settings['language'])) {
          $settings['pattern_items'][$node_type_name] = t('Default path pattern for @node_type (applies to all @node_type content types with blank patterns below)', array('@node_type' => $node_type->name));
          foreach ($languages as $langcode => $language) {
            $settings['pattern_items'][$node_type_name . '_' . $langcode] = t('Pattern for all @language @node_type paths', array('@node_type' => $node_type->name, '@language' => $language->name));
          }
        }
        else {
          $settings['pattern_items'][$node_type_name] = t('Pattern for all @node_type paths', array('@node_type' => $node_type->name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
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
  $query->addTag('pathauto_bulk_update');
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

  path_node_update_alias_multiple($nids, 'bulkupdate');
  $context['sandbox']['count'] += count($nids);
  $context['sandbox']['current'] = max($nids);
  $context['message'] = t('Updated alias for node @nid.', array('@nid' => end($nids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_path().
 */
function taxonomy_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'taxonomy_term';
      $settings['token_type'] = 'term';
      $settings['group_header'] = t('Taxonomy term paths');
      $settings['pattern_description'] = t('Default path pattern (applies to all vocabularies with blank patterns below)');
      $settings['pattern_default'] = '[term:vocabulary]/[term:name]';
      $settings['batch_update_callback'] = 'taxonomy_path_bulk_update_batch_process';
      $settings['batch_file'] = backdrop_get_path('module', 'path') . '/path.path.inc';

      $vocabularies = taxonomy_get_vocabularies();
      if (count($vocabularies)) {
        $settings['pattern_items'] = array();
        foreach ($vocabularies as $vocabulary_name => $vocabulary) {
          $settings['pattern_items'][$vocabulary_name] = t('Pattern for all %vocab-name paths', array('%vocab-name' => $vocabulary->name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
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
  $query->addTag('pathauto_bulk_update');
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

  path_taxonomy_term_update_alias_multiple($tids, 'bulkupdate');
  $context['sandbox']['count'] += count($tids);
  $context['sandbox']['current'] = max($tids);
  $context['message'] = t('Updated alias for term @tid.', array('@tid' => end($tids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}

/**
 * Implements hook_path().
 */
function user_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'user';
      $settings['token_type'] = 'user';
      $settings['group_header'] = t('User paths');
      $settings['pattern_description'] = t('Pattern for user account page paths');
      $settings['pattern_default'] = 'users/[user:name]';
      $settings['batch_update_callback'] = 'user_path_bulk_update_batch_process';
      $settings['batch_file'] = backdrop_get_path('module', 'path') . '/path.path.inc';
      return (object) $settings;
    default:
      break;
  }
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
  $query->addTag('pathauto_bulk_update');
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

  path_user_update_alias_multiple($uids, 'bulkupdate');
  $context['sandbox']['count'] += count($uids);
  $context['sandbox']['current'] = max($uids);
  $context['message'] = t('Updated alias for user @uid.', array('@uid' => end($uids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
