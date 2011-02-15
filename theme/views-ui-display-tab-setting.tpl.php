<?php
// $Id$
/**
 * @file views-ui-display-tab-setting.tpl.php
 * Template for each row inside the "boxes" on the display query edit screen.
 */
?>
<div class="views-display-setting clearfix <?php print $zebra; ?>">
  <?php if ($description): ?>
    <span class="label"><?php print ($description . t(':')); ?></span>
  <?php endif; ?>
  <?php print $link; ?>
  <?php if ($settings): ?>
    | <?php print $settings; ?>
  <?php endif; ?>
</div>
