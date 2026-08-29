<?php

/**
 * Devel Generate batch handling functions using the BatchAPI.
 */

/**
 * Form constructor generating in batches.
 */

function devel_generate_batch_content($form_state) {
  $operations = array();

  // Setup the batch operations and save the variables.
  $operations[] = array('devel_generate_batch_content_pre_node', array($form_state['values']));

  // Add the kill operation.
  if ($form_state['values']['kill_content']) {
    $operations[] = array('devel_generate_batch_content_kill', array());
  }

  // Add the operations to create the nodes.
  for ($num = 0; $num < $form_state['values']['num_nodes']; $num ++) {
    $operations[] = array('devel_generate_batch_content_add_node', array());
  }

  // Start the batch.
  $batch = array(
    'title' => t('Generating Content'),
    'operations' => $operations,
    'finished' => 'devel_generate_batch_finished',
    'file' => backdrop_get_path('module', 'devel_generate') . '/devel_generate_batch.inc',
  );
  batch_set($batch);
}

function devel_generate_batch_content_kill(&$context) {
  module_load_include('inc', 'devel_generate', 'devel_generate');
  devel_generate_content_kill($context['results']);
}

function devel_generate_batch_content_pre_node($vars, &$context) {
  $context['results'] = $vars;
  $context['results']['num_nids'] = 0;
  module_load_include('inc', 'devel_generate', 'devel_generate');
  devel_generate_content_pre_node($context['results']);
}

/**
 * Batch API callback: Generate nodes.
 */
function devel_generate_batch_content_add_node(&$context) {
  module_load_include('inc', 'devel_generate', 'devel_generate');
  devel_generate_content_add_node($context['results']);
  $context['results']['num_nids'] ++;
}

/**
 * Display a message when a batch is complete.
 */
function devel_generate_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = t('Finished @num_nids nodes created successfully.', array('@num_nids' => $results['num_nids']));
  }
  else {
    $message = t('Finished with an error.');
  }
  backdrop_set_message($message);
}

