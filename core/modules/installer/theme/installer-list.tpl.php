<?php
/**
 * @file
 * 
 * Default theming implementation for displaying list of projects.
 * 
 * Available variables:
 * - $main_content: The main content area, namely the projects list, including the pager.
 * - $filters: The filters block.
 * - $install_list: The install queue block.
 * These are defined in installer_browser_preprocess_installer_browser_list().
 * 
 * @see installer_browser_preprocess_installer_browser_list().
 */
?>
<div class="installer-browser clearfix">
    <?php print $filters; ?>
  <div class="installer-browser-main installer-browser-region">
    <?php print $main_content; ?>
  </div>

  <div class="installer-browser-sidebar-right installer-browser-region">
    <?php print $install_list; ?>
    <?php print $advanced; ?>
  </div>
  
</div>
