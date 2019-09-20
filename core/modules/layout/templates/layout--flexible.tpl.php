<?php
/**
 * @file
 * Template for a flexible template.
 *
 * Variables:
 * - $messages: Status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links: Array of actions local to the page, such as 'Add menu' on
 *   the menu administration interface.
 * - $classes: Array of classes to be added to the layout wrapper.
 * - $attributes: Additional attributes to be added to the layout wrapper.
 * - $row_data: An array of information about each row. Each item in the array
 *   contains the folowing information:
 *   - $row_data['region_md']: the row region widths in Bootstrap format.
 *   - $row_data['region_name']: the region name.
 *   - $row_data['content_key']: The key of the $content array which contains
 *     the HTML for that region.
 * - $content: An array of content, each item in the array is nameed to one
 *   region of the layout.
 */
?>
<div class="layout--flexible layout <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>
  <div class="layout-flexible-content <?php $region_buttons ? print 'layout-flexible-editor' : ''; ?>">
  <?php foreach ($row_data as $name => $row): ?>
    <<?php print $row['element']; ?> data-row-id="<?php print $name; ?>" class="flexible-row <?php print 'l-' . $name; ?>" <?php print $row['row_id']; ?>>
      <div class="<?php print $row['row_class']; ?>">
        <?php if ($region_buttons): ?>
          <div class="layout-flexible-region-buttons clearfix">
            <?php print $region_buttons[$name]; ?>
          </div>
        <?php endif; ?>
        <div class="l-flexible-row row">
        <?php foreach ($row['regions'] as $region): ?>
          <div class="l-col col-md-<?php print $region['region_md']; ?>">
            <?php if ($region_buttons): ?>
              <div class="layout-editor-region" id="layout-editor-region-<?php print $name; ?>" data-region-name="<?php print $name; ?>">
                <div class="layout-editor-region-title clearfix">
                  <h2 class="label"><?php print $region['region_name']; ?></h2>
                </div>
              </div>
            <?php else: ?>
              <?php print $content[$region['content_key']]; ?>
            <?php endif; ?>
          </div>
        <?php endforeach; ?>
        </div>
      </div>
    </<?php print $row['element']; ?>>
  <?php endforeach; ?>
  </div>
</div>
