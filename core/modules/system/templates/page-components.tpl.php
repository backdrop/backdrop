<?php
/**
 * @file
 * Template for outputting title component blocks.
 *
 * Combo title block displays the page title, local task tabs, 
 * local actions links, and messages.
 *
 * To override the display of page components, copy this file and place it
 * inside your theme. 
 * To override individual individual components, create one of the following
 * templates in your theme:
 * - page-components.tpl.php
 * - page-components--action-links.tpl.php
 * - page-components--messages.tpl.php
 * - page-components--tabs.tpl.php
 * - page-components--title.tpl.php
 * - page-components--title-combo.tpl.php
 *
 * Variables available:
 * Variables:
 * - $title: The page title, for use in the actual HTML content.
 * - $messages: Status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links: Array of actions local to the page, such as 'Add menu' on
 *   the menu administration interface.
 */
?>
<?php if ($messages): ?>
  <div class="l-messages" role="status" aria-label="<?php print t('Status messages'); ?>">
    <?php print $messages; ?>
  </div>
<?php endif; ?>

<?php if ($title): ?>
  <?php print render($title_prefix); ?>
    <h1 class="title">
      <?php print $title; ?>
    </h1>
  <?php print render($title_suffix); ?>
<?php endif; ?>

<?php if ($tabs): ?>
  <div class="tabs">
    <?php print $tabs; ?>
  </div>
<?php endif; ?>

<?php if ($action_links): ?>
  <?php print $action_links; ?>
<?php endif; ?>
