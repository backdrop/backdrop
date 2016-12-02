<?php
/**
 * @file
 * Menu callbacks and forms for Devel Generate module.
 */

/**
 * Generates users using FormAPI.
 */
function devel_generate_users_form($form, &$form_state) {
  $form['num'] = array(
    '#type' => 'number',
    '#title' => t('How many users would you like to generate?'),
    '#default_value' => 50,
    '#min' => 1,
    '#max' => 99999,
  );
  $form['kill_users'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete all users (except user id 1) before generating new users.'),
    '#default_value' => FALSE,
  );
  $options = user_roles(TRUE);
  unset($options[BACKDROP_AUTHENTICATED_ROLE]);
  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Which roles should the users receive?'),
    '#description' => t('Users always receive the <em>authenticated user</em> role.'),
    '#options' => $options,
  );

  $options = array(1 => t('Now'));
  foreach (array(3600, 86400, 604800, 2592000, 31536000) as $interval) {
    $options[$interval] = format_interval($interval, 1) . ' ' . t('ago');
  }
  $form['time_range'] = array(
    '#type' => 'select',
    '#title' => t('How old should user accounts be?'),
    '#description' => t('User ages will be distributed randomly from the current time, back to the selected time.'),
    '#options' => $options,
    '#default_value' => 31536000,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );
  return $form;
}

/**
 * FormAPI submission to generate users.
 */
function devel_generate_users_form_submit($form_id, &$form_state) {
  module_load_include('inc', 'devel_generate');
  $values = $form_state['values'];
  devel_create_users($values['num'], $values['kill_users'], $values['time_range'], $values['roles']);
}

/**
 * Generates nodes using FormAPI.
 */
function devel_generate_content_form($form, &$form_state) {
  $options = array();

  $types = node_type_get_types();
  $suffix = '';
  foreach ($types as $type) {
    if (module_exists('comment')) {
      $default = $type->settings['comment_default'];
      $map = array(t('Hidden'), t('Closed'), t('Open'));
      $suffix = ' <small>('. t('Comments: ') . $map[$default]. ')</small>';
    }
    $options[$type->type] = t($type->name) . $suffix;
  }

  if (empty($options)) {
    backdrop_set_message(t('You do not have any content types that can be generated. <a href="@create-type">Go create a new content type</a> already!</a>', array('@create-type' => url('admin/structure/types/add'))), 'error', FALSE);
    return $form;
  }

  $form['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content types'),
    '#options' => $options,
    '#default_value' => array_keys($options),
  );
  if (module_exists('checkall')) $form['node_types']['#checkall'] = TRUE;
  $form['kill_content'] = array(
    '#type' => 'checkbox',
    '#title' => t('<strong>Delete all content</strong> in these content types before generating new content.'),
    '#default_value' => FALSE,
  );
  $form['num_nodes'] = array(
    '#type' => 'number',
    '#title' => t('How many nodes would you like to generate?'),
    '#default_value' => 50,
    '#min' => 1,
    '#max' => 999999,
  );

  $options = array(1 => t('Now'));
  foreach (array(3600, 86400, 604800, 2592000, 31536000) as $interval) {
    $options[$interval] = format_interval($interval, 1) . ' ' . t('ago');
  }
  $form['time_range'] = array(
    '#type' => 'select',
    '#title' => t('How far back in time should the nodes be dated?'),
    '#description' => t('Node creation dates will be distributed randomly from the current time, back to the selected time.'),
    '#options' => $options,
    '#default_value' => 31536000,
  );

  $form['max_comments'] = array(
    '#type' => module_exists('comment') ? 'number' : 'value',
    '#title' => t('Maximum number of comments per node.'),
    '#description' => t('Only content types that have comments enabled will receive comments. Note that some nodes will randomly receive zero comments. Some will receive the max.'),
    '#default_value' => 0,
    '#min' => 0,
    '#max' => 99999,
    '#access' => module_exists('comment'),
  );
  $form['title_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum number of words in titles'),
    '#default_value' => 4,
    '#min' => 1,
    '#max' => 100,
  );
  $form['add_alias'] = array(
    '#type' => 'checkbox',
    '#disabled' => !module_exists('path'),
    '#description' => t('Content types will use the default patterns when generated. Checking this box will add a custom alias to all content types that do not have a pattern specified.'),
    '#title' => t('Ensure all content has a URL alias.'),
    '#default_value' => FALSE,
  );

  unset($options);
  $options[LANGUAGE_NONE] = t('Language neutral');
  if (module_exists('locale')) {
    $languages = language_list(TRUE);
    foreach ($languages as $language) {
      $options[$language->langcode] = $language->name;
    }
  }
  $form['add_language'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Set language on nodes'),
    '#multiple' => TRUE,
    '#access' => module_exists('locale'),
    '#description' => t('Requires locale.module'),
    '#options' => $options,
    '#default_value' => array(LANGUAGE_NONE),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );
  $form['#redirect'] = FALSE;

  return $form;
}

/**
 * FormAPI submission to generate nodes.
 */
function devel_generate_content_form_submit($form_id, &$form_state) {
  module_load_include('inc', 'devel_generate', 'devel_generate');
  $form_state['values']['node_types'] = array_filter($form_state['values']['node_types']);
  if ($form_state['values']['num_nodes'] <= 50 && $form_state['values']['max_comments'] <= 10) {
    module_load_include('inc', 'devel_generate');
    devel_generate_content($form_state);
  }
  else {
    module_load_include('inc', 'devel_generate', 'devel_generate_batch');
    devel_generate_batch_content($form_state);
  }
}

/**
 * Generates taxonomy terms using FormAPI.
 */
function devel_generate_term_form($form, &$form_state) {
  $options = array();
  foreach (taxonomy_get_vocabularies() as $vid => $vocab) {
    $options[$vid] = $vocab->name;
  }
  $form['vids'] = array(
    '#type' => 'checkboxes',
    '#multiple' => TRUE,
    '#title' => t('Generate terms for these vocabularies'),
    '#required' => TRUE,
    '#options' => $options,
  );
  $form['num_terms'] = array(
    '#type' => 'number',
    '#title' => t('Number of terms?'),
    '#default_value' => 10,
    '#min' => 1,
    '#max' => 99999,
  );
  $form['title_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum number of characters in term names'),
    '#default_value' => 12,
    '#min' => 1,
    '#max' => 1000,
  );
  $form['kill_taxonomy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete existing terms in specified vocabularies before generating new terms.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );

  return $form;
}

/**
 * Generates taxonomy vocabularies using FormAPI.
 */
function devel_generate_vocab_form($form, &$form_state) {
  $form['num_vocabs'] = array(
    '#type' => 'number',
    '#title' => t('Number of vocabularies?'),
    '#default_value' => 1,
    '#min' => 1,
    '#max' => 999,
  );
  $form['title_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum number of characters in vocabulary names'),
    '#default_value' => 12,
    '#min' => 1,
    '#max' => 1000,
  );
  $form['kill_taxonomy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete existing vocabularies before generating new ones.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );

  return $form;
}

/**
 * FormAPI submission to generate taxonomy terms.
 */
function devel_generate_term_form_submit($form_id, &$form_state) {
  module_load_include('inc', 'devel_generate');
  $vocabs = taxonomy_vocabulary_load_multiple($form_state['values']['vids']);
  devel_generate_term_data($vocabs, $form_state['values']['num_terms'], $form_state['values']['title_length'], $form_state['values']['kill_taxonomy']);
}

/**
 * FormAPI submission to generate taxonomy vocabularies.
 */
function devel_generate_vocab_form_submit($form_id, &$form_state) {
  module_load_include('inc', 'devel_generate');
  devel_generate_vocab_data($form_state['values']['num_vocabs'], $form_state['values']['title_length'], $form_state['values']['kill_taxonomy']);
}

/**
 * Inserts nodes properly based on generation options.
 *
 * @param $node
 *  The base node created on submit. Inspects $node->devel_generate.
 */
function devel_generate_node_insert($node) {
  if (isset($node->devel_generate)) {
    $results = $node->devel_generate;

    // Add comments if configured and enabled on the given type of node.
    if (!empty($results['max_comments']) && $node->comment >= COMMENT_NODE_OPEN) {
      devel_generate_add_comments($node, $results['users'], $results['max_comments'], $results['title_length']);
    }

    // Add an url alias. Cannot happen before save because we don't know the nid.
    if (!empty($results['add_alias'])) {
      // Check if an alias will be made automatically.
      module_load_include('inc', 'path');
      if (function_exists('path_generate_entity_alias')) {
        $alias = path_generate_entity_alias($node);
        if (!$alias) {
          $path = array(
            'source' => 'node/' . $node->nid,
            'alias' => 'node-' . $node->nid . '-' . $node->type,
          );
          path_save($path);
        }
      }
    }
  }
}

/**
 * Set a message for either drush or the web interface.
 *
 * @param $msg
 *  The message to display.
 * @param $type
 *  The message type, as defined by backdrop_set_message().
 *
 * @return
 *  Context-appropriate message output.
 */
function devel_generate_set_message($msg, $type = 'status') {
  $function  = function_exists('drush_log') ? 'drush_log' : 'backdrop_set_message';
  $function($msg, $type);
}

/**
 * Generates menus using FormAPI.
 */
function devel_generate_menu_form() {
  $menu_enabled = module_exists('menu');
  if ($menu_enabled) {
    $menus = menu_get_menus() + array('__new-menu__' => t('Create new menu(s)'));
  }
  else {
    $menus = menu_list_system_menus();
  }
  $form['existing_menus'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Generate links for these menus'),
    '#options' => $menus,
    '#default_value' => array('__new-menu__'),
    '#required' => TRUE,
  );
  if ($menu_enabled) {
    $form['num_menus'] = array(
      '#type' => 'number',
      '#title' => t('Number of new menus to create'),
      '#default_value' => 2,
      '#min' => 1,
      '#max' => 999,
      '#states' => array(
        'visible' => array(
          ':input[name="existing_menus[__new-menu__]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }
  $form['num_links'] = array(
    '#type' => 'number',
    '#title' => t('Number of links to generate'),
    '#default_value' => 50,
    '#min' => 1,
    '#max' => 10000,
    '#required' => TRUE,
  );
  $form['title_length'] = array(
    '#type' => 'number',
    '#title' => t('Maximum number of characters in menu and menu link names'),
    '#default_value' => 12,
    '#min' => 1,
    '#max' => 1000,
    '#required' => TRUE,
  );
  $form['link_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Types of links to generate'),
    '#options' => array(
      'node' => t('Nodes'),
      'front' => t('Front page'),
      'external' => t('External'),
    ),
    '#default_value' => array('node', 'front', 'external'),
    '#required' => TRUE,
  );
  $form['max_depth'] = array(
    '#type' => 'select',
    '#title' => t('Maximum link depth'),
    '#options' => range(0, MENU_MAX_DEPTH),
    '#default_value' => floor(MENU_MAX_DEPTH / 2),
    '#required' => TRUE,
  );
  unset($form['max_depth']['#options'][0]);
  $form['max_width'] = array(
    '#type' => 'number',
    '#title' => t('Maximum menu width'),
    '#default_value' => 6,
    '#min' => 1,
    '#max' => 100,
    '#description' => t("Limit the width of the generated menu's first level of links to a certain number of items."),
    '#required' => TRUE,
  );
  $form['kill'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete existing custom generated menus and menu links before generating new ones.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );
  return $form;
}

/**
 * FormAPI submission to generate menus.
 */
function devel_generate_menu_form_submit($form_id, &$form_state) {
  // If the create new menus checkbox is off, set the number of new menus to 0.
  if (!isset($form_state['values']['existing_menus']['__new-menu__']) || !$form_state['values']['existing_menus']['__new-menu__']) {
    $form_state['values']['num_menus'] = 0;
  }
  module_load_include('inc', 'devel_generate');
  devel_generate_menu_data($form_state['values']['num_menus'], $form_state['values']['existing_menus'], $form_state['values']['num_links'], $form_state['values']['title_length'], $form_state['values']['link_types'], $form_state['values']['max_depth'], $form_state['values']['max_width'], $form_state['values']['kill']);
}
