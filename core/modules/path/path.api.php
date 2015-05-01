<?php
/**
 * @file
 * Hooks provided by the Path module.
 *
 * Path module provides automatic path aliasing by using tokens in path
 * patterns. Thus the simplest integration is just to provide tokens using
 * hook_token_info() and hook_tokens().
 *
 * If you wish to provide automatic path creation for custom paths provided by
 * your module, there are a few steps involved.
 *
 * 1. hook_path_info()
 *    Provide information required by Path for the settings form as well as
 *    bulk generation. See the documentation for hook_path_info() for more
 *    details.
 *
 * 2. path_generate_entity_alias()
 *    At the appropriate time (usually when a new item is being created for
 *    which a generated alias is desired), call path_generate_entity_alias()
 *    with the appropriate parameters to generate the alias. Then save the
 *    alias with path_save_automatic_alias(). See the user, taxonomy, and node
 *    hook implementations for examples.
 *
 * 3. path_delete_all_by_source()
 *    At the appropriate time (usually when an item is being deleted), call
 *    path_delete_all_by_source() to remove any aliases that were created for the
 *    content being removed. See the documentation for path_delete_all_by_source() for
 *    more details.
 *
 * There are other integration points with Path module, namely alter hooks that
 * allow you to change the data used by Path at various points in the
 * process. See the below hook documentation for details.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Respond to a path being inserted.
 *
 * @param $path
 *   An associative array containing the following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: Unique path alias identifier.
 *   - langcode: The language code of the alias.
 *
 * @see path_save()
 */
function hook_path_insert($path) {
  db_insert('mytable')
    ->fields(array(
      'alias' => $path['alias'],
      'pid' => $path['pid'],
    ))
    ->execute();
}

/**
 * Respond to a path being updated.
 *
 * @param $path
 *   An associative array containing the following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: Unique path alias identifier.
 *   - langcode: The language code of the alias.
 *
 * @see path_save()
 */
function hook_path_update($path) {
  db_update('mytable')
    ->fields(array('alias' => $path['alias']))
    ->condition('pid', $path['pid'])
    ->execute();
}

/**
 * Respond to a path being deleted.
 *
 * @param $path
 *   An associative array containing the following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: Unique path alias identifier.
 *   - langcode: The language code of the alias.
 *
 * @see path_delete()
 */
function hook_path_delete($path) {
  db_delete('mytable')
    ->condition('pid', $path['pid'])
    ->execute();
}

/**
 * @} End of "addtogroup hooks".
 */

/**
 * Provide information about the way your module's aliases will be built.
 *
 * The information you provide here is used to build the form
 * on search/path/patterns
 *
 * @return array
 *   A 2-level array of automatic path settings. Each item should have a unique
 *   key (often the name of the providing module). Each sub-array should contain
 *   the following:
 *   - token_type: Which token type should be allowed in the patterns form.
 *   - group_header: Translated label for the settings group
 *   - pattern_description: The translated label for the default pattern (e.g.,
 *       t('Default path pattern (applies to all content types with blank
 *       patterns below)')
 *   - pattern_default: Default pattern  (e.g. 'content/[node:title]')
 *   - batch_update_callback: The name of function that should be ran for
 *       bulk update. See node_path_bulk_update_batch_process() for an example.
 *   - batch_file: The name of the file with the bulk update function.
 *   - pattern_items: Optional. An array of descriptions keyed by bundles.
 */
function hook_path_info() {
  $info['file'] = array(
    'module' => 'file',
    'token_type' => 'file',
    'group_header' => t('File paths'),
    'pattern_description' => t('Default path pattern (applies to all file types with blank patterns below)'),
    'pattern_default' => 'files/[file:name]',
    'batch_update_callback' => 'file_entity_path_bulk_update_batch_process',
    'batch_file' => backdrop_get_path('module', 'file') . '/file.path.inc',
  );

  foreach (file_type_get_enabled_types() as $file_type => $type) {
    $info['file']['pattern_items'][$file_type] = t('Pattern for all @file_type paths.', array('@file_type' => $type->label));
  }
  return $info;
}

/**
 * Determine if a possible URL alias would conflict with any existing paths.
 *
 * Returning TRUE from this function will trigger path_alias_uniquify() to
 * generate a similar URL alias with a suffix to avoid conflicts.
 *
 * @param string $alias
 *   The potential URL alias.
 * @param string $source
 *   The source path for the alias (e.g. 'node/1').
 * @param string $langcode
 *   The language code for the alias (e.g. 'en').
 *
 * @return bool
 *   TRUE if $alias conflicts with an existing, reserved path, or FALSE/NULL if
 *   it does not match any reserved paths.
 *
 * @see path_alias_uniquify()
 */
function hook_path_is_alias_reserved($alias, $source, $langcode) {
  // Check our module's list of paths and return TRUE if $alias matches any of
  // them.
  return (bool) db_query("SELECT 1 FROM {mytable} WHERE path = :path", array(':path' => $alias))->fetchField();
}

/**
 * Alter the pattern to be used before an alias is generated by Pathauto.
 *
 * @param string $pattern
 *   The alias pattern for Pathauto to pass to token_replace() to generate the
 *   URL alias.
 * @param array $context
 *   An associative array of additional options, with the following elements:
 *   - 'module': The module or entity type being aliased.
 *   - 'op': A string with the operation being performed on the object being
 *     aliased. Can be either 'insert', 'update', 'return', or 'bulkupdate'.
 *   - 'source': A string of the source path for the alias (e.g. 'node/1').
 *   - 'data': An array of keyed objects to pass to token_replace().
 *   - 'type': The sub-type or bundle of the object being aliased.
 *   - 'langcode': A string of the language code for the alias (e.g. 'en').
 */
function hook_path_pattern_alter(&$pattern, array &$context) {
  // Switch out any [node:created:*] tokens with [node:updated:*] on update.
  if ($context['module'] == 'node' && ($context['op'] == 'update')) {
    $pattern = preg_replace('/\[node:created(\:[^]]*)?\]/', '[node:updated$1]', $pattern);
  }
}

/**
 * Alter Pathauto-generated aliases before saving.
 *
 * @param string $alias
 *   The automatic alias after token replacement and strings cleaned.
 * @param array $context
 *   An associative array of additional options, with the following elements:
 *   - 'module': The module or entity type being aliased.
 *   - 'op': A string with the operation being performed on the object being
 *     aliased. Can be either 'insert', 'update', 'return', or 'bulkupdate'.
 *   - 'source': A string of the source path for the alias (e.g. 'node/1').
 *     This can be altered by reference.
 *   - 'data': An array of keyed objects to pass to token_replace().
 *   - 'type': The sub-type or bundle of the object being aliased.
 *   - 'langcode': A string of the language code for the alias (e.g. 'en').
 *   - 'pattern': A string of the pattern used for aliasing the object.
 */
function hook_path_alias_alter(&$alias, array &$context) {
  // Add a suffix so that all aliases get saved as 'content/my-title.html'
  $alias .= '.html';

  // Force all aliases to be saved as language neutral.
  $context['langcode'] = LANGUAGE_NONE;
}

/**
 * Alter the list of punctuation characters for Pathauto control.
 *
 * @param $punctuation
 *   An array of punctuation to be controlled by Pathauto during replacement
 *   keyed by punctuation name. Each punctuation record should be an array
 *   with the following key/value pairs:
 *   - value: The raw value of the punctuation mark.
 *   - name: The human-readable name of the punctuation mark. This must be
 *     translated using t() already.
 */
function hook_path_punctuation_chars_alter(array &$punctuation) {
  // Add the trademark symbol.
  $punctuation['trademark'] = array('value' => '™', 'name' => t('Trademark symbol'));

  // Remove the dollar sign.
  unset($punctuation['dollar']);
}
