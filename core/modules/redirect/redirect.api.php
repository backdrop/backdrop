<?php

/**
 * @file
 * Hooks provided by the Redirect module.
 */

/**
 * @defgroup redirect_api_hooks Redirect API Hooks
 * @{
 * During redirect operations (create, update, view, delete, etc.), there are
 * several sets of hooks that get invoked to allow modules to modify the
 * redirect operation:
 * - All-module hooks: Generic hooks for "redirect" operations. These are
 *   always invoked on all modules.
 * - Entity hooks: Generic hooks for "entity" operations. These are always
 *   invoked on all modules.
 *
 * Here is a list of the redirect and entity hooks that are invoked, and other
 * steps that take place during redirect operations:
 * - Creating a new redirect (calling redirect_save() on a new redirect):
 *   - hook_redirect_presave() (all)
 *   - Redirect written to the database
 *   - hook_redirect_insert() (all)
 *   - hook_entity_insert() (all)
 * - Updating an existing redirect (calling redirect_save() on an existing redirect):
 *   - hook_redirect_presave() (all)
 *   - Redirect written to the database
 *   - hook_redirect_update() (all)
 *   - hook_entity_update() (all)
 * - Loading a redirect (calling redirect_load(), redirect_load_multiple(), or
 *   entity_load() with $entity_type of 'redirect'):
 *   - Redirect information is read from database.
 *   - hook_entity_load() (all)
 *   - hook_redirect_load() (all)
 * - Deleting a redirect (calling redirect_delete() or redirect_delete_multiple()):
 *   - Redirect is loaded (see Loading section above)
 *   - Redirect information is deleted from database
 *   - hook_redirect_delete() (all)
 *   - hook_entity_delete() (all)
 * - Preparing a redirect for editing (note that if it's
 *   an existing redirect, it will already be loaded; see the Loading section
 *   above):
 *   - hook_redirect_prepare() (all)
 * - Validating a redirect during editing form submit (calling
 *   redirect_form_validate()):
 *   - hook_redirect_validate() (all)
 * @}
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Act on redirects being loaded from the database.
 *
 * This hook is invoked during redirect loading, which is handled by
 * entity_load(), via classes RedirectController and
 * DefaultEntityController. After the redirect information is read from
 * the database or the entity cache, hook_entity_load() is invoked on all
 * implementing modules, and then hook_redirect_load() is invoked on all
 * implementing modules.
 *
 * This hook should only be used to add information that is not in the redirect
 * table, not to replace information that is in that table (which could
 * interfere with the entity cache). For performance reasons, information for
 * all available redirects should be loaded in a single query where possible.
 *
 * The $types parameter allows for your module to have an early return (for
 * efficiency) if your module only supports certain redirect types.
 *
 * @param $redirects
 *   An array of the redirects being loaded, keyed by rid.
 * @param $types
 *   An array containing the types of the redirects.
 *
 * @ingroup redirect_api_hooks
 */
function hook_redirect_load(array &$redirects, $types) {

}

/**
 * Alter the list of redirects matching a certain source.
 *
 * @param $redirects
 *   An array of redirect objects.
 * @param $source
 *   The source request path.
 * @param $context
 *   An array with the following key/value pairs:
 *   - langcode: The language code of the source request.
 *   - query: An array of the source request query string.
 *
 * @see redirect_load_by_source()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_load_by_source_alter(array &$redirects, $source, array $context) {
  foreach ($redirects as $rid => $redirect) {
    if ($redirect->source !== $source) {
      // If the redirects to do not exactly match $source (e.g. case
      // insensitive matches), then remove them from the results.
      unset($redirects[$rid]);
    }
  }
}

/**
 * Control access to a redirect.
 *
 * Modules may implement this hook if they want to have a say in whether or not
 * a given user has access to perform a given operation on a redirect.
 *
 * The administrative account (user ID #1) always passes any access check,
 * so this hook is not called in that case. Users with the "administer redirects"
 * permission may always update and delete redirects through the administrative
 * interface.
 *
 * Note that not all modules will want to influence access on all
 * redirect types. If your module does not want to actively grant or
 * block access, return REDIRECT_ACCESS_IGNORE or simply return nothing.
 * Blindly returning FALSE will break other redirect access modules.
 *
 * @param $redirect
 *   The redirect object on which the operation is to be performed, or, if it
 *   does not yet exist, the type of redirect to be created.
 * @param $op
 *   The operation to be performed. Possible values:
 *   - "create"
 *   - "delete"
 *   - "update"
 * @param $account
 *   A user object representing the user for whom the operation is to be
 *   performed.
 *
 * @return
 *   REDIRECT_ACCESS_ALLOW if the operation is to be allowed;
 *   REDIRECT_ACCESS_DENY if the operation is to be denied;
 *   REDIRECT_ACCESSS_IGNORE to not affect this operation at all.
 *
 * @see redirect_access()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_access($op, $redirect, $account) {
  $type = is_string($redirect) ? $redirect : $redirect->type;

  if (in_array($type, array('normal', 'special'))) {
    if ($op == 'create' && user_access('create ' . $type . ' redirects', $account)) {
      return REDIRECT_ACCESS_ALLOW;
    }

    if ($op == 'update') {
      if (user_access('edit any ' . $type . ' content', $account) || (user_access('edit own ' . $type . ' content', $account) && ($account->uid == $redirect->uid))) {
        return REDIRECT_ACCESS_ALLOW;
      }
    }

    if ($op == 'delete') {
      if (user_access('delete any ' . $type . ' content', $account) || (user_access('delete own ' . $type . ' content', $account) && ($account->uid == $redirect->uid))) {
        return REDIRECT_ACCESS_ALLOW;
      }
    }
  }

  // Returning nothing from this function would have the same effect.
  return REDIRECT_ACCESS_IGNORE;
}

/**
 * Act on a redirect object about to be shown on the add/edit form.
 *
 * This hook is invoked from redirect_object_prepare().
 *
 * @param $redirect
 *   The redirect that is about to be shown on the add/edit form.
 *
 * @ingroup redirect_api_hooks
 */
function hook_redirect_prepare($redirect) {

}

/**
 * Perform redirect validation before a redirect is created or updated.
 *
 * This hook is invoked from redirect_validate(), after a user has has finished
 * editing the redirect and is submitting it. It is invoked at the end of all
 * the standard validation steps.
 *
 * To indicate a validation error, use form_set_error().
 *
 * Note: Changes made to the $redirect object within your hook implementation
 * will have no effect. The preferred method to change a redirect's content is
 * to use hook_redirect_presave() instead. If it is really necessary to change
 * the redirect at the validate stage, you can use form_set_value().
 *
 * @param $redirect
 *   The redirect being validated.
 * @param $form
 *   The form being used to edit the redirect.
 * @param $form_state
 *   The form state array.
 *
 * @see redirect_validate()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_validate($redirect, $form, $form_state) {

}

/**
 * Act on a redirect being inserted or updated.
 *
 * This hook is invoked from redirect_save() before the redirect is saved to
 * the database.
 *
 * @param $redirect
 *   The redirect that is being inserted or updated.
 *
 * @see redirect_save()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_presave($redirect) {

}

/**
 * Respond to creation of a new redirect.
 *
 * This hook is invoked from redirect_save() after the redirect is inserted
 * into the redirect table in the database.
 *
 * @param $redirect
 *   The redirect that is being created.
 *
 * @see redirect_save()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_insert($redirect) {

}

/**
 * Respond to updates to a redirect.
 *
 * This hook is invoked from redirect_save() after the redirect is updated in
 * the redirect table in the database.
 *
 * @param $redirect
 *   The redirect that is being updated.
 *
 * @see redirect_save()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_update($redirect) {

}

/**
 * Respond to redirect deletion.
 *
 * This hook is invoked from redirect_delete_multiple() after the redirect has
 * been removed from the redirect table in the database.
 *
 * @param $redirect
 *   The redirect that is being deleted.
 *
 * @see redirect_delete_multiple()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_delete($redirect) {

}

/**
 * Act on a redirect being redirected.
 *
 * This hook is invoked from redirect_redirect() before the redirect callback
 * is invoked.
 *
 * @param $redirect
 *   The redirect that is being used for the redirect.
 *
 * @see redirect_redirect()
 * @see backdrop_page_is_cacheable()
 * @ingroup redirect_api_hooks
 */
function hook_redirect_alter($redirect) {
}

/**
 * @} End of "addtogroup hooks".
 */
