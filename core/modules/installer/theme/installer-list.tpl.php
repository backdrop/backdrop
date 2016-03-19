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
 * These are defined in installer_browser_preprocess_installer_browser_list()
 * 
 * @see installer_browser_preprocess_installer_browser_list()
 */
?>
<div id="installer-browser" class="clearfix">
  <div id="installer-browser-main" class="installer-browser-region">
    <?php print $main_content; ?>
  </div>

  <div id="installer-browser-sidebar-right" class="installer-browser-region">
    <?php print $filters; ?>
    <?php print $install_list; ?>
    <?php print $advanced; ?>
  </div>
  
</div>
