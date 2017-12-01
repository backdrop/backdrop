<?php

/**
 * @file
 * Administration form for node settings for content translation.
 */

 /**
  * Builds the configuration form for node language configuration.
  */
function translation_node_configure_form() {

  $form = array(
    '#submit' => array('translation_node_configure_form_submit'),
  );

  $form['translation_show_translation_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show content translation links'),
    '#description' => t('Show the links to translations in content body and teasers. If you uncheck this option, switching language will only be available from the language switcher block.'),
    '#default_value' => config_get('translation.settings', 'translation_show_translation_links'),
    '#group' => 'translation_node_options',
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

 /**
  * Submit handler for the translation_node_configure_form.
  */
function translation_node_configure_form_submit(&$form, &$form_state) {
  config_set(
    'translation.settings',
    'translation_show_translation_links',
    $form_state['input']['translation_show_translation_links']
  );
  backdrop_set_message(t('Your configuration has been saved.'));
}
