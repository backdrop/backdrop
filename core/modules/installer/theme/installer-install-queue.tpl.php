<?php 
/**
 * @file
 * 
 * Default theming implementation for displaying list of queued projects
 * 
 * Available variables:
 * - $queue_html: The html for the install_queue 
 * These are defined in project_browser_preprocess_project_browser_install_queue()
 * 
 * @see project_browser_preprocess_project_browser_install_queue()
 */
?>
<div id="project-browser-install-queue" class="clearfix">
  <?php print $queue_html; ?>
</div>
