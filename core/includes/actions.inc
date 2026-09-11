<?php
/**
 * @file
 * This is the actions engine for executing stored actions.
 */

/**
 * @defgroup actions Actions
 * @{
 * Functions that perform an action on a certain system object.
 *
 * Action functions are declared by modules by implementing hook_action_info().
 * Modules can run action functions to run by calling action_execute().
 *
 * Each action function takes two arguments:
 * - $entity: The object that the action acts on, such as a node, comment, or
 *   user.
 * - $context: An array of information passed by reference that includes
 *   information about the current action execution, including the following
 *   keys:
 *   - action_name: The name of the action being executed.
 *   - action_info: The definition of the action being executed, as provided by
 *     hook_action_info().
 *   - redirect: A path to which the user should be redirected after the action
 *     is complete. This can be used to direct the user to a configuration form
 *     to configure the action or ask for confirmation.
 *
 * @} End of "defgroup actions".
 */

/**
 * Retrieves a single action's info by its name, or all actions
 *
 * @param string|null $action_name
 *   (optional) The name of the action to retrieve.
 *
 * @return array|false
 *   An array of action info as provided by hook_action_info(). If no action
 *   name is specified, all actions in the system will be returned as an array
 *   keyed by the action name. If an action name is specified but not found,
 *   FALSE will be returned.
 */
function actions_get_info($action_name = NULL) {
  $actions = &backdrop_static(__FUNCTION__);

  if (!isset($actions)) {
    foreach (module_implements('action_info') as $module) {
      $module_actions = module_invoke($module, 'action_info');
      foreach ($module_actions as $module_action_name => $action_info) {
        $action_info += array(
          'callback' => $module_action_name,
          // @todo In Backdrop 2.x, the default access callback should be
          // user_access() rather than TRUE, requiring always-available actions
          // to specify that they have no access check.
          'access callback' => TRUE,
          'access arguments' => array(),
          'module' => $module,
        );
        $actions[$module_action_name] = $action_info;
      }
    }
    backdrop_alter('action_info', $actions);
    backdrop_sort($actions);
  }

  if (!isset($action_name)) {
    return $actions;
  }
  elseif (isset($actions[$action_name])) {
    return $actions[$action_name];
  }
  else {
    return FALSE;
  }
}

/**
 * Checks if the current user account has access to a single action.
 *
 * @param string $action_name
 *   The name of the action to check access.
 * @param UserInterface|null $account
 *   The user account to check access action against. Defaults to the currently
 *   logged-in user.
 */
function actions_access($action_name, ?UserInterface $account = NULL) {
  global $user;
  if (!isset($account)) {
    $account = $user;
  }

  $action_info = actions_get_info($action_name);
  if (!$action_info) {
    trigger_error(t('The action "@action" could not be found.', array('@action' => $action_name)), E_USER_ERROR);
  }

  // Check if access is allowed to this action.
  $access_callback = $action_info['access callback'];
  $access_arguments = $action_info['access arguments'];
  $access_arguments[] = $account;
  $access = ($access_callback === TRUE);
  if (!$access) {
    $access = call_user_func_array($access_callback, $access_arguments);
  }

  return $access;
}

/**
 * Executes a single action.
 *
 * @param string $action_name
 *   The machine name of the action to be executed.
 * @param Entity|null $entity
 *   The entity object that the action will act upon, such as a node, user, or
 *   comment object.
 * @param array $context
 *   Information about the current action execution. See actions_get_info().
 *
 * @return mixed
 *   The result of the executed action callback.
 *
 * @ingroup actions
 */
function actions_execute($action_name, ?Entity $entity, &$context) {
  // $stack tracks the number of recursive calls.
  static $stack;
  $stack++;
  $recursion_limit = config_get('system.core', 'action_recursion_limit');
  if (empty($recursion_limit)) {
    $recursion_limit = 35;
  }
  if ($stack > $recursion_limit) {
    watchdog('actions', 'Stack overflow: recursion limit for actions_execute() has been reached. Stack is limited by %limit calls.', array('%limit' => $recursion_limit), WATCHDOG_ERROR);
    return NULL;
  }

  if (!actions_access($action_name)) {
    watchdog('actions', 'Action %name not executed because the user account does not have the required access.', array('%name' => $action_name), WATCHDOG_ERROR);
    return FALSE;
  }

  // Get action info and include the file containing the callbacks.
  $action_info = actions_get_info($action_name);
  $callback = $action_info['callback'];
  if (isset($action_info['file'])) {
    include_once BACKDROP_ROOT . '/' . backdrop_get_path('module', $action_info['module']) . '/' . $action_info['file'];
  }
  $context['action_name'] = $action_name;
  $context['action_info'] = $action_info;
  if (function_exists($callback)) {
    $result = $callback($entity, $context);
  }
  else {
    $result = NULL;
    trigger_error(t('The action "@action" could not be executed because the function "@callback" could not be found.', array(
      '@action' => $action_name,
      '@callback' => $callback,
    )), E_USER_ERROR);
  }

  $stack--;
  return $result;
}

/**
 * Redirect the user after an action has been performed.
 *
 * This function should be called after action_execute() if the
 * $context['redirect'] property has been populated.
 *
 * @param mixed $redirect
 *   Either an array or path string to which the user should be redirected.
 */
function actions_redirect($redirect) {
  if (!is_array($redirect)) {
    $redirect = array($redirect, array());
  }

  // Remove any existing destinations and tack it to the return URL if needed.
  if (isset($_GET['destination'])) {
    $redirect[1]['query']['destination'] = $_GET['destination'];
    unset($_GET['destination']);
  }
  else {
    $redirect[1]['query']['destination'] = $_GET['q'];
  }

  // See backdrop_redirect_form().
  call_user_func_array('backdrop_goto', $redirect);
}
