<?php

/**
 * @file
 * Administrative page callbacks for the Forum module.
 */

/**
 * Page callback: Returns a form for creating a new forum.
 *
 * @param $edit
 *   (optional) Associative array containing a forum term to be edited.
 *   Defaults to an empty array.
 *
 * @return
 *   A form for creating a new forum.
 *
 * @see forum_menu()
 */
function forum_form_main($edit = array()) {
  $edit = (array) $edit;
  if ((isset($_POST['op']) && $_POST['op'] == t('Delete')) || !empty($_POST['confirm'])) {
    return backdrop_get_form('forum_confirm_delete', $edit['tid']);
  }
  return backdrop_get_form('forum_form_forum', $edit);
}

/**
 * Form constructor for adding and editing a forum.
 *
 * @param $edit
 *   (optional) Associative array containing a forum term to be added or edited.
 *   Defaults to an empty array.
 *
 * @see forum_form_submit()
 * @ingroup forms
 */
function forum_form_forum($form, &$form_state, $edit = array()) {
  $edit += array(
    'name' => '',
    'description' => '',
    'tid' => NULL,
    'weight' => 0,
  );
  $form['name'] = array('#type' => 'textfield',
    '#title' => t('Forum name'),
    '#default_value' => $edit['name'],
    '#maxlength' => 255,
    '#description' => t('Short but meaningful name for this collection of threaded discussions.'),
    '#required' => TRUE,
  );
  $form['description'] = array('#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $edit['description'],
    '#description' => t('Description and guidelines for discussions within this forum.'),
  );
  $form['parent']['#tree'] = TRUE;
  $form['parent'][0] = _forum_parent_select($edit['tid'], t('Parent'));
  $form['weight'] = array('#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $edit['weight'],
    '#description' => t('Forums are displayed in ascending order by weight (forums with equal weights are displayed alphabetically).'),
  );

  $form['vocabulary'] = array('#type' => 'hidden', '#value' => config_get('forum.settings', 'forum_nav_vocabulary'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ($edit['tid']) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['tid'] = array('#type' => 'hidden', '#value' => $edit['tid']);
  }
  $form['#submit'][] = 'forum_form_submit';
  $form['#theme'] = 'forum_form';

  return $form;
}

/**
 * Form submission handler for forum_form_forum().
 */
function forum_form_submit($form, &$form_state) {
  $term = entity_create('taxonomy_term', $form_state['values']);

  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_NEW:
      $form_state['values']['tid'] = $term->tid;
      backdrop_set_message(t('Created new forum %term.', array('%term' => $form_state['values']['name'])));
      break;
    case SAVED_UPDATED:
      backdrop_set_message(t('The forum %term has been updated.', array('%term' => $form_state['values']['name'])));
      // Clear the page and block caches to avoid stale data.
      cache_clear_all();
      break;
  }
  $form_state['redirect'] = 'admin/structure/forum';
  return;
}

/**
 * Returns HTML for a forum form.
 *
 * By default this does not alter the appearance of a form at all, but is
 * provided as a convenience for themers.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_forum_form($variables) {
  return backdrop_render_children($variables['form']);
}

/**
 * Form constructor for confirming deletion of a forum taxonomy term.
 *
 * @param $tid
 *   ID of the term to be deleted.
 *
 * @see forum_confirm_delete_submit()
 * @ingroup forms
 */
function forum_confirm_delete($form, &$form_state, $tid) {
  $term = taxonomy_term_load($tid);

  $form['tid'] = array('#type' => 'value', '#value' => $tid);
  $form['name'] = array('#type' => 'value', '#value' => $term->name);

  return confirm_form($form, t('Are you sure you want to delete the forum %name?', array('%name' => $term->name)), 'admin/structure/forum', t('Deleting a forum will also delete its sub-forums, if any. To delete posts in this forum, visit <a href="@content">content administration</a> first. This action cannot be undone.', array('@content' => url('admin/content'))), t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for forum_confirm_delete().
 */
function forum_confirm_delete_submit($form, &$form_state) {
  taxonomy_term_delete($form_state['values']['tid']);
  backdrop_set_message(t('The forum %term and all sub-forums have been deleted.', array('%term' => $form_state['values']['name'])));
  watchdog('content', 'forum: deleted %term and all its sub-forums.', array('%term' => $form_state['values']['name']));

  $form_state['redirect'] = 'admin/structure/forum';
  return;
}

/**
 * Form constructor for the forum settings page.
 *
 * @see forum_menu()
 * @see system_settings_form()
 * @ingroup forms
 */
function forum_admin_settings($form) {
  $config = config('forum.settings');
  $number = backdrop_map_assoc(array(5, 10, 15, 20, 25, 30, 35, 40, 50, 60, 80, 100, 150, 200, 250, 300, 350, 400, 500));
  $form['forum_hot_topic'] = array('#type' => 'select',
    '#title' => t('Hot topic threshold'),
    '#default_value' => $config->get('forum_hot_topic'),
    '#options' => $number,
    '#description' => t('The number of replies a topic must have to be considered "hot".'),
  );
  
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );    
  
  return $form;
}

function forum_admin_settings_submit($form, $form_state) {
  $config = config('forum.settings');
  $config->set('forum_hot_topic', $form_state['values']['forum_hot_topic']);
  $config->save();
}

/**
 * Form constructor for the forum overview form.
 *
 * Returns a form for controlling the hierarchy of existing forums.
 *
 * @see forum_menu()
 * @ingroup forms
 */
function forum_overview($form, &$form_state) {
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');
  $config = config('forum.settings');
  $vid = $config->get('forum_nav_vocabulary');
  $vocabulary = taxonomy_vocabulary_load($vid);
  $form = taxonomy_overview_terms($form, $form_state, $vocabulary);

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#term'])) {
      $term = $form[$key]['#term'];
      $form[$key]['view']['#href'] = 'forum/' . $term['tid'];
      $form[$key]['operations']['#links']['edit']['title'] = t('edit forum');
      $form[$key]['operations']['#links']['edit']['href'] = 'admin/structure/forum/edit/forum/' . $term['tid'];
    }
  }

  // Remove the alphabetical reset.
  unset($form['actions']['reset_alphabetical']);

  // The form needs to have submit and validate handlers set explicitly.
  $form['#theme'] = 'taxonomy_overview_terms';
  $form['#submit'] = array('taxonomy_overview_terms_submit'); // Use the existing taxonomy overview submit handler.
  $form['#empty_text'] = t('No forums available. <a href="@forum">Add forum</a>.', array('@forum' => url('admin/structure/forum/add/forum')));
  return $form;
}

/**
 * Returns a select box for available parent terms.
 *
 * @param $tid
 *   ID of the term that is being added or edited.
 * @param $title
 *   Title for the select box.
 *
 * @return
 *   A select form element.
 */
function _forum_parent_select($tid, $title) {

  $parents = taxonomy_term_load_parents($tid);
  if ($parents) {
    $parent = array_shift($parents);
    $parent = $parent->tid;
  }
  else {
    $parent = 0;
  }

  $vid = config_get('forum.settings', 'forum_nav_vocabulary');
  $children = taxonomy_get_tree($vid, $tid);

  // A term can't be the child of itself, nor of its children.
  foreach ($children as $child) {
    $exclude[] = $child->tid;
  }
  $exclude[] = $tid;

  $tree = taxonomy_get_tree($vid);
  $options[0] = '<' . t('root') . '>';
  if ($tree) {
    foreach ($tree as $term) {
      if (!in_array($term->tid, $exclude)) {
        $options[$term->tid] = str_repeat(' -- ', $term->depth) . $term->name;
      }
    }
  }
  $description = t('Forums may be placed at the top (root) level, or inside another forum.');

  return array('#type' => 'select', '#title' => $title, '#default_value' => $parent, '#options' => $options, '#description' => $description, '#required' => TRUE);
}
