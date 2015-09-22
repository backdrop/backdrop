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
    t('Operations'),
  );

  $rows = array();
  foreach ($custom_blocks_info as $delta => $block_info) {
    $row = array();
    $row[] = check_plain($block_info['info']);
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => 'admin/structure/block/manage/' . $delta . '/configure',
    );
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/block/manage/' . $delta . '/delete',
    );
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
 * @param $module
 *   Name of the module that implements the block to be configured.
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
    $custom_block = block_custom_block_load($delta);
  }
  else {
    $custom_block = array(
      'info' => '',
      'title' => '',
      'body' => '',
      'format' => NULL,
    );
  }
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta,
  );

  if ($custom_block['info']) {
    backdrop_set_title(t("'%name' block", array('%name' => $custom_block['info'])), PASS_THROUGH);
  }
  else {
    backdrop_set_title(t('Add new block'));
  }

  $form['settings']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#maxlength' => 255,
    '#description' => t('The title of the block as shown to the user.'),
    '#default_value' => isset($custom_block['title']) ? $custom_block['title'] : '',
    '#weight' => -18,
  );

  // Module-specific block configuration.
  if ($settings = module_invoke('block', 'block_configure', $delta)) {
    foreach ($settings as $k => $v) {
      $form['settings'][$k] = $v;
    }
  }

  // Prepare cancel link.
  if (isset($_GET['destination'])) {
    $path = urldecode($_GET['destination']);
  }
  elseif (isset($_SERVER['HTTP_REFERER'])) {
    $path = urldecode($_SERVER['HTTP_REFERER']);
  }
  elseif (user_access('administer blocks')) {
    $path = 'admin/structure/block';
  }
  else {
    $path = '<front>';
  }
  $options = backdrop_parse_url($path);
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
 * @see block_admin_configure()
 * @see block_admin_configure_submit()
 */
function block_admin_configure_validate($form, &$form_state) {
  $custom_block_exists = (bool) db_query_range('SELECT 1 FROM {block_custom} WHERE bid <> :bid AND info = :info', 0, 1, array(
    ':bid' => $form_state['values']['delta'],
    ':info' => $form_state['values']['info'],
  ))->fetchField();
  if (empty($form_state['values']['info']) || $custom_block_exists) {
    form_set_error('info', t('Ensure that each block description is unique.'));
  }
}

/**
 * Form submission handler for block_admin_configure().
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
 * @see block_add_block_form()
 * @see block_add_block_form_submit()
 */
function block_add_block_form_validate($form, &$form_state) {
  $custom_block_exists = (bool) db_query_range('SELECT 1 FROM {block_custom} WHERE info = :info', 0, 1, array(':info' => $form_state['values']['info']))->fetchField();

  if (empty($form_state['values']['info']) || $custom_block_exists) {
    form_set_error('info', t('Ensure that each block description is unique.'));
  }
}

/**
 * Form submission handler for block_add_block_form().
 *
 * Saves the new custom block.
 *
 * @see block_add_block_form()
 * @see block_add_block_form_validate()
 */
function block_add_block_form_submit($form, &$form_state) {
  $delta = block_custom_block_save($form_state['values']);
  // Store block delta to allow other modules to work with new block.
  $form_state['values']['delta'] = $delta;

  backdrop_set_message(t('The block has been created.'));
  $form_state['redirect'] = 'admin/structure/block';
}

/**
 * Form constructor for the custom block deletion form.
 *
 * @param $module
 *   The name of the module that implements the block to be deleted. This should
 *   always equal 'block' since it only allows custom blocks to be deleted.
 * @param $delta
 *   The unique ID of the block within the context of $module.
 *
 * @see block_menu()
 * @see block_custom_block_delete_submit()
 */
function block_custom_block_delete($form, &$form_state, $delta) {
  $custom_block = block_custom_block_load($delta);
  $form['info'] = array('#type' => 'hidden', '#value' => $custom_block['info']);
  $form['bid'] = array('#type' => 'hidden', '#value' => $delta);

  return confirm_form($form, t('Are you sure you want to delete the block %name?', array('%name' => $custom_block['info'])), 'admin/structure/block', '', t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for block_custom_block_delete().
 *
 * @see block_custom_block_delete()
 */
function block_custom_block_delete_submit($form, &$form_state) {
  db_delete('block_custom')
    ->condition('bid', $form_state['values']['bid'])
    ->execute();
  backdrop_set_message(t('The block %name has been removed.', array('%name' => $form_state['values']['info'])));
  $form_state['redirect'] = 'admin/structure/block';
}

