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
 * Form submission handler for block_admin_display_form().
 *
 * @see block_admin_display_form()
 */
function block_admin_display_form_submit($form, &$form_state) {
  $transaction = db_transaction();
  try {
    foreach ($form_state['values']['blocks'] as $block) {
      $block['status'] = (int) ($block['region'] != BLOCK_REGION_NONE);
      $block['region'] = $block['status'] ? $block['region'] : '';
      db_update('block')
        ->fields(array(
          'status' => $block['status'],
          'weight' => $block['weight'],
          'region' => $block['region'],
        ))
        ->condition('module', $block['module'])
        ->condition('delta', $block['delta'])
        ->condition('theme', $block['theme'])
        ->execute();
    }
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('block', $e);
    throw $e;
  }
  backdrop_set_message(t('The block settings have been updated.'));
  cache_clear_all();
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
    '#title' => t('Block title'),
    '#maxlength' => 64,
    '#description' => t('The title of the block as shown to the user.'),
    '#default_value' => isset($custom_block['title']) ? $custom_block['title'] : '',
    '#weight' => -19,
  );

  // Module-specific block configuration.
  if ($settings = module_invoke('block', 'block_configure', $delta)) {
    foreach ($settings as $k => $v) {
      $form['settings'][$k] = $v;
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save block'),
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
  cache_clear_all();
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
  cache_clear_all();
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
  cache_clear_all();
  $form_state['redirect'] = 'admin/structure/block';
}

/**
 * Processes variables for block-admin-display-form.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $form
 *
 * @see block-admin-display.tpl.php
 * @see theme_block_admin_display()
 */
function template_preprocess_block_admin_display_form(&$variables) {
  $variables['block_regions'] = $variables['form']['block_regions']['#value'];
  if (isset($variables['block_regions'][BLOCK_REGION_NONE])) {
    $variables['block_regions'][BLOCK_REGION_NONE] = t('Disabled');
  }

  foreach ($variables['block_regions'] as $key => $value) {
    // Initialize an empty array for the region.
    $variables['block_listing'][$key] = array();
  }

  // Initialize disabled blocks array.
  $variables['block_listing'][BLOCK_REGION_NONE] = array();

  // Add each block in the form to the appropriate place in the block listing.
  foreach (element_children($variables['form']['blocks']) as $i) {
    $block = &$variables['form']['blocks'][$i];

    // Fetch the region for the current block.
    $region = (isset($block['region']['#default_value']) ? $block['region']['#default_value'] : BLOCK_REGION_NONE);

    // Set special classes needed for table drag and drop.
    $block['region']['#attributes']['class'] = array('block-region-select', 'block-region-' . $region);
    $block['weight']['#attributes']['class'] = array('block-weight', 'block-weight-' . $region);

    $variables['block_listing'][$region][$i] = new stdClass();
    $variables['block_listing'][$region][$i]->row_class = !empty($block['#attributes']['class']) ? implode(' ', $block['#attributes']['class']) : '';
    $variables['block_listing'][$region][$i]->block_modified = !empty($block['#attributes']['class']) && in_array('block-modified', $block['#attributes']['class']);
    $variables['block_listing'][$region][$i]->block_title = backdrop_render($block['info']);
    $variables['block_listing'][$region][$i]->region_select = backdrop_render($block['region']) . backdrop_render($block['theme']);
    $variables['block_listing'][$region][$i]->weight_select = backdrop_render($block['weight']);
    $variables['block_listing'][$region][$i]->operations = backdrop_render($block['operations']);
    $variables['block_listing'][$region][$i]->printed = FALSE;
  }

  $variables['form_submit'] = backdrop_render_children($variables['form']);
}

