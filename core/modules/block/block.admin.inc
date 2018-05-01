<?php

/**
 * @file
 * Admin page callbacks for the block module.
 */

/**
 * Page callback; Display a list of all custom blocks.
 */
function block_admin_list() {
  $custom_blocks_info = block_block_info();

  $header = array(
    t('Block'),
    array('data' => t('Description'), 'class' => array('priority-low')),
    t('Operations'),
  );

  $rows = array();
  foreach ($custom_blocks_info as $delta => $block_info) {
    $row = array();
    $row[] = check_plain($block_info['info']);
    $row[] = check_plain($block_info['description']);
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/block/manage/' . $delta . '/configure',
    );
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/block/manage/' . $delta . '/delete',
    );
    if (module_exists('locale')) {
      $links['translate'] = array(
        'title' => t('Translate'),
        'href' => 'admin/structure/block/manage/' . $delta . '/translation',
      );
    }
    $operations = array(
      '#type' => 'operations',
      '#links' => $links,
    );
    $row[] = array('data' => $operations);
    $rows[] = $row;
  }

  return array(
    '#theme' => 'table__block_admin_list',
    '#rows' => $rows,
    '#header' => $header,
    '#empty' => t('No custom blocks have been created yet.'),
  );
}

/**
 * Form constructor for the block configuration form.
 *
 * Also used by block_add_block_form() for adding a new custom block.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param $delta
 *   Unique ID of the block within the context of $module.
 *
 * @see block_menu()
 * @see block_admin_configure_validate()
 * @see block_admin_configure_submit()
 * @ingroup forms
 */
function block_admin_configure($form, &$form_state, $delta = NULL) {
  if ($delta) {
    $langcode = NULL;
    if (module_exists('locale') && !isset($form['langcode'])) {
      // Get the current Language and try to load the block in that Language.
      global $language;
      $langcode = $language->langcode;
    }
    elseif (isset($form['langcode'])) {
      $langcode = $form['langcode']['#value'];
    }
    $custom_block = block_custom_block_load($delta, $langcode);
  }
  else {
    $custom_block = array(
      'info' => '',
      'title' => '',
      'body' => array('value' => '', 'format' => NULL),
    );
    if (module_exists('locale')) {
      $custom_block['default_langcode'] = LANGUAGE_NONE;
    }
  }
  if ($custom_block['info']) {
    backdrop_set_title(t("'%name' block", array('%name' => $custom_block['info'])), PASS_THROUGH);
  }
  else {
    backdrop_set_title(t('Add custom block'));
  }

  // Set title for translate block form.
  if (isset($form['langcode'])) {
    $language = $language = language_load($form['langcode']['#value']);
    backdrop_set_title(t("Translate '%name' block to %language", array('%name' => $custom_block['info'], '%language' => $language->name)), PASS_THROUGH);
  }

  // Call our own block configuration form (also used by Layout module).
  $form += block_custom_block_form($custom_block, TRUE);

  // Prepare cancel link.
  if (isset($_GET['destination'])) {
    $path = $_GET['destination'];
  }
  elseif (isset($_SERVER['HTTP_REFERER'])) {
    $path = $_SERVER['HTTP_REFERER'];
  }
  elseif (user_access('administer blocks')) {
    $path = 'admin/structure/block';
  }
  else {
    $path = '<front>';
  }
  $options = backdrop_parse_url($path);
  // Check if we have a clean path without language prefix.
  if (module_exists('locale')) {
    list(, $options['path']) = language_url_split_prefix($options['path'], language_list());
  }
  $options['attributes']['class'][] = 'form-cancel';
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save block'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $options['path'],
    '#options' => $options,
    '#weight' => 1,
  );

  return $form;
}

/**
 * Form validation handler for block_admin_configure().
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_admin_configure()
 * @see block_admin_configure_submit()
 */
function block_admin_configure_validate($form, &$form_state) {
  $blocks = block_block_info();
  foreach ($blocks as $delta => $block) {
    if ($delta != $form_state['values']['delta'] && $block['info'] == $form_state['values']['info']) {
      form_set_error('info', t('Ensure that each block description is unique.'));
      return;
    }
  }
}

/**
 * Form submission handler for block_admin_configure().
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_admin_configure()
 * @see block_admin_configure_validate()
 */
function block_admin_configure_submit($form, &$form_state) {
  block_custom_block_save($form_state['values'], $form_state['values']['delta']);
  backdrop_set_message(t('The block configuration has been saved.'));
  $form_state['redirect'] = 'admin/structure/block';
}

/**
 * Form constructor for the add block form.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_menu()
 * @see block_add_block_form_validate()
 * @see block_add_block_form_submit()
 * @ingroup forms
 */
function block_add_block_form($form, &$form_state) {
  return block_admin_configure($form, $form_state);
}

/**
 * Form validation handler for block_add_block_form().
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_add_block_form()
 * @see block_add_block_form_submit()
 */
function block_add_block_form_validate($form, &$form_state) {
  return block_admin_configure_validate($form, $form_state);
}

/**
 * Form submission handler for block_add_block_form().
 *
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * Saves the new custom block.
 *
 * @see block_add_block_form()
 * @see block_add_block_form_validate()
 */
function block_add_block_form_submit($form, &$form_state) {
  block_custom_block_save($form_state['values'], $form_state['values']['delta']);
  backdrop_set_message(t('The block has been created.'));
  $form_state['redirect'] = 'admin/structure/block';
}

/**
 * Form constructor for the custom block deletion form.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param $delta
 *   The unique ID of the block within the context of $module.
 *
 * @see block_menu()
 * @see block_custom_block_delete_submit()
 */
function block_custom_block_delete($form, &$form_state, $delta) {
  $custom_block = block_custom_block_load($delta);
  $form['info'] = array('#type' => 'hidden', '#value' => $custom_block['info']);
  $form['delta'] = array('#type' => 'hidden', '#value' => $delta);

  return confirm_form($form, t('Are you sure you want to delete the block %name?', array('%name' => $custom_block['info'])), 'admin/structure/block', '', t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for block_custom_block_delete().
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_custom_block_delete()
 */
function block_custom_block_delete_submit($form, &$form_state) {
  config('block.custom.' . $form_state['values']['delta'])->delete();
  backdrop_set_message(t('The block %name has been removed.', array('%name' => $form_state['values']['info'])));
  $form_state['redirect'] = 'admin/structure/block';
}


/**
 * Form constructor for the block translation list form.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param $delta
 *   Unique ID of the block within the context of $module.
 * @ingroup forms
 */
function block_custom_block_translate_list($form, &$form_state, $delta) {
  $custom_block = block_custom_block_load($delta, 'LANGUAGE_NONE');

  $language_list = language_list();

  $header = array(
    t('Language'),
    t('Title'),
    t('Status'),
    t('Operations'),
  );

  $rows = array();

  if (isset($custom_block['default_langcode']) && ($custom_block['default_langcode'] == LANGUAGE_NONE)) {
    $row[] = t('Language neutral (source)');
    $row[] = isset($custom_block['title']) ? $custom_block['title'] : t('N/A');
    $row[] = t('Source');
    $links = array();
    $links['edit'] = array(
      'title' => t('Edit'),
      'href' => 'admin/structure/block/manage/' . $delta . '/configure',
    );
    $operations = array(
      '#type' => 'operations',
      '#links' => $links,
    );
    $row[] = array('data' => $operations);
    $rows[] = $row;
  }

  foreach ($language_list as $langcode => $item) {
    $row = array();
    $links = array();

    // Check if this is the source language.
    if (isset($custom_block['default_langcode']) && ($langcode == $custom_block['default_langcode'])) {
      $row[] = t($item->name) . ' ' . t('(source)');
      $row[] = isset($custom_block['title']) ? $custom_block['title'] : t('N/A');
      $row[] = t('Source');
      $links['edit'] = array(
        'title' => t('EDIT'),
        'href' => 'admin/structure/block/manage/' . $delta . '/translate/' . $langcode,
      );
    }
    else {
      $row[] = t($item->name);
      $row[] = isset($custom_block['translations'][$langcode]['title']) ? $custom_block['translations'][$langcode]['title'] : t('N/A');
      $row[] = isset($custom_block['translations'][$langcode]) ? t('Translated') : t('Not Translated');
      $links['edit'] = array(
        'title' => isset($custom_block['translations'][$langcode]) ? t('EDIT') : t('ADD TRANSLATION'),
        'href' => 'admin/structure/block/manage/' . $delta . '/translate/' . $langcode,
      );
    }

    $operations = array(
      '#type' => 'operations',
      '#links' => $links,
    );
    $row[] = array('data' => $operations);
    $rows[] = $row;
  }

  if ($custom_block['info']) {
    backdrop_set_title(t("Translate '%name' block", array('%name' => $custom_block['info'])), PASS_THROUGH);
  }

  return array(
    '#theme' => 'table__block_admin_list',
    '#rows' => $rows,
    '#header' => $header,
    '#empty' => t('No languages found to translate to.'),
  );
}


/**
 * Menu callback function for translating a single block.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 * @param  string $delta
 *   The unique machine name of that block.
 * @param  string $langcode
 *   The language code value to translate to.
 *
 * @return array
 *   The configuration form.
 */
function block_custom_block_translate($form, &$form_state, $delta, $langcode) {
  $form['langcode'] = array('#type' => 'hidden', '#value' => $langcode);

  return block_admin_configure($form, $form_state, $delta);
}

/**
 * Form validation handler for block_custom_block_translate().
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_custom_block_translate()
 * @see block_custom_block_translate_submit()
 */
function block_custom_block_translate_validate($form, &$form_state) {
  return block_admin_configure_validate($form, $form_state);
}

/**
 * Form submission handler for block_custom_block_translate().
 *
 * Saves the new custom block.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @see block_custom_block_translate()
 * @see block_custom_block_translate_validate()
 */
function block_custom_block_translate_submit($form, &$form_state) {
  $langcode = NULL;
  $delta = $form_state['values']['delta'];
  if (isset($form_state['values']['langcode'])) {
    $langcode = $form_state['values']['langcode'];
  }
  block_custom_block_save($form_state['values'], $form_state['values']['delta'], $langcode);
  backdrop_set_message(t('The block translation has been saved.'));
  $form_state['redirect'] = 'admin/structure/block/manage/' . $delta . '/translation';
}
