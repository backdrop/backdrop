<?php
/**
 * @file
 * Output for a Dashboard panel.
 *
 * Available variables:
 *   - $panel: A renderable element containing the contents of the panel. This
 *     typically will hold the following children elements:
 *     - list: A list of tasks or administrative items.
 *     - table: A table of administrative tasks with manage links.
 *     - link: A link to an administrative page.
 */

$more_link = isset($panel['link']) ? backdrop_render($panel['link']) : '';
?>
<?php print backdrop_render_children($panel); ?>

<?php if ($more_link): ?>
  <div class="dashboard-more-link">
    <?php print $more_link; ?>
  </div>
<?php endif; ?>
