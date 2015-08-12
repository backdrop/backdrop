<?php
/**
 * @file
 * Default view template to display a rows in a grid.
 *
 * - $title: The title of this group of rows.  May be empty.
 * - $classes: An array of classes to apply to the grid, based on settings.
 * - $attributes: An array of additional HTML attributes for the grid.
 * - $caption: The caption for this grid. May be empty.
 * - $rows: A nested array of rows. Each row contains an array of columns.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 * - $column_classes: An array of classes to apply to each column, indexed by
 *   row number, then column number. This matches the index in $rows.
 *
 * @ingroup views_templates
 */
?>
<?php dpm($classes); if (!empty($title)) : ?>
  <h3><?php print $title . "IKNKN"; ?></h3>
<?php endif; ?>
  <div class="<?php print $classes; ?>">
<?php if ($options['alignment'] == 'vertical') : ?>
<?php foreach ($columns as $column_id => $column) : ?>
  <div class="testerclass <?php print trim($column_classes[$column_id]); ?>">
  <?php foreach ($column as $item_id => $item) : ?>
    <div class="<?php print trim($item['classes']); ?>">
      <?php print $item['content']; ?>
    </div>
  <?php endforeach; ?>
  </div>
<?php endforeach; ?>
<?php else : ?>
  <?php foreach ($rows as $row_id => $row) : ?>
    <div class="<?php print trim($row_classes[$row_id]); ?>">
  <?php foreach ($row as $item_id => $item) : ?>
    <div class="<?php print trim($item['classes']); ?>">
      <?php print $item['content']; ?>
    </div>
  <?php endforeach; ?>
  </div>
  <?php endforeach; ?>
<?php endif; ?>
  </div>
