<?php
/**
 * Common functions needed whenever displaying a layout.
 */

/**
 * Determine if a block is visible.
 *
 * @param $block
 *   The block object to test for access.
 * @param $layout
 *   The layout object containing the block object to be tested.
 */
function layout_block_access($block, Layout $layout) {
  return layout_check_access($block->access, $layout->getContexts());
}

/**
 * Determine if the current user has access via a plugin.
 *
 * This function is used to check access on multiple things, including access
 * to an individual block as well as the entire layout.
 *
 * @param $settings
 *   An array of settings set by the user.
 * @param $contexts
 *   An array of zero or more contexts that may be used to determine if
 *   the user has access.
 *
 * @return
 *   TRUE if access is granted, false if otherwise.
 */
function layout_check_access($settings, $contexts = array()) {
  if (empty($settings['plugins'])) {
    return TRUE;
  }

  if (!isset($settings['logic'])) {
    $settings['logic'] = 'and';
  }

  foreach ($settings['plugins'] as $test) {
    $access = layout_create_access($test['name'], $test['settings']);
    $test_contexts = $access->selectContexts($contexts);
    $pass = $access->checkAccess($test_contexts);

    if (!empty($test['not'])) {
      $pass = !$pass;
    }

    if ($pass && $settings['logic'] == 'or') {
      // Pass if 'or' and this rule passed.
      return TRUE;
    }
    else if (!$pass && $settings['logic'] == 'and') {
      // Fail if 'and' and this rule failed.
      return FALSE;
    }
  }

  // Return TRUE if logic was and, meaning all rules passed.
  // Return FALSE if logic was or, meaning no rule passed.
  return $settings['logic'] == 'and';
}
