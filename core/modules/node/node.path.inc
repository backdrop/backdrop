<?php
/**
 * @file
 * Path integration.
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
    'type delete callback' => 'node_path_type_delete_callback',
    'batch update callback' => 'node_path_bulk_update_batch_process',
    'batch file' => 'node.path.inc',
    'batch file path' => backdrop_get_path('module', 'node'),
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
      $info['node']['pattern items'][$node_type_name] = t('Default URL alias pattern for %node_type (applies to all %node_type content types with blank URL alias patterns below)', array('%node_type' => $node_type->name));
      foreach ($languages as $langcode => $language) {
        $info['node']['pattern items'][$node_type_name . '_' . $langcode] = t('URL alias pattern for all %node_type URLs in %language', array('%node_type' => $node_type->name, '%language' => $language->name));
      }
    }
    else {
      $info['node']['pattern items'][$node_type_name] = t('URL alias pattern for all %node_type URLs', array('%node_type' => $node_type->name));
    }
  }

  return $info;
}

/**
 * Callback to delete aliases for node types.
 */
function node_path_type_delete_callback($types) {
  $query = db_select('node', 'n');
  $query->addField('n', 'nid');
  $query->condition('n.type', $types, 'IN');
  $nids = $query->execute()->fetchCol();

  $sources = array();
  foreach ($nids as $nid) {
    $sources[] = 'node/' . $nid;
  }

  $query = db_delete('url_alias');
  $query->condition('source', $sources, 'IN');
  $query->execute();
}

/**
 * Batch processing callback; Generate or update aliases for nodes.
 */
function node_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('node', 'n');
  $query->addField('n', 'nid');
  $query->condition('n.nid', $context['sandbox']['current'], '>');
  $query->orderBy('n.nid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'node');
  if (!empty($context['choices'])) {
    $query->condition('n.type', $context['choices'], 'IN');
  }
  $query->leftJoin('url_alias', 'ua', "CONCAT('node/', n.nid) = ua.source");
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
  $nids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();
  $nodes = node_load_multiple($nids);

  foreach ($nodes as $node) {
    if (path_save_automatic_entity_alias($node)) {
      $context['results']['total']['node'] += 1;
    }
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($nids);
  $context['sandbox']['current'] = max($nids);
  $context['message'] = t('Updated alias for node @nid.', array('@nid' => end($nids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
