<?php
// $Id: views_plugin_argument_default_user.inc,v 1.1.6.1 2010/01/28 22:44:36 dereine Exp $
/**
 * @file
 * Contains the user from URL argument default plugin.
 */

/**
 * Default argument plugin to extract a user via menu_get_object
 */
class views_plugin_argument_default_user extends views_plugin_argument_default {
  function option_definition() {
    $options = parent::option_definition();
    $options['user'] = array('default' => '');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['user'] = array(
      '#type' => 'textfield',
      '#title' => t('Default argument'),
      '#default_value' => $this->options['user'],
    );
  }

  function convert_options(&$options) {
    if (!isset($options['user']) && isset($this->argument->options['default_argument_user'])) {
      $options['user'] = $this->argument->options['default_argument_user'];
    }
  }

  function get_argument() {
    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user_uid_optional', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    if (!empty($this->options['user'])) {
      foreach (range(1, 3) as $i) {
        $node = menu_get_object('node', $i);
        if (!empty($node)) {
          return $node->uid;
        }
      }
    }

    if (arg(0) == 'user' && is_numeric(arg(1))) {
      return arg(1);
    }

    if (arg(0) == 'node' && is_numeric(arg(1))) {
      $node = node_load(arg(1));
      if ($node) {
        return $node->uid;
      }
    }
  }
}

