<?php
// $Id$
/**
 * @file views-ui-display-tab-bucket.tpl.php
 * Template for each "box" on the display query edit screen.
 */
?>
<div class="<?php print $classes; ?>">
  <?php print $item_help_icon; ?>
  <?php if(!empty($actions)) : ?>
    <?php print $actions; ?>
  <?php endif; ?>
  <h3><?php print $title; ?></h3>
  <?php print $content; ?>
</div>
