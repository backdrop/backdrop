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
  <?php if(!empty($add) || !empty($rearrange) || !empty($delete)) : ?>
    <ul class="horizontal right actions links drop-list">
      <?php if ($add) : ?>
      <li>
        <?php print $add; ?>
      </li>
      <?php endif; ?>
      <?php if ($rearrange) : ?>
      <li>
        <?php print $rearrange; ?>
      </li>
      <?php endif; ?>
      <?php if ($delete) : ?>
      <li>
        <?php print $delete; ?>
      </li>
      <?php endif; ?>
    </ul>
  <?php endif; ?>
  <h3><?php print $title; ?></h3>
  <?php print $content; ?>
</div>
