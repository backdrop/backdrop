<?php
/**
 * @file
 * Template for a single column layout.
 *
 * Variables:
 * - $title: The page title, for use in the actual HTML content.
 * - $messages: Status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links: Array of actions local to the page, such as 'Add menu' on
 *   the menu administration interface.
 * - $classes: Array of classes to be added to the layout wrapper.
 * - $attributes: Additional attributes to be added to the layout wrapper.
 * - $content: An array of content, each item in the array is nameed to one
 *   region of the layout. This layout supports the following sections:
 *   - $content['header']
 *   - $content['top']
 *   - $content['content']
 *   - $content['footer']
 */
?>
<div class="layout--flexible layout <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>
  <!-- hardcoded here to dev stress free -->
  <?php if ($messages): ?>
    <div class="l-messages" role="status" aria-label="<?php print t('Status messages'); ?>">
      <?php print $messages; ?>
    </div>
  <?php endif; ?>

  <?php foreach ($region_data as $name => $region): ?>
    <<?php print $region['element']; ?> class="l-wrapper <?php print 'l-' . $name; ?>" <?php print $region['row_id']; ?>>
      <div class="<?php print $region['row_class']; ?>">
        <?php if ($region_buttons): ?>
          <div class="layout-editor-region-title clearfix">
            <?php print $region_buttons[$name]; ?>
          </div>
        <?php endif; ?>
        <div class="l-flexible-row row">
        <?php foreach ($region['columns'] as $column): ?>
          <div class="l-col col-md-<?php print $column['column_md']; ?>">
            <?php print $content[$column['content_key']]; ?>
          </div>
        <?php endforeach; ?>
        </div>
      </div>
    </<?php print $region['element']; ?>>
  <?php endforeach; ?>
</div>
