<?php
/**
 * @file
 * Password callback file for the user module.
 */

/**
 * Get the password strength score required for an account.
 *
 * @param object $username
 *   Backdrop username.
 *
 * @return bool
 *   True or false.
 */
function user_password_reject_weak($username) {
  // We use username instead of uid because this also runs for anonymous
  // users who are creating an account.
  $account = user_load_by_name($username);
  $reject_weak = config_get('system.core', 'user_password_reject_weak');

  // Allow other modules to alter this value (e.g. based on role).
  backdrop_alter('user_password_reject_weak', $reject_weak, $account);

  return $reject_weak;
}

/**
 * Generates help text for core password constraints on forms.
 *
 * @see user_account_form()
 */
function _user_password_policy_help() {
  $help = array();
  $help[] = t('The password cannot be the same as the username or e-mail address.');
  $help[] = t('The password must be sufficiently long or complex.');
  $variables = array(
    'items' => $help,
    'title' => '',
    'type' => 'ul',
    'attributes' => array(),
  );
  return theme_item_list($variables);
}

/**
 * Evaluate password strength
 * Adapted from https://github.com/dropbox/zxcvbn.
 *
 * @see Backdrop.evaluatePasswordStrength
 *
 * @param string $password
 * @param string $username
 * @param string $email
 *
 * @return int
 *   An integer representing password strength.
 */
function _user_password_evaluate_strength($password, $username, $email) {
  $strength = 0;

  $has_lowercase = preg_match('/[a-z]+/', $password);
  $has_uppercase = preg_match('/[A-Z]+/', $password);
  $has_numbers = preg_match('/[0-9]+/', $password);
  $has_punctuation = preg_match('/[^a-zA-Z0-9]+/', $password);

  // Calculate the number of unique character sets within a string.
  $cardinality = ($has_lowercase * 26) + ($has_uppercase * 26) + ($has_numbers * 10) + ($has_punctuation * 33);

  // Assign strength based on the level of entropy within the password, times
  // its length.
  $length = backdrop_strlen($password);
  $strength = (log($cardinality) / log(2)) * $length + 1;

  if (backdrop_strtolower($password) == backdrop_strtolower($username)) {
    $strength = 5;
  }
  if (backdrop_strtolower($password) == backdrop_strtolower($email)) {
    $strength = 5;
  }

  return $strength;
}
