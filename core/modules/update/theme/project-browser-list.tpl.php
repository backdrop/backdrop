<?php
/**
 * @file
 * 
 * Default theming implementation for displaying list of projects.
 * 
 * Available variables:
 * - $main_content: The main content area, namely the projects list, including the pager
 * - $filters: The filters block
 * - $install_list: The install queue block
 * These are defined in project_browser_preprocess_project_browser_list()
 * 
 * @see project_browser_preprocess_project_browser_list()
 */
?>
<div id="project-browser" class="clearfix">
  <div id="project-browser-main" class="project-browser-region">
    <?php print $main_content; ?>
  </div>

  <div id="project-browser-sidebar-right" class="project-browser-region">
    <?php print $filters; ?>
    <?php print $install_list; ?>
  </div>
  
</div>
