<?php 
/**
 * @file
 * 
 * Default theming implementation for displaying the install page
 * 
 * Available variables:
 * - $task_list: A list of tasks that are being performed, with the current task marked
 * - $main_content: The html that goes in the center area of the page
 * These are defined in project_browser_preprocess_project_browser_install()
 * 
 * @see project_browser_preprocess_project_browser_install()
 */
?>

<div class="project-browser-install-sidebar-left">
  <?php print $task_list; ?>
</div>
<div class="project-browser-install-main">
  <?php print $main_content; ?>
</div>
