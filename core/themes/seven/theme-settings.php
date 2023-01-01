<?php
/**
 * @file
 * Theme settings file for Seven.
 */
$form['admin_pages'] = array(
  '#type' => 'fieldset',
  '#title' => t('Admin pages'),
);
$form['admin_pages']['respect_position'] = array(
  '#type' => 'checkbox',
  '#title' => t('Respect position of blocks on admin pages.'),
  '#description' => t('Some modules apply "left" or "right" position to their menu items, which is intended to set which column the item appears in on admin pages. The default behavior of Seven theme is to do its own positioning of the blocks. If this box is checked, Seven will respect the positions as set by the modules.'),
  '#default_value' => theme_get_setting('respect_position'),
);
