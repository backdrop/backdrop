<?php
// $Id$
/**
 * @file views-ui-display-tab-bucket.tpl.php
 * Template for each "box" on the display query edit screen.
 */
?>
<div class="<?php print $classes; ?>">
  <?php if ($overridden): ?>
    <div class="icon-linked"></div>
  <?php endif; ?>
  <ul class="horizontal right actions links">
    <?php if ($rearrange) : ?>
    <li>
      <?php print $rearrange; ?>
    </li>
    <?php endif; ?>
    <?php if ($add) : ?>
    <li>
      <?php print $add; ?>
    </li>
    <?php endif; ?>
  </ul>
  <h3><?php print $title; ?></h3>
  <?php print $content; ?>
</div>
