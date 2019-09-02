<?php
/**
 * @file
 * Hooks for file module.
 */

/**
 * Control download access to files.
 *
 * The hook is typically implemented to limit access based on the entity the
 * file is referenced, e.g., only users with access to a node should be allowed
 * to download files attached to that node.
 *
 * @param $field
 *   The field to which the file belongs.
 * @param $entity_type
 *   The type of $entity; for example, 'node' or 'user'.
 * @param $entity
 *   The $entity to which $file is referenced.
 *
 * @return
 *   TRUE is access should be allowed by this entity or FALSE if denied. Note
 *   that denial may be overridden by another entity controller, making this
 *   grant permissive rather than restrictive.
 *
 * @see hook_field_access().
 */
function hook_file_download_access($field, $entity_type, $entity) {
  if ($entity_type == 'node') {
    return node_access('view', $entity);
  }
}

/**
 * Alter the access rules applied to a file download.
 *
 * Entities that implement file management set the access rules for their
 * individual files. Module may use this hook to create custom access rules
 * for file downloads.
 *
 * @see hook_file_download_access().
 *
 * @param $grants
 *   An array of grants gathered by hook_file_download_access(). The array is
 *   keyed by the module that defines the entity type's access control; the
 *   values are Boolean grant responses for each module.
 * @param $field
 *   The field to which the file belongs.
 * @param $entity_type
 *   The type of $entity; for example, 'node' or 'user'.
 * @param $entity
 *   The $entity to which $file is referenced.
 */
function hook_file_download_access_alter(&$grants, $field, $entity_type, $entity) {
  // For our example module, we always enforce the rules set by node module.
  if (isset($grants['node'])) {
    $grants = array('node' => $grants['node']);
  }
}

/**
 * Alter default file types.
 *
 * @param $types
 *   Array of file types.
 *
 * @see hook_file_default_types()
 */
function hook_file_default_types_alter(&$types) {
  $types['image']->mimetypes[] = 'image/svg+xml';
}

/**
 * Define file formatters.
 *
 * @return array
 *   An array whose keys are file formatter names and whose values are arrays
 *   describing the formatter. Each formatter may contain the following keys:
 *   - label: The human-readable name for the formatter.
 *   - default settings: default values for the formatter settings, if any.
 *   - view callback: a function to call when the formatter is being viewed.
 *   - settigs callback: a function to call for collecting the settings.
 *   - hidden: wheter the formatter is hidden or not.
 *   - mime types: An array of mime types this formatter applies to.
 *
 * @see hook_file_formatter_info_alter()
 */
function hook_file_formatter_info() {
  // Add a simple file formatter for displaying an image in a chosen style.
  if (module_exists('image')) {
    $formatters['file_image'] = array(
      'label' => t('Image'),
      'default settings' => array(
        'image_style' => '',
      ),
      'view callback' => 'file_file_formatter_file_image_view',
      'settings callback' => 'file_file_formatter_file_image_settings',
      'hidden' => TRUE,
      'mime types' => array('image/*'),
    );
  }
  return $formatters;
}

/**
 * Perform alterations on file formatters.
 *
 * @param array $info
 *   Array of information on file formatters exposed by
 *   hook_file_formatter_info() implementations.
 */
function hook_file_formatter_info_alter(array &$info) {
  // @todo Add example.
}

/**
 * Define formatter output.
 *
 * Note: This is not really a hook. The function name is manually specified via
 * 'view callback' in hook_file_formatter_info(), with this recommended callback
 * name pattern.
 *
 * @param object $file
 *   The file entity.
 * @param array $display
 *   An array containing settings for how to display the file.
 * @param string $langcode
 *   A language code indiciating the language used to render the file.
 */
function hook_file_formatter_FORMATTER_view($file, $display, $langcode) {
  $element = array(
    '#theme' => 'image',
    '#path' => $file->uri,
    '#width' => isset($file->override['attributes']['width']) ? $file->override['attributes']['width'] : $file->metadata['width'],
    '#height' => isset($file->override['attributes']['height']) ? $file->override['attributes']['height'] : $file->metadata['height'],
    '#alt' => token_replace($display['settings']['alt'], array('file' => $file), $replace_options),
    '#title' => token_replace($display['settings']['title'], array('file' => $file), $replace_options),
  );
  return $element;
}

/**
 * Define formatter settings.
 *
 * Note: This is not really a hook. The function name is manually specified via
 * 'settings callback' in hook_file_formatter_info(), with this recommended
 * callback name pattern.
 *
 * @param $form
 *   An array represeting the settings form.
 * @param $form_state
 *   An array representing the current state of the settings form.
 * @param $settings
 *   An array containing default settings for the form elements.
 */
function hook_file_formatter_FORMATTER_settings($form, &$form_state, $settings) {
  $element['image_style'] = array(
    '#title' => t('Image style'),
    '#type' => 'select',
    '#options' => image_style_options(FALSE),
    '#default_value' => $settings['image_style'],
    '#empty_option' => t('None (original image)'),
  );
  return $element;
}

/**
 * Add to files as they are viewed.
 *
 * @param File $file
 *   The fully loaded file object
 * @param string $view_mode
 *   The machhine name of the Display mode for viewing the file.
 * @langcode string
 *   A language code indiciating the language used to render the file.
 */
function hook_file_view($file, $view_mode, $langcode) {
  // Add a contextual link to edit the file.
  if ($view_mode != 'full' && $view_mode != 'preview') {
    $file->content['links']['file']['#links']['edit'] = l(t('Edit'), 'file/' . $file->fid);
  }
}

/**
 * Alter files as they are viewed.
 */
function hook_file_view_alter(&$build, $type) {
  // Change the contextual link to edit the file.
  if ($view_mode != 'full' && $view_mode != 'preview') {
    $build['#contextual_links']['file']['#links']['edit'] = l(t('Manage'), 'file/' . $file->fid);
  }
}

/**
 * Add mass file operations.
 *
 * This hook enables modules to inject custom operations into the mass
 * operations dropdown found at admin/content/file, by associating a callback
 * function with the operation, which is called when the form is submitted. The
 * callback function receives one initial argument, which is an array of the
 * checked files.
 *
 * @return array
 *   An array of operations. Each operation is an associative array that may
 *   contain the following key-value pairs:
 *   - 'label': Required. The label for the operation, displayed in the dropdown
 *     menu.
 *   - 'callback': Required. The function to call for the operation.
 *   - 'callback arguments': Optional. An array of additional arguments to pass
 *     to the callback function.
 */
function hook_file_operations() {
  $operations = array(
    'delete' => array(
      'label' => t('Delete selected files'),
      'callback' => NULL,
    ),
  );
  return $operations;
}

/**
 * Control access to a file.
 *
 * Modules may implement this hook if they want to have a say in whether or not
 * a given user has access to perform a given operation on a file.
 *
 * The administrative account (user ID #1) always passes any access check,
 * so this hook is not called in that case. Users with the "bypass file access"
 * permission may always view and edit files through the administrative
 * interface.
 *
 * Note that not all modules will want to influence access on all
 * file types. If your module does not want to actively grant or
 * block access, return FILE_ACCESS_IGNORE or simply return nothing.
 * Blindly returning FALSE will break other file access modules.
 *
 * @param string $op
 *   The operation to be performed. Possible values:
 *   - "create"
 *   - "delete"
 *   - "update"
 *   - "view"
 *   - "download".
 * @param object $file
 *   The file on which the operation is to be performed, or, if it does
 *   not yet exist, the type of file to be created.
 * @param object $account
 *   A user object representing the user for whom the operation is to be
 *   performed.
 *
 * @return string|null
 *   FILE_ACCESS_ALLOW if the operation is to be allowed;
 *   FILE_ACCESS_DENY if the operation is to be denied;
 *   FILE_ACCESS_IGNORE to not affect this operation at all.
 *
 * @ingroup file_access
 */
function hook_file_access($op, $file, $account) {
  $type = is_string($file) ? $file : $file->type;

  if ($op !== 'create' && (REQUEST_TIME - $file->timestamp) < 3600) {
    // If the file was uploaded in the last hour, deny access to it.
    return FILE_ACCESS_DENY;
  }

  // Returning nothing from this function would have the same effect.
  return FILE_ACCESS_IGNORE;
}

/**
 * Control access to listings of files.
 *
 * @param object $query
 *   A query object describing the composite parts of a SQL query related to
 *   listing files.
 *
 * @see hook_query_TAG_alter()
 * @ingroup file_access
 */
function hook_query_file_access_alter(QueryAlterableInterface $query) {
  // Only show files that have been uploaded more than an hour ago.
  $query->condition('timestamp', REQUEST_TIME - 3600, '<=');
}

/**
 * Act on a file being displayed as a search result.
 *
 * This hook is invoked from file_search_execute(), after file_load()
 * and file_view() have been called.
 *
 * @param object $file
 *   The file being displayed in a search result.
 *
 * @return array
 *   Extra information to be displayed with search result. This information
 *   should be presented as an associative array. It will be concatenated
 *   with the file information (filename) in the default search result theming.
 *
 * @see template_preprocess_search_result()
 * @see search-result.tpl.php
 *
 * @ingroup file_api_hooks
 */
function hook_file_search_result($file) {
  $file_usage_count = db_query('SELECT count FROM {file_usage} WHERE fid = :fid', array('fid' => $file->fid))->fetchField();
  return array(
    'file_usage_count' => format_plural($file_usage_count, '1 use', '@count uses'),
  );
}

/**
 * Act on a file being indexed for searching.
 *
 * This hook is invoked during search indexing, after file_load(), and after
 * the result of file_view() is added as $file->rendered to the file object.
 *
 * @param object $file
 *   The file being indexed.
 *
 * @return string
 *   Additional file information to be indexed.
 *
 * @ingroup file_api_hooks
 */
function hook_file_update_index($file) {
  $text = '';
  $uses = db_query('SELECT module, count FROM {file_usage} WHERE fid = :fid', array(':fid' => $file->fid));
  foreach ($uses as $use) {
    $text .= '<h2>' . check_plain($use->module) . '</h2>' . check_plain($use->count);
  }
  return $text;
}

/**
 * Provide additional methods of scoring for core search results for files.
 *
 * A file's search score is used to rank it among other files matched by the
 * search, with the highest-ranked files appearing first in the search listing.
 *
 * For example, a module allowing users to vote on files could expose an
 * option to allow search results' rankings to be influenced by the average
 * voting score of a file.
 *
 * All scoring mechanisms are provided as options to site administrators, and
 * may be tweaked based on individual sites or disabled altogether if they do
 * not make sense. Individual scoring mechanisms, if enabled, are assigned a
 * weight from 1 to 10. The weight represents the factor of magnification of
 * the ranking mechanism, with higher-weighted ranking mechanisms having more
 * influence. In order for the weight system to work, each scoring mechanism
 * must return a value between 0 and 1 for every file. That value is then
 * multiplied by the administrator-assigned weight for the ranking mechanism,
 * and then the weighted scores from all ranking mechanisms are added, which
 * brings about the same result as a weighted average.
 *
 * @return array
 *   An associative array of ranking data. The keys should be strings,
 *   corresponding to the internal name of the ranking mechanism, such as
 *   'recent', or 'usage'. The values should be arrays themselves, with the
 *   following keys available:
 *   - "title": the human readable name of the ranking mechanism. Required.
 *   - "join": part of a query string to join to any additional necessary
 *     table. This is not necessary if the table required is already joined to
 *     by the base query, such as for the {file_managed} table. Other tables
 *     should use the full table name as an alias to avoid naming collisions.
 *     Optional.
 *   - "score": part of a query string to calculate the score for the ranking
 *     mechanism based on values in the database. This does not need to be
 *     wrapped in parentheses, as it will be done automatically; it also does
 *     not need to take the weighted system into account, as it will be done
 *     automatically. It does, however, need to calculate a decimal between
 *     0 and 1; be careful not to cast the entire score to an integer by
 *     inadvertently introducing a variable argument. Required.
 *   - "arguments": if any arguments are required for the score, they can be
 *     specified in an array here.
 *
 * @ingroup file_api_hooks
 */
function hook_file_ranking() {
  // If voting is disabled, we can avoid returning the array, no hard feelings.
  if (variable_get('vote_file_enabled', TRUE)) {
    return array(
      'vote_average' => array(
        'title' => t('Average vote'),
        // Note that we use i.sid, the search index's search item id, rather
        // than fm.fid.
        'join' => 'LEFT JOIN {vote_file_data} vote_file_data ON vote_file_data.fid = i.sid',
        // The highest possible score should be 1,
        // and the lowest possible score, always 0, should be 0.
        'score' => 'vote_file_data.average / CAST(%f AS DECIMAL)',
        // Pass in the highest possible voting score as a decimal argument.
        'arguments' => array(variable_get('vote_score_max', 5)),
      ),
    );
  }
}

/**
 * Alter file download headers.
 *
 * @param array $headers
 *   Array of download headers.
 * @param object $file
 *   File object.
 */
function hook_file_download_headers_alter(array &$headers, $file) {
  // Instead of being powered by PHP, tell the world this resource was powered
  // by your custom module!
  $headers['X-Powered-By'] = 'My Module';
}

/**
 * React to a file being downloaded.
 */
function hook_file_transfer($uri, array $headers) {
  // Redirect a download for an S3 file to the actual location.
  if (file_uri_scheme($uri) == 's3') {
    $url = file_create_url($uri);
    drupal_goto($url);
  }
}

/**
 * Decides which file type (bundle) should be assigned to a file entity.
 *
 * @param object $file
 *   File object.
 *
 * @return array
 *   Array of file type machine names that can be assigned to a given file type.
 *   If there are more proposed file types the one, that was returned the first,
 *   wil be chosen. This can be, however, changed in alter hook.
 *
 * @see hook_file_type_alter()
 */
function hook_file_type($file) {
  // Assign all files uploaded by anonymous users to a special file type.
  if (user_is_anonymous()) {
    return array('untrusted_files');
  }
}

/**
 * Alters list of file types that can be assigned to a file.
 *
 * @param array $types
 *   List of proposed types.
 * @param object $file
 *   File object.
 */
function hook_file_type_alter(array &$types, $file) {
  // Choose a specific, non-first, file type.
  $types = array($types[4]);
}

/**
 * Provides metadata information.
 *
 * @return array
 *   An array of metadata information.
 */
function hook_file_metadata_info() {
  // @todo Add example.
}

/**
 * Alters metadata information.
 *
 * @todo Add documentation.
 *
 * @return array
 *   an array of metadata information.
 */
function hook_file_metadata_info_alter() {
  // @todo Add example.
}

/**
 * Alters skip fields status.
 *
 * Use this to choose to skip or complete step 4 of the file upload process.
 *
 * @param bool &$skip_fields
 *   Set to TRUE to skip the form for editing extra file entity fields.
 * @param array $form_state
 *   State array of the current upload form.
 */
function hook_file_upload_skip_fields_alter(&$skip_fields, $form_state) {
  if ($form_state['file']->type == 'video') {
    $skip_fields = TRUE;
  }
}
