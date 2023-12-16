<?php
/**
 * @file
 * Theme functions for the Taxonomy module.
 */

/**
 * Returns HTML for the vocabulary human readable name, and its machine name.
 *
 * @param $variables
 *   An associative array containing:
 *   - 'vocabulary': An object containing the human readable name, and the
 *     machine name of the vocabulary.
 *
 * @see taxonomy_overview_vocabularies()
 * @ingroup themeable
 */
function theme_taxonomy_vocabulary_name($variables) {
  $vocabulary = $variables['vocabulary'];

  $output = '';
  $output .= '<div class="vocabulary-label">' . check_plain($vocabulary->name) . '</div>';
  $output .= '<div class="vocabulary-machine-name">(<span>' . check_plain($vocabulary->machine_name) . '</span>)</div>';
  return $output;
}

/**
 * Returns HTML for the vocabulary description.
 *
 * @param $variables
 *   An associative array containing:
 *   - 'vocabulary': An object containing the description of the vocabulary.
 *
 * @see taxonomy_overview_vocabularies()
 * @ingroup themeable
 */
function theme_taxonomy_vocabulary_description($variables) {
  $vocabulary = $variables['vocabulary'];

  $output = '';
  if ($vocabulary->description) {
    $output = '<div class="vocabulary-description">' . filter_xss_admin($vocabulary->description) . '</div>';
  }
  return $output;
}

/**
 * Returns HTML for a terms overview form as a sortable list of terms.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see taxonomy_overview_terms()
 * @ingroup themeable
 */
function theme_taxonomy_overview_terms($variables) {
  $form = $variables['form'];

  $page_increment = $form['#page_increment'];
  $page_entries = $form['#page_entries'];
  $back_step = $form['#back_step'];
  $forward_step = $form['#forward_step'];

  if (taxonomy_vocabulary_access('update', $form['#vocabulary'])) {
    // Add drag and drop if parent fields are present in the form.
    if ($form['#parent_fields']) {
      backdrop_add_tabledrag('taxonomy', 'match', 'parent', 'term-parent', 'term-parent', 'term-id', FALSE);
      backdrop_add_tabledrag('taxonomy', 'depth', 'group', 'term-depth', NULL, NULL, FALSE);
      backdrop_add_js(backdrop_get_path('module', 'taxonomy') . '/js/taxonomy.admin.js');
      backdrop_add_js(array('taxonomy' => array('backStep' => $back_step, 'forwardStep' => $forward_step)), 'setting');
      backdrop_add_css(backdrop_get_path('module', 'taxonomy') . '/css/taxonomy.css');
    }
    backdrop_add_tabledrag('taxonomy', 'order', 'sibling', 'term-weight');
  }

  $errors = form_get_errors() != FALSE ? form_get_errors() : array();
  $rows = array();
  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#term'])) {
      $term = &$form[$key];

      $row = array();
      $row[] = (isset($term['#term']['depth']) && $term['#term']['depth'] > 0 ? theme('indentation', array('size' => $term['#term']['depth'])) : ''). backdrop_render($term['view']);
      if ($form['#parent_fields']) {
        $term['tid']['#attributes']['class'] = array('term-id');
        $term['parent']['#attributes']['class'] = array('term-parent');
        $term['depth']['#attributes']['class'] = array('term-depth');
        $row[0] .= backdrop_render($term['parent']) . backdrop_render($term['tid']) . backdrop_render($term['depth']);
      }
      if (taxonomy_vocabulary_access('update', $form['#vocabulary'])) {
        $term['weight']['#attributes']['class'] = array('term-weight');
      }
      if (module_exists('language') && $form['#vocabulary']->language == TAXONOMY_LANGUAGE_ENABLED) {
        $row[] = backdrop_render($term['langcode']);
      }
      if (taxonomy_vocabulary_access('update', $form['#vocabulary'])) {
        $row[] = backdrop_render($term['weight']);
      }
      $row[] = backdrop_render($term['operations']);
      $row = array('data' => $row);
      $rows[$key] = $row;
    }
  }

  // Add necessary classes to rows.
  $row_position = 0;
  foreach ($rows as $key => $row) {
    $rows[$key]['class'] = array();
    if (isset($form['#parent_fields'])) {
      $rows[$key]['class'][] = 'draggable';
    }

    // Add classes that mark which terms belong to previous and next pages.
    if ($row_position < $back_step || $row_position >= $page_entries - $forward_step) {
      $rows[$key]['class'][] = 'taxonomy-term-preview';
    }

    if ($row_position !== 0 && $row_position !== count($rows) - 1) {
      if ($row_position == $back_step - 1 || $row_position == $page_entries - $forward_step - 1) {
        $rows[$key]['class'][] = 'taxonomy-term-divider-top';
      }
      elseif ($row_position == $back_step || $row_position == $page_entries - $forward_step) {
        $rows[$key]['class'][] = 'taxonomy-term-divider-bottom';
      }
    }

    // Add an error class if this row contains a form error.
    foreach ($errors as $error_key => $error) {
      if (strpos($error_key, $key) === 0) {
        $rows[$key]['class'][] = 'error';
      }
    }
    $row_position++;
  }

  $output = '';

  if (module_exists('language') && $form['#vocabulary']->language == TAXONOMY_LANGUAGE_ENABLED) {
    $form['langcode_term']['#field_suffix'] = backdrop_render($form['langcode_filter']);
    $output .= backdrop_render($form['langcode_term']);
  }

  $header = array();
  $header[] = t('Name');
  if (module_exists('language') && $form['#vocabulary']->language == TAXONOMY_LANGUAGE_ENABLED) {
    $header[] = t('Language');
  }
  if (taxonomy_vocabulary_access('update', $form['#vocabulary'])) {
    $header[] = t('Weight');
  }
  $header[] = t('Operations');
  $output .= theme('table', array('header' => $header,
    'rows' => $rows, 'attributes' => array('id' => 'taxonomy'),
    'empty' => $form['#empty_text'],
  ));
  $output .= backdrop_render_children($form);
  $output .= theme('pager');

  return $output;
}

/**
 * Process variables for taxonomy-term.tpl.php.
 */
function template_preprocess_taxonomy_term(&$variables) {
  $variables['view_mode'] = $variables['elements']['#view_mode'];
  $variables['term'] = $variables['elements']['#term'];
  $term = $variables['term'];

  $uri = entity_uri('taxonomy_term', $term);
  $variables['term_url']  = url($uri['path'], $uri['options']);
  $variables['term_name'] = check_plain($term->name);
  $variables['page']      = $variables['view_mode'] == 'full' && taxonomy_term_is_page($term);

  // Flatten the term object's member fields.
  $variables = array_merge((array) $term, $variables);

  // Helpful $content variable for templates.
  $variables['content'] = array();
  foreach (element_children($variables['elements']) as $key) {
    $variables['content'][$key] = $variables['elements'][$key];
  }

  // field_attach_preprocess() overwrites the $[field_name] variables with the
  // values of the field in the language that was selected for display, instead
  // of the raw values in $term->[field_name], which contain all values in all
  // languages.
  field_attach_preprocess('taxonomy_term', $term, $variables['content'], $variables);

  // Gather classes, and clean up name so there are no underscores.
  $vocabulary_name_css = backdrop_clean_css_identifier($term->vocabulary);
  $variables['classes'][] = 'vocabulary-' . $vocabulary_name_css;
}

