<?php

/**
 * Return an array for setting up a Batch API job to sync all configuration.
 */
function config_sync_batch($config_statuses = NULL) {
  if (empty($config_statuses)) {
    $config_statuses = config_get_statuses();
  }
  $operations[] = array('_config_sync_start', array($config_statuses));
  $operations[] = array('_config_sync_validate', array());
  $operations[] = array('_config_sync_import', array());
  $batch = array(
    'operations' => $operations,
    'title' => t('Importing configuration'),
    'init_message' => t('Starting import'),
    'error_message' => t('An error occurred while importing the configuration.'),
    'file' => drupal_get_path('module', 'config') . '/includes/config.sync.inc',
    'finished' => '_config_sync_finished',
  );
  return $batch;
}

/**
 * Batch API callback. Start the config import.
 */
function _config_sync_start($config_statuses, &$context) {
  // @todo: Switch to a proper state system, since variables should be
  // deprecated by the configuration system.
  variable_set('config_sync', REQUEST_TIME);

  // The complete list of changes are stored in the results array. This makes
  // these values available in all subsequent batch callbacks.
  $context['results']['all_configs'] = $config_statuses;
  $context['results']['pending_changes'] = array_filter($config_statuses);
  $context['results']['completed_changes'] = array();
}

/**
 * Batch API callback. Validate the changes before attempting to sync.
 */
function _config_sync_validate(&$context) {
  if (!isset($context['sandbox']['batch_size'])) {
    $context['sandbox']['batch_size'] = variable_get('config_sync_batch_size', 50);
    $context['sandbox']['total_size'] = count($context['results']['pending_changes']);
    $context['sandbox']['current_index'] = 0;
  }

  $config_files = array_slice($context['results']['pending_changes'], $context['sandbox']['current_index'], $context['sandbox']['batch_size']);
  try {
    foreach ($config_files as $config_file => $config_change_type) {
      switch ($config_change_type) {
        case 'create':
          $staging_config = config($config_file, 'staging')->load();
          module_invoke_all('config_create_validate', $staging_config, $context['results']['all_configs']);
          break;
        case 'update':
          $staging_config = config($config_file, 'staging')->load();
          $active_config = config($config_file, 'active')->load();
          module_invoke_all('config_update_validate', $staging_config, $active_config, $context['results']['all_configs']);
          break;
        case 'delete':
          $active_config = config($config_file, 'active')->load();
          module_invoke_all('config_delete_validate', $active_config, $context['results']['all_configs']);
          break;
      }
      $context['sandbox']['current_index']++;
    }
  }
  catch (ConfigException $e) {
    $context['results']['errors'][] = $e->getMessage();
    $context['finished'] = 1;
    return;
  }

  $context['finished'] = $context['sandbox']['total_size'] ? $context['sandbox']['current_index'] / $context['sandbox']['total_size'] : 1;
}

/**
 * Batch API callback. Import the individual config files.
 */
function _config_sync_import(&$context) {
  // Bail if any errors have been set during validation.
  if (isset($context['results']['errors'])) {
    $context['finished'] = 1;
    return;
  }

  if (!isset($context['sandbox']['batch_size'])) {
    $context['sandbox']['batch_size'] = variable_get('config_sync_batch_size', 50);
    $context['sandbox']['total_size'] = count($context['results']['pending_changes']);
    $context['sandbox']['current_index'] = 0;
  }

  $config_files = array_slice($context['results']['pending_changes'], $context['sandbox']['current_index'], $context['sandbox']['batch_size']);
  foreach ($config_files as $config_file => $config_change_type) {
    switch ($config_change_type) {
      case 'create':
        $staging_config = config($config_file, 'staging')->load();
        $active_config = config($config_file, 'active')->setData($staging_config->get());
        module_invoke_all('config_create', $active_config);
        $active_config->save();
        break;
      case 'update':
        $staging_config = config($config_file, 'staging')->load();
        $active_config = config($config_file, 'active')->load();
        module_invoke_all('config_update', $staging_config, $active_config);
        $active_config->setData($staging_config->get())->save();
        break;
      case 'delete':
        $active_config = config($config_file, 'active')->load();
        module_invoke_all('config_delete', $active_config);
        $active_config->delete();
        break;
    }
    unset($context['results']['pending_changes'][$config_file]);
    $context['results']['completed_changes'][$config_file] = $config_change_type;
    $context['sandbox']['current_index']++;
  }

  $context['finished'] = $context['sandbox']['total_size'] ? $context['sandbox']['current_index'] / $context['sandbox']['total_size'] : 1;
}

/**
 * Batch API callback. Finish the config import.
 */
function _config_sync_finished($status, $results, &$context) {
  if ($status === FALSE) {
    drupal_set_message(t('Configuration sync failed. Check that the configuration files are properly formatted.'), 'error');
  }
  if (!empty($results['errors'])) {
    drupal_set_message(t('Configuration sync failed. The following errors were reported:') . ' ' . theme('item_list', array('items' => $results['errors'])), 'error');
  }
  if (!empty($results['completed_changes'])) {
    drupal_set_message(t('Configuration sync completed. @files configuration files synced.', array('@files' => count($results['completed_changes']))));

    // Clean up the staging directory.
    $config_dir = config_get_config_directory('staging');
    $config_storage = new ConfigFileStorage($config_dir);
    $config_storage->deleteAll();
  }
  variable_del('config_sync');
}
