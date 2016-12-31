<?php
/**
 * @file
 * Action callbacks for File module.
 */

/**
 * Deletes a specific file.
 *
 * @param File $file
 *   The user entity whose account will be blocked.
 *
 * @ingroup actions
 */
function file_delete_file_action(File $file, &$context) {
  if (!user_access('delete files')) {
    return;
  }

  $file->delete();
}
