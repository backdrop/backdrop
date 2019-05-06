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

  <?php foreach ($regions as $name => $region): ?>
    <?php 
      $row_class = 'flexible-row-' . $name . ' ' . $region['contains'] . ' ' . $region['classes'];
      $row_id = $flexible_editor ? 'id = "flexible-row-id-' . $name . '"' : '';
      $element = !empty($region['element']) ? $region['element'] : 'div';
    ?>
    <<?php print $element; ?> class="l-wrapper <?php print 'l-' . $name; ?>" <?php print $row_id; ?>>
      <div class="container container-fluid <?php print $row_class; ?>">
        <?php if ($region_buttons): ?>
          <div class="layout-editor-region-title clearfix">
            <?php print $region_buttons[$name]; ?>
          </div>
        <?php endif; ?>
        <div class="l-flexible-row row">
        <?php if ($region['contains'] == 'column_12'): ?>
            <div class="l-col col-md-12">
              <?php $content_key = $flexible_editor ? $name : $name . '_0'; ?>
              <?php print $content[$content_key]; ?>
            </div>
        <?php else: ?>
          <?php 
            $col_info = $column_data[$region['contains']];
            $split = explode(':', $col_info['bootstrap']);
            $i = 0;
          ?>
          <?php foreach ($split as $col): ?>
            <div class="l-col col-md-<?php print $col; ?>">
              <?php $content_key = $flexible_editor ? $name : $name . '_' . $i; ?>
              <?php print $content[$content_key]; ?>
              <?php $i++; ?>
            </div>
          <?php endforeach; ?>
        <?php endif; ?>
        </div>
      </div>
    </<?php print $element; ?>>
  <?php endforeach; ?>
</div>
