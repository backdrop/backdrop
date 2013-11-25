<?php
/**
 * @file
 * Administrative pages and callbacks for the Configuration management module.
 */

/**
 * Form callback; Build the form for syncing all staged configuration.
 */
function config_sync_form(array $form, array &$form_state) {
  $config_statuses = config_get_statuses();

  if (empty($config_statuses)) {
    $form['no_changes'] = array(
      '#theme' => 'table',
      '#header' => array('Name', 'Operations'),
      '#rows' => array(),
      '#empty' => t('There are no configuration changes.'),
    );
    $form['actions']['#access'] = FALSE;
    return $form;
  }

  // Add the AJAX library to the form for dialog support.
  $form['#attached']['library'][] = array('system', 'drupal.ajax');

  foreach ($config_statuses as $config_change_type => $config_files) {
    if (empty($config_files)) {
      continue;
    }

    // @todo A table caption would be more appropriate, but does not have the
    //   visual importance of a heading.
    $form[$config_change_type]['heading'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h3',
    );
    switch ($config_change_type) {
      case 'create':
        $form[$config_change_type]['heading']['#value'] = format_plural(count($config_files), '@count new', '@count new');
        break;

      case 'update':
        $form[$config_change_type]['heading']['#value'] = format_plural(count($config_files), '@count changed', '@count changed');
        break;

      case 'delete':
        $form[$config_change_type]['heading']['#value'] = format_plural(count($config_files), '@count removed', '@count removed');
        break;
    }
    $form[$config_change_type]['list'] = array(
      '#theme' => 'table',
      '#header' => array('Name', 'Operations'),
    );

    foreach ($config_files as $config_file) {
      $links['view_diff'] = array(
        '#type' => 'link',
        '#title' => t('View differences'),
        '#href' => 'admin/config/development/configuration/diff/' . $config_file,
      );
      $form[$config_change_type]['list']['#rows'][] = array(
        'name' => $config_file,
        'operations' => array(
          'data' => $links,
        ),
      );
    }
  }

  $form['actions'] = array(
    '#type' => 'actions'
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import all'),
    '#access' => count($config_statuses) > 0,
  );

  return $form;
}

/**
 * Submit handler for config_sync_form().
 */
function config_sync_form_submit(array &$form, array &$form_state) {
  $config_importer = new ConfigImporter(
    $form_state['storage_comparer'],
    $this->eventDispatcher,
    $this->configFactory,
    $this->entity_manager,
    $this->lock,
    $this->uuidService
  );
  if ($config_importer->alreadyImporting()) {
    drupal_set_message(t('Another request may be synchronizing configuration already.'));
  }
  else{
    try {
      $config_importer->import();
      drupal_flush_all_caches();
      drupal_set_message(t('The configuration was imported successfully.'));
    }
    catch (ConfigException $e) {
      // Return a negative result for UI purposes. We do not differentiate
      // between an actual synchronization error and a failed lock, because
      // concurrent synchronizations are an edge-case happening only when
      // multiple developers or site builders attempt to do it without
      // coordinating.
      watchdog_exception('config_import', $e);
      drupal_set_message(t('The import failed due to an error. Any errors have been logged.'), 'error');
    }
  }

  return $form;
}

/**
 * Shows diff of specified configuration file.
 *
 * @param string $config_file
 *   The name of the configuration file.
 *
 * @return string
 *   Table showing a two-way diff between the active and staged configuration.
 */
function config_diff_page($config_file) {
  $diff = config_diff($config_file);

  $build = array();

  $build['#title'] = t('View changes of @config_file', array('@config_file' => $config_file));
  // Add the CSS for the inline diff.
  $build['#attached']['css'][] = drupal_get_path('module', 'system') . '/css/system.diff.css';

  $build['diff'] = array(
    '#theme' => 'table',
    '#header' => array(
      array('data' => t('Old'), 'colspan' => '2'),
      array('data' => t('New'), 'colspan' => '2'),
    ),
    '#rows' => $diff,
  );

  $build['back'] = array(
    '#type' => 'link',
    '#attributes' => array(
      'class' => array(
        'dialog-cancel',
      ),
    ),
    '#title' => "Back to 'Synchronize configuration' page.",
    '#href' => 'admin/config/development/configuration',
  );

  return $build;
}

/**
 * Form callback; Provide a form for exporting the current configuration.
 */
function config_export_full_form(array $form, array $form_state) {
  $form['description'] = array(
    '#markup' => '<p>' . t('Use the export button below to download your site configuration.') . '</p>',
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
  );
  return $form;
}

/**
 * Submit handler for config_export_full_form().
 */
function config_export_full_form_submit(array $form, array &$form_state) {
  $form_state['redirect'] = 'admin/development/config/export';
}

/**
 * Downloads a tarball of the site configuration.
 */
function config_download_export() {
  $filename = file_create_filename('config.tar.gz', file_directory_temp());
  $archiver = new ArchiverTar('temporary://' . $filename, 'gz');
  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);
  $config_files = array();
  foreach ($config_storage->listAll() as $config_name) {
    $config_files[] = $config_dir . '/' . $config_name . '.json';
  }
  $archiver->getArchive()->createModify($config_files, '', config_get_config_directory());

  file_download('temporary', 'config.tar.gz');
  file_unmanaged_delete('temporary://config.tar.gz');
}

/**
 * Form callback; Provide a form for importing a full set of configuration.
 */
function config_import_full_form(array &$form, array &$form_state) {
  $form['description'] = array(
    '#markup' => '<p>' . t('Use the upload button below.') . '</p>',
  );
  $form['import_tarball'] = array(
    '#type' => 'file',
    '#title' => t('Select your configuration export file'),
    '#description' => t('This form will redirect you to the import configuration screen.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
  );
  return $form;
}

/**
 * Validate handler for config_import_full_form().
 */
function config_import_full_form_validate(array &$form, array &$form_state) {
  if (!empty($_FILES['files']['error']['import_tarball'])) {
    form_set_error('import_tarball', t('The import tarball could not be uploaded.'));
  }
  else {
    $form_state['values']['import_tarball'] = $_FILES['files']['tmp_name']['import_tarball'];
  }
}

/**
 * Submit handler for config_import_full_form().
 */
function config_import_full_form_submit(array &$form, array &$form_state) {
  if ($path = $form_state['values']['import_tarball']) {
    $this->configStorage->deleteAll();
    try {
      $archiver = new ArchiveTar($path, 'gz');
      $files = array();
      foreach ($archiver->listContent() as $file) {
        $files[] = $file['filename'];
      }
      $archiver->extractList($files, config_get_config_directory(CONFIG_STAGING_DIRECTORY));
      drupal_set_message(t('Your configuration files were successfully uploaded, ready for import.'));
      $form_state['redirect_route']['route_name'] = 'config.sync';
    }
    catch (\Exception $e) {
      form_set_error('import_tarball', t('Could not extract the contents of the tar file. The error message is <em>@message</em>', array('@message' => $e->getMessage())));
    }
    drupal_unlink($path);
  }
}

/**
 * Form callback; Builds teh form for exporting a single configuration file.
 */
function config_export_single_form(array $form, array &$form_state, $config_type = NULL, $config_name = NULL) {
  foreach ($this->entityManager->getDefinitions() as $entity_type => $definition) {
    if (isset($definition['config_prefix']) && isset($definition['entity_keys']['uuid'])) {
      $this->definitions[$entity_type] = $definition;
    }
  }
  $entity_types = array_map(function ($definition) {
    return $definition['label'];
  }, $this->definitions);
  // Sort the entity types by label, then add the simple config to the top.
  uasort($entity_types, 'strnatcasecmp');
  $config_types = array(
      'system.simple' => t('Simple configuration'),
    ) + $entity_types;
  $form['config_type'] = array(
    '#title' => t('Configuration type'),
    '#type' => 'select',
    '#options' => $config_types,
    '#default_value' => $config_type,
    '#ajax' => array(
      'callback' => 'config_export_single_form_update_type',
      'wrapper' => 'edit-config-type-wrapper',
    ),
  );
  $default_type = isset($form_state['values']['config_type']) ? $form_state['values']['config_type'] : $config_type;
  $form['config_name'] = array(
    '#title' => t('Configuration name'),
    '#type' => 'select',
    '#options' => $this->findConfiguration($default_type),
    '#default_value' => $config_name,
    '#required' => TRUE,
    '#prefix' => '<div id="edit-config-type-wrapper">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'callback' => 'config_export_single_form_update_export',
      'wrapper' => 'edit-export-wrapper',
    ),
  );

  $form['export'] = array(
    '#title' => t('Here is your configuration:'),
    '#type' => 'textarea',
    '#rows' => 24,
    '#required' => TRUE,
    '#prefix' => '<div id="edit-export-wrapper">',
    '#suffix' => '</div>',
  );
  if ($config_type && $config_name) {
    $fake_form_state = array('values' => array(
      'config_type' => $config_type,
      'config_name' => $config_name,
    ));
    $form['export'] = $this->updateExport($form, $fake_form_state);
  }
  return $form;
}

/**
 * Handles switching the configuration type selector.
 */
function config_export_single_form_update_type($form, &$form_state) {
  $form['config_name']['#options'] = $this->findConfiguration($form_state['values']['config_type']);
  return $form['config_name'];
}

/**
 * AJAX submit handler for config_export_single_form().
 *
 * Handles switching the export textarea.
 */
function config_export_single_form_update_export($form, &$form_state) {
  // Determine the full config name for the selected config entity.
  if ($form_state['values']['config_type'] !== 'system.simple') {
    $definition = $this->entityManager->getDefinition($form_state['values']['config_type']);
    $name = $definition['config_prefix'] . '.' . $form_state['values']['config_name'];
  }
  // The config name is used directly for simple configuration.
  else {
    $name = $form_state['values']['config_name'];
  }
  // Read the raw data for this config name, encode it, and display it.
  $data = $this->configStorage->read($name);
  $form['export']['#value'] = $this->configStorage->encode($data);
  $form['export']['#description'] = t('The filename is %name.', array('%name' => $name . '.yml'));
  return $form['export'];
}

/**
 * Handles switching the configuration type selector.
 */
function _config_export_single_form_switch($config_type) {
  $names = array(
    '' => t('- Select -'),
  );
  // For a given entity type, load all entities.
  if ($config_type && $config_type !== 'system.simple') {
    $entity_storage = $this->entityManager->getStorageController($config_type);
    foreach ($entity_storage->loadMultiple() as $entity) {
      $entity_id = $entity->id();
      $label = $entity->label() ?: $entity_id;
      $names[$entity_id] = $label;
    }
  }
  // Handle simple configuration.
  else {
    // Gather the config entity prefixes.
    $config_prefixes = array_map(function ($definition) {
      return $definition['config_prefix'] . '.';
    }, $this->definitions);

    // Find all config, and then filter our anything matching a config prefix.
    $names = MapArray::copyValuesToKeys($this->configStorage->listAll());
    foreach ($names as $config_name) {
      foreach ($config_prefixes as $config_prefix) {
        if (strpos($config_name, $config_prefix) === 0) {
          unset($names[$config_name]);
        }
      }
    }
  }
  return $names;
}

/**
 * Form callback; Build the form to import a single configuration file.
 */
function config_import_single_form(array $form, array &$form_state) {
  $entity_types = array();
  foreach ($this->entityManager->getDefinitions() as $entity_type => $definition) {
    if (isset($definition['config_prefix']) && isset($definition['entity_keys']['uuid'])) {
      $entity_types[$entity_type] = $definition['label'];
    }
  }
  // Sort the entity types by label, then add the simple config to the top.
  uasort($entity_types, 'strnatcasecmp');
  $config_types = array(
      'system.simple' => t('Simple configuration'),
    ) + $entity_types;
  $form['config_type'] = array(
    '#title' => t('Configuration type'),
    '#type' => 'select',
    '#options' => $config_types,
    '#required' => TRUE,
  );
  $form['config_name'] = array(
    '#title' => t('Configuration name'),
    '#type' => 'textfield',
    '#states' => array(
      'required' => array(
        ':input[name="config_type"]' => array('value' => 'system.simple'),
      ),
      'visible' => array(
        ':input[name="config_type"]' => array('value' => 'system.simple'),
      ),
    ),
  );
  $form['import'] = array(
    '#title' => t('Paste your configuration here'),
    '#type' => 'textarea',
    '#rows' => 24,
    '#required' => TRUE,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#button_type' => 'primary',
  );
  return $form;
}

/**
 * Validate handler for config_import_single_form().
 */
function config_import_single_form_validate(array &$form, array &$form_state) {
  // Decode the submitted import.
  $data = $this->configStorage->decode($form_state['values']['import']);

  // Validate for config entities.
  if ($form_state['values']['config_type'] !== 'system.simple') {
    $definition = $this->entityManager->getDefinition($form_state['values']['config_type']);
    $id_key = $definition['entity_keys']['id'];
    $entity_storage = $this->entityManager->getStorageController($form_state['values']['config_type']);
    // If an entity ID was not specified, set an error.
    if (!isset($data[$id_key])) {
      form_set_error('import', t('Missing ID key "@id_key" for this @entity_type import.', array('@id_key' => $id_key, '@entity_type' => $definition['label'])));
      return;
    }
    $uuid_key = $definition['entity_keys']['uuid'];
    // If there is an existing entity, ensure matching ID and UUID.
    if ($entity = $entity_storage->load($data[$id_key])) {
      $this->configExists = $entity;
      if (!isset($data[$uuid_key])) {
        form_set_error('import', t('An entity with this machine name already exists but the import did not specify a UUID.'));
        return;
      }
      if ($data[$uuid_key] !== $entity->uuid()) {
        form_set_error('import', t('An entity with this machine name already exists but the UUID does not match.'));
        return;
      }
    }
    // If there is no entity with a matching ID, check for a UUID match.
    elseif (isset($data[$uuid_key]) && $entity_storage->loadByProperties(array($uuid_key => $data[$uuid_key]))) {
      form_set_error('import', t('An entity with this UUID already exists but the machine name does not match.'));
    }
  }
  else {
    $config = $this->config($form_state['values']['config_name']);
    $this->configExists = $config->isNew() ? $config : FALSE;
  }

  // Store the decoded version of the submitted import.
  form_set_value($form['import'], $data, $form_state);
}

/**
 * Submit handler for config_import_single_form().
 */
function config_import_single_form_submit(array &$form, array &$form_state) {
  // If this form has not yet been confirmed, store the values and rebuild.
  if (!$this->data) {
    $form_state['rebuild'] = TRUE;
    $this->data = $form_state['values'];
    return;
  }

  // If a simple configuration file was added, set the data and save.
  if ($this->data['config_type'] === 'system.simple') {
    $this->config($this->data['config_name'])->setData($this->data['import'])->save();
    drupal_set_message(t('The %name configuration was imported.', array('%name' => $this->data['config_name'])));
  }
  // For a config entity, create a new entity and save it.
  else {
    try {
      $entity = $this->entityManager
        ->getStorageController($this->data['config_type'])
        ->create($this->data['import']);
      $entity->save();
      drupal_set_message(t('The @entity_type %label was imported.', array('@entity_type' => $entity->entityType(), '%label' => $entity->label())));
    }
    catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
  }
}

/**
 * Utility function to get the current diffs between active and staging.
 */
function config_get_statuses() {
  $active_directory = config_get_config_directory('active');
  $staging_directory = config_get_config_directory('staging');
  $active_storage = new ConfigFileStorage($active_directory);
  $staging_storage = new ConfigFileStorage($staging_directory);
  $active_files = $active_storage->listAll();
  $staging_files = $staging_storage->listAll();

  $config_statuses = array();
  if ($staging_files) {
    if ($delete_diffs = array_diff($active_files, $staging_files)) {
      $config_statuses['delete'] = $delete_diffs;
    }
    if ($create_diffs = array_diff($staging_files, $active_files)) {
      $config_statuses['create'] = $create_diffs;
    }
    if ($remaining_diffs = array_diff($active_files, $delete_diffs)) {
      foreach ($remaining_diffs as $file) {
        $active_config = $active_storage->read($file);
        $staging_config = $staging_storage->read($file);
        if ($active_config != $staging_config) {
          $config_statuses['update'][] = $file;
        }
      }
    }
  }

  return $config_statuses;
}

/**
 * Return a formatted diff of a named config between staging and active.
 *
 * @param string $name
 *   The name of the configuration object to diff.
 *
 * @return array
 *   An array of formatted strings showing the diffs between the two storages.
 */
function config_diff($name) {
  $active_directory = config_get_config_directory('active');
  $staging_directory = config_get_config_directory('staging');
  $active_storage = new ConfigFileStorage($active_directory);
  $staging_storage = new ConfigFileStorage($staging_directory);

  $source_data = explode("\n", drupal_json_encode($active_storage->read($name), TRUE));
  $target_data = explode("\n", drupal_json_encode($staging_storage->read($name), TRUE));

  // Check for new or removed files.
  if ($source_data === array('false')) {
    // Added file.
    $source_data = array(t('File added'));
  }
  if ($target_data === array('false')) {
    // Deleted file.
    $target_data = array(t('File removed'));
  }

  $diff = new Diff($source_data, $target_data);
  $formatter = new BackdropDiffFormatter();
  return $formatter->format($diff);
}
