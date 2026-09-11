<?php
/**
 * @file
 * Path integration.
 */

/**
 * Implements hook_path_info().
 */
function taxonomy_path_info() {
  $info['taxonomy_term'] = array(
    'entity type' => 'taxonomy_term',
    'label' => t('Taxonomy term'),
    'pattern label' => t('Default URL alias pattern for taxonomy terms'),
    'pattern description' => t('Fallback pattern for all taxonomy terms without a specific URL alias pattern below.'),
    'pattern default' => '[term:vocabulary]/[term:name]',
    'type delete callback' => 'taxonomy_path_type_delete_callback',
    'batch update callback' => 'taxonomy_path_bulk_update_batch_process',
    'batch file' => 'taxonomy.path.inc',
    'batch file path' => backdrop_get_path('module', 'taxonomy'),
    'source prefix' => 'taxonomy/term/',
  );

  $languages = array();
  if (module_exists('language')) {
    $languages = array(LANGUAGE_NONE => (object) array(
      'name' => t('Language neutral'),
      'langcode' => LANGUAGE_NONE,
    )) + language_list();
  }

  $vocabularies = taxonomy_get_vocabularies();
  if (count($vocabularies)) {
    $info['taxonomy_term']['pattern items'] = array();
    $info['taxonomy_term']['pattern items label'] = t('URL alias patterns for specific vocabularies');
    foreach ($vocabularies as $vocabulary_name => $vocabulary) {
      if (count($languages) && !empty($vocabulary->language)) {
        $info['taxonomy_term']['pattern items'][$vocabulary_name] = t('Default URL alias pattern for all taxonomy terms in the vocabulary %vocab-name (any language)', array('%vocab-name' => $vocabulary->name));
        foreach ($languages as $langcode => $language) {
          $info['taxonomy_term']['pattern items'][$vocabulary_name . '_' . $langcode] = t('Taxonomy terms in the vocabulary %vocab-name in %language', array('%vocab-name' => $vocabulary->name, '%language' => $language->name));
        }
      }
      else {
        $info['taxonomy_term']['pattern items'][$vocabulary_name] = t('Taxonomy terms in the vocabulary %vocab-name', array('%vocab-name' => $vocabulary->name));
      }
    }
  }

  return $info;
}

/**
 * Callback to delete aliases for taxonomy types.
 */
function taxonomy_path_type_delete_callback($types) {
  $query = db_select('taxonomy_term_data', 'td');
  $query->addField('td', 'tid');
  $query->condition('td.vocabulary', $types, 'IN');
  $tids = $query->execute()->fetchCol();

  $sources = array();
  foreach ($tids as $tid) {
    $sources[] = 'taxonomy/term/' . $tid;
  }

  $query = db_delete('url_alias');
  $query->condition('source', $sources, 'IN');
  $query->execute();
}

/**
 * Batch processing callback; Generate URL aliases for taxonomy terms.
 */
function taxonomy_path_bulk_update_batch_process(&$context) {
  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('taxonomy_term_data', 'td');
  $query->addField('td', 'tid');
  $query->condition('td.tid', $context['sandbox']['current'], '>');
  $query->orderBy('td.tid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'taxonomy_term');
  if (!empty($context['choices'])) {
    $query->condition('td.vocabulary', $context['choices'], 'IN');
  }
  $query->leftJoin('url_alias', 'ua', "CONCAT('taxonomy/term/', td.tid) = ua.source");
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
  $tids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();
  $terms = taxonomy_term_load_multiple($tids);

  foreach ($terms as $term) {
    if (path_save_automatic_entity_alias($term)) {
      $context['results']['total']['taxonomy_term'] += 1;
    }
  }
  path_verbose_resume();

  $context['sandbox']['count'] += count($tids);
  $context['sandbox']['current'] = max($tids);
  $context['message'] = t('Updated alias for term @tid.', array('@tid' => end($tids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }
}
