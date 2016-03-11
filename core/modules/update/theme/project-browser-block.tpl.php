<?php
/**
 * @file
 * Default theme implementation to display a block on the projects browser page
 *
 * Available variables:
 * - $title: The title of the block
 * - $content: The content of the block
 * These are defined in project_browser_preprocess_project_browser_block()
 * 
 * @see project_browser_preprocess_project_browser_block()
 */
?>
<div class="<?php print implode(' ', $classes); ?>">
  <?php print render($title_prefix); ?>
  <h2><?php print $title; ?></h2>
  <?php print render($title_suffix); ?>
  <div class="content">
    <?php print $content; ?>
  </div>
</div>
