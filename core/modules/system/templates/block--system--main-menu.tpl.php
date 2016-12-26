<?php
/**
 * @file
 * Template for outputting the main menu block.
 *
 * Variables available:
 * - $classes: Array of classes that should be displayed on the block's wrapper.
 * - $title: The title of the block.
 * - $title_prefix/$title_suffix: A prefix and suffix for the title tag. This
 *   is important to print out as administrative links to edit this block are
 *   printed in these variables.
 * - $content: The actual content of the block.
 */
?>

<div class="<?php print implode(' ', $classes); ?>"<?php print drupal_attributes($attributes); ?>>
<?php print render($title_prefix); ?>
<?php if ($title): ?>
  <h2 class="block-title"><?php print $title; ?></h2>
<?php endif;?>
<?php print render($title_suffix); ?>
  <?php print render($title_suffix); ?>
  <div class="block-content">
    <?php if ($content['#menu_toggle']): ?>
      <input id="main-menu-state" type="checkbox" />
      <label class="main-menu-btn" for="main-menu-state">  <span class="main-menu-btn-icon"></span><?php print t('Menu') ?></label>
    <?php endif; ?>
    <?php print render($content); ?>
  </div>
</div>
