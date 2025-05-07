<?php
/**
 * @file
 * Default view template to display a rows in a grid.
 *
 * - $title: The title of this group of rows.  May be empty.
 * - $classes: An array of classes to apply to the grid, based on settings.
 * - $attributes: An array of additional HTML attributes for the grid.
 * - $item: An array of grid items.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 *
 * @ingroup views_templates
 */
?>
<?php if (!empty($title)) : ?>
  <h3><?php print $title; ?></h3>
<?php endif; ?>

<div class="<?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <?php foreach ($items as $id => $item): ?>
    <div <?php if (!empty($row_classes[$id])) { print 'class="' . implode(' ', $row_classes[$id]) . '"';  } ?>>
      <div class="views-responsive-grid-box-inner">
        <?php print $item; ?>
      </div>
    </div>
  <?php endforeach; ?>
</div>
