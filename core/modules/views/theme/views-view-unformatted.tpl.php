<?php

/**
 * @file
 * Default simple view template to display a list of rows.
 *
 * @ingroup views_templates
 */
?>
<?php if (!empty($title)): ?>
  <h3><?php print $title; ?></h3>
<?php endif; ?>
<?php foreach ($rows as $id => $row): ?>
  <div<?php if ($classes[$id]) { print ' class="' . implode(' ', $classes[$id]) .'"';  } ?>>
    <?php print $row; ?>
  </div>
<?php endforeach; ?>
