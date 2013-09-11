<?php

/**
 * @file
 * Hooks provided by the Taxonomy module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Act on taxonomy vocabularies when loaded.
 *
 * Modules implementing this hook can act on the vocabulary objects before they
 * are returned by taxonomy_vocabulary_load_multiple().
 *
 * @param $vocabulary
 *   An array of taxonomy vocabulary objects.
 */
function hook_taxonomy_vocabulary_load($vocabularies) {
  foreach ($vocabularies as $vocabulary) {
    $vocabulary->synonyms = variable_get('taxonomy_' . $vocabulary->vid . '_synonyms', FALSE);
  }
}


/**
 * Act on taxonomy vocabularies before they are saved.
 *
 * Modules implementing this hook can act on the vocabulary object before it is
 * inserted or updated.
 *
 * @param $vocabulary
 *   A taxonomy vocabulary object.
 */
function hook_taxonomy_vocabulary_presave($vocabulary) {
  $vocabulary->foo = 'bar';
}

/**
 * Act on taxonomy vocabularies when inserted.
 *
 * Modules implementing this hook can act on the vocabulary object when saved
 * to the database.
 *
 * @param $vocabulary
 *   A taxonomy vocabulary object.
 */
function hook_taxonomy_vocabulary_insert($vocabulary) {
  if ($vocabulary->synonyms) {
    variable_set('taxonomy_' . $vocabulary->vid . '_synonyms', TRUE);
  }
}

/**
 * Act on taxonomy vocabularies when updated.
 *
 * Modules implementing this hook can act on the vocabulary object when updated.
 *
 * @param $vocabulary
 *   A taxonomy vocabulary object.
 */
function hook_taxonomy_vocabulary_update($vocabulary) {
  $status = $vocabulary->synonyms ? TRUE : FALSE;
  if ($vocabulary->synonyms) {
    variable_set('taxonomy_' . $vocabulary->vid . '_synonyms', $status);
  }
}

/**
 * Act before taxonomy vocabulary deletion.
 *
 * This hook is invoked from taxonomy_vocabulary_delete() before
 * field_attach_delete_bundle() is called and before the vocabulary is actually
 * removed from the database.
 *
 * @param $vocabulary
 *   The taxonomy vocabulary object for the vocabulary that is about to be
 *   deleted.
 *
 * @see hook_taxonomy_vocabulary_delete()
 * @see taxonomy_vocabulary_delete()
 */
function hook_taxonomy_vocabulary_predelete($vocabulary) {
  if (variable_get('taxonomy_' . $vocabulary->vid . '_synonyms', FALSE)) {
    variable_del('taxonomy_' . $vocabulary->vid . '_synonyms');
  }
}

/**
 * Respond to taxonomy vocabulary deletion.
 *
 * This hook is invoked from taxonomy_vocabulary_delete() after
 * field_attach_delete_bundle() has been called and after the vocabulary has
 * been removed from the database.
 *
 * @param $vocabulary
 *   The taxonomy vocabulary object for the vocabulary that has been deleted.
 *
 * @see hook_taxonomy_vocabulary_predelete()
 * @see taxonomy_vocabulary_delete()
 */
function hook_taxonomy_vocabulary_delete($vocabulary) {
  if (variable_get('taxonomy_' . $vocabulary->vid . '_synonyms', FALSE)) {
    variable_del('taxonomy_' . $vocabulary->vid . '_synonyms');
  }
}

/**
 * Act on taxonomy terms when loaded.
 *
 * Modules implementing this hook can act on the term objects returned by
 * taxonomy_term_load_multiple().
 *
 * For performance reasons, information to be added to term objects should be
 * loaded in a single query for all terms where possible.
 *
 * Since terms are stored and retrieved from cache during a page request, avoid
 * altering properties provided by the {taxonomy_term_data} table, since this
 * may affect the way results are loaded from cache in subsequent calls.
 *
 * @param $terms
 *   An array of term objects, indexed by tid.
 */
function hook_taxonomy_term_load($terms) {
  $result = db_query('SELECT tid, foo FROM {mytable} WHERE tid IN (:tids)', array(':tids' => array_keys($terms)));
  foreach ($result as $record) {
    $terms[$record->tid]->foo = $record->foo;
  }
}

/**
 * Act on taxonomy terms before they are saved.
 *
 * Modules implementing this hook can act on the term object before it is
 * inserted or updated.
 *
 * @param $term
 *   A term object.
 */
function hook_taxonomy_term_presave($term) {
  $term->foo = 'bar';
}

/**
 * Act on taxonomy terms when inserted.
 *
 * Modules implementing this hook can act on the term object when saved to
 * the database.
 *
 * @param $term
 *   A taxonomy term object.
 */
function hook_taxonomy_term_insert($term) {
  if (!empty($term->synonyms)) {
    foreach (explode ("\n", str_replace("\r", '', $term->synonyms)) as $synonym) {
      if ($synonym) {
        db_insert('taxonomy_term_synonym')
        ->fields(array(
          'tid' => $term->tid,
          'name' => rtrim($synonym),
        ))
        ->execute();
      }
    }
  }
}

/**
 * Act on taxonomy terms when updated.
 *
 * Modules implementing this hook can act on the term object when updated.
 *
 * @param $term
 *   A taxonomy term object.
 */
function hook_taxonomy_term_update($term) {
  hook_taxonomy_term_delete($term);
  if (!empty($term->synonyms)) {
    foreach (explode ("\n", str_replace("\r", '', $term->synonyms)) as $synonym) {
      if ($synonym) {
        db_insert('taxonomy_term_synonym')
        ->fields(array(
          'tid' => $term->tid,
          'name' => rtrim($synonym),
        ))
        ->execute();
      }
    }
  }
}

/**
 * Act before taxonomy term deletion.
 *
 * This hook is invoked from taxonomy_term_delete() before
 * field_attach_delete() is called and before the term is actually removed from
 * the database.
 *
 * @param $term
 *   The taxonomy term object for the term that is about to be deleted.
 *
 * @see taxonomy_term_delete()
 */
function hook_taxonomy_term_predelete($term) {
  db_delete('term_synoynm')->condition('tid', $term->tid)->execute();
}

/**
 * Respond to taxonomy term deletion.
 *
 * This hook is invoked from taxonomy_term_delete() after field_attach_delete()
 * has been called and after the term has been removed from the database.
 *
 * @param $term
 *   The taxonomy term object for the term that has been deleted.
 *
 * @see taxonomy_term_delete()
 */
function hook_taxonomy_term_delete($term) {
  db_delete('term_synoynm')->condition('tid', $term->tid)->execute();
}

/**
 * Alter the results of taxonomy_term_view().
 *
 * This hook is called after the content has been assembled in a structured
 * array and may be used for doing processing which requires that the complete
 * taxonomy term content structure has been built.
 *
 * If the module wishes to act on the rendered HTML of the term rather than the
 * structured content array, it may use this hook to add a #post_render
 * callback. Alternatively, it could also implement
 * hook_preprocess_taxonomy_term(). See backdrop_render() and theme()
 * documentation respectively for details.
 *
 * @param $build
 *   A renderable array representing the node content.
 *
 * @see hook_entity_view_alter()
 */
function hook_taxonomy_term_view_alter(&$build) {
  if ($build['#view_mode'] == 'full' && isset($build['an_additional_field'])) {
    // Change its weight.
    $build['an_additional_field']['#weight'] = -10;
  }

  // Add a #post_render callback to act on the rendered HTML of the term.
  $build['#post_render'][] = 'my_module_node_post_render';
}

/**
 * @} End of "addtogroup hooks".
 */
