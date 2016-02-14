<?php
/**
 * @file
 * Template for a layout region.
 *
 * Variables:
 * - $blocks: Rendered layout blocks for this region.
 * - $classes: Array of classes to be added to the layout wrapper.
 * - $attributes: Additional attributes to be added to the layout wrapper.
 */
?>
<div class="region <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <?php print $blocks; ?>
</div>
