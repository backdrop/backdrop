<?php
/**
 * @file
 * Builds a list of pseudo-tokens for use in the "Log entry format" field.
 *
 * @see syslog_form_system_logging_settings_alter()
 * @see system_logging_settings()
 */

/**
 * Implements hook_token_info().
 *
 * Note: no need to implement hook_tokens() - the tokens provided here are not
 * actual tokens, so they don't need to be replaced on save.
 */
function syslog_token_info() {
  $info['types']['syslog'] = array(
    'name' => t('Syslog message variables'),
    'description' => t('Tokens used by the Syslog module when generating log messages.'),
    'needs-data' => 'syslog',
  );

  $info['tokens']['syslog'] = array(
    '!base_url' => array(
      'name' => t('Base URL'),
      'description' => t('Base URL of the site.'),
    ),
    '!timestamp' => array(
      'name' => t('Timestamp'),
      'description' => t('Unix timestamp of the log entry.'),
    ),
    '!type' => array(
      'name' => t('Message type'),
      'description' => t('The category to which this message belongs.'),
    ),
    '!ip' => array(
      'name' => t('User IP address'),
      'description' => t('The IP address of the user triggering the message.'),
    ),
    '!uid' => array(
      'name' => t('User ID'),
      'description' => t('The ID of the user triggering the message.'),
    ),
    '!request_uri' => array(
      'name' => t('Requested URI'),
      'description' => t('The Requested URI.'),
    ),
    '!referer' => array(
      'name' => t('HTTP referer'),
      'description' => t('The HTTP referer, if available.'),
    ),
    '!link' => array(
      'name' => t('Link'),
      'description' => t('A link to associate with the message.'),
    ),
    '!message' => array(
      'name' => t('Message'),
      'description' => t('The message to store in the log.'),
    ),
  );

  // Add a flag to mark all these tokens as "pseudo tokens". This makes it so
  // that they are not wrapped in "[]" and there's no "parent:" prefix added to
  // the token name either.
  foreach ($info['tokens']['syslog'] as $token => $token_info) {
    $info['tokens']['syslog'][$token]['pseudo_token'] = TRUE;
  }

  return $info;
}
