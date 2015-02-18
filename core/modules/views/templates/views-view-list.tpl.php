<?php
/**
 * @file
 * Default simple view template to display a list of rows.
 *
 * - $title: The title of this group of rows.  May be empty.
 * - $rows: @todo document.
 * - $row_classes: @todo document.
 * - $list_type_prefix: @todo document.
 * - $list_type_suffix: @todo document.
 * - $wrapper_prefix: @todo document.
 * - $wrapper_suffix: @todo document.
 *
 * Additional variables:
 * - $options['type'] will either be ul or ol.
 *
 * @ingroup views_templates
 */
?>
<?php print $wrapper_prefix; ?>
  <?php if (!empty($title)) : ?>
    <h3><?php print $title; ?></h3>
  <?php endif; ?>
  <?php print $list_type_prefix; ?>
    <?php foreach ($rows as $id => $row): ?>
      <li class="<?php print implode(' ', $row_classes[$id]); ?>"><?php print $row; ?></li>
    <?php endforeach; ?>
  <?php print $list_type_suffix; ?>
<?php print $wrapper_suffix; ?>
