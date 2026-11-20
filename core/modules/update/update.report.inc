<?php
/**
 * @file
 * Code required only when rendering the available updates report.
 */

/**
 * Page callback: Generates a page about the update status of projects.
 *
 * @see update_menu()
 */
function update_status() {
  // With many modules installed, this page would load too slow, if we fetch
  // info from the update server on page load.
  // Note that if cron runs regularly and there is no problem fetching data, the
  // info will still be recent. Note also that the page displays, when this info
  // has been fetched, and a button to refresh manually in batches.
  $refresh = TRUE;
  $module_count = db_query("SELECT COUNT(*) FROM {system} WHERE filename LIKE 'modules/%' AND status = 1")->fetchField();
  // @todo "30" is an arbitrary number.
  if ($module_count > 30) {
    $refresh = FALSE;
  }
  if ($available = update_get_available($refresh)) {
    module_load_include('inc', 'update', 'update.compare');
    $data = update_calculate_project_data($available);
    return theme('update_report', array('data' => $data));
  }
  else {
    return theme('update_report', array('data' => _update_no_data()));
  }
}

