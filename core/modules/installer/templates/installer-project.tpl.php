<?php
/**
 * @file
 *
 * Default theme implementation for displaying a project.
 *
 * Available variables:
 * - $first: Whether or not this project item is the first one in the list.
 * - $title: The title of the project.
 * - $last_updated: A string representing when the project was last updated.
 * - $description: The project description. This could be long.
 * - $extras: A string of any extra info that should go at the bottom.
 * - $status: Whether the project is already installed, enabled, or not.
 * - $install: The add to install queue button/link.
 * These are defined in template_preprocess_installer_browser_project().
 *
 * @see template_preprocess_installer_browser_project().
 */

$classes = array('project-item');
if ($image) {
  $classes[] = 'has-image';
}

?>
<div class="<?php print implode(' ' , $classes); ?>">
  <?php if ($image): ?>
    <div class="screenshot">
      <?php print $image; ?>
    </div>
  <?php endif; ?>

  <div class="project-information" >
    <div class="project-title">
      <?php print $title; ?>
    </div>
    <div class="project-description description">
      <?php print $description; ?>
    </div>
  </div>

  <div class="project-extras">
    <div class="project-extra">
      <?php print $extras; ?>
    </div>
    <div class="project-status">
        <?php print $status; ?>
        <?php print $install; ?>
    </div>
  </div>
</div>
