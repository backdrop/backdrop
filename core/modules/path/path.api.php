<?php

/**
 * @file
 * Hooks provided by the Path module.
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
 * Added pathauto.api.php functions; moving pathauto contrib to
 * backdrop/core/path
 */
/**
 * @file
 * Documentation for pathauto API.
 *
 * It may be helpful to review some examples of integration from
 * pathauto.pathauto.inc.
 *
 * Pathauto works by using tokens in path patterns.  Thus the simplest
 * integration is just to provide tokens.  Token support is provided by Drupal
 * core. To provide additional token from your module, implement the following
 * hooks:
 *
 * hook_tokens() - http://api.drupal.org/api/function/hook_tokens
 * hook_token_info() - http://api.drupal.org/api/function/hook_token_info
 *
 * If you wish to provide pathauto integration for custom paths provided by your
 * module, there are a few steps involved.
 *
 * 1. hook_pathauto()
 *    Provide information required by pathauto for the settings form as well as
 *    bulk generation. See the documentation for hook_pathauto() for more
 *    details.
 *
 * 2. pathauto_create_alias()
 *    At the appropriate time (usually when a new item is being created for
 *    which a generated alias is desired), call pathauto_create_alias() with the
 *    appropriate parameters to generate and create the alias. See the user,
 *    taxonomy, and node hook implementations in pathauto.module for examples.
 *    Also see the documentation for pathauto_create_alias().
 *
 * 3. pathauto_path_delete_all()
 *    At the appropriate time (usually when an item is being deleted), call
 *    pathauto_path_delete_all() to remove any aliases that were created for the
 *    content being removed.  See the documentation for
 *    pathauto_path_delete_all() for more details.
 *
 * 4. hook_path_alias_types()
 *    For modules that create new types of content that can be aliased with
 *    pathauto, a hook implementation is needed to allow the user to delete them
 *    all at once.  See the documentation for hook_path_alias_types() below for
 *    more information.
 *
 * There are other integration points with pathauto, namely alter hooks that
 * allow you to change the data used by pathauto at various points in the
 * process.  See the below hook documentation for details.
 */

/**
 * Used primarily by the bulk delete form.  This hooks provides pathauto the
 * information needed to bulk delete aliases created by your module.  The keys
 * of the return array are used by pathauto as the system path prefix to delete
 * from the url_aliases table.  The corresponding value is simply used as the
 * label for each type of path on the bulk delete form.
 *
 * @return
 *   An array whose keys match the beginning of the source paths
 *   (e.g.: "node/", "user/", etc.) and whose values describe the type of page
 *   (e.g.: "Content", "Users"). Like all displayed strings, these descriptions
 *   should be localized with t(). Use % to match interior pieces of a path,
 *   like "user/%/track". This is a database wildcard (meaning "user/%/track"
 *   matches "user/1/track" as well as "user/1/view/track").
 */
function hook_path_alias_types() {
  $objects['user/'] = t('Users');
  $objects['node/'] = t('Content');
  return $objects;
}

/**
 * Provide information about the way your module's aliases will be built.
 *
 * The information you provide here is used to build the form
 * on search/path/patterns. File pathauto.pathauto.inc provides example
 * implementations for system modules.
 *
 * @see node_pathauto()
 *
 * @param $op
 *   At the moment this will always be 'settings'.
 *
 * @return object|null
 *   An object, or array of objects (if providing multiple groups of path
 *   patterns).  Each object should have the following members:
 *   - 'module': The module or entity type.
 *   - 'token_type': Which token type should be allowed in the patterns form.
 *   - 'group_header': Translated label for the settings group
 *   - 'pattern_description': The translated label for the default pattern (e.g.,
 *      t('Default path pattern (applies to all content types with blank
 *      patterns below)')
 *   - 'pattern_default': Default pattern  (e.g. 'content/[node:title]'
 *   - 'batch_update_callback': The name of function that should be ran for
 *      bulk update. @see node_path_bulk_update_batch_process for example
 *   - 'batch_file': The name of the file with the bulk update function.
 *   - 'pattern_items': Optional. An array of descriptions keyed by bundles.
 */
function hook_path($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'file';
      $settings['token_type'] = 'file';
      $settings['group_header'] = t('File paths');
      $settings['pattern_description'] = t('Default path pattern (applies to all file types with blank patterns below)');
      $settings['pattern_default'] = 'files/[file:name]';
      $settings['batch_update_callback'] = 'file_entity_path_bulk_update_batch_process';
      $settings['batch_file'] = backdrop_get_path('module', 'file_entity') . '/file_entity.pathauto.inc';

      foreach (file_type_get_enabled_types() as $file_type => $type) {
        $settings['pattern_items'][$file_type] = t('Pattern for all @file_type paths.', array('@file_type' => $type->label));
      }
      return (object) $settings;

    default:
      break;
  }
}

/**
 * Determine if a possible URL alias would conflict with any existing paths.
 *
 * Returning TRUE from this function will trigger pathauto_alias_uniquify() to
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
 * @see pathauto_alias_uniquify()
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
