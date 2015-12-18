<?php
/**
 * @file
 * Template for the Taylor layout.
 *
 * Variables:
 * - $title: The page title, for use in the actual HTML content.
 * - $messages: Status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node.)
 * - $action_links: Array of actions local to the page, such as 'Add menu' on
 *   the menu administration interface.
 * - $classes: Array of CSS classes to be added to the layout wrapper.
 * - $attributes: Array of additional HTML attributes to be added to the layout
 *     wrapper. Flatten using backdrop_attributes().
 * - $content: An array of content, each item in the array is keyed to one
 *   region of the layout. This layout supports the following sections:
 *   - $content['header']
 *   - $content['top']
 *   - $content['half']
 *   - $content['quarter1']
 *   - $content['quarter2']
 *   - $content['bottom']
 *   - $content['footer']
 */
?>
<div class="layout--taylor <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>

  <?php if ($content['header']): ?>
    <header class="l-header" role="banner" aria-label="<?php print t('Site header'); ?>">
      <div class="container">
        <?php print $content['header']; ?>
      </div>
    </header>
  <?php endif; ?>

  <?php if ($messages): ?>
    <div class="l-messages container">
      <?php print $messages; ?>
    </div>
  <?php endif; ?>

  <div class="l-container">
  <div class="l-container-inner container">
    <div class="l-page-header">
      <a id="main-content"></a>
      <?php print render($title_prefix); ?>
      <?php if ($title): ?>
        <h1 class="title" id="page-title">
          <?php print $title; ?>
        </h1>
      <?php endif; ?>
      <?php print render($title_suffix); ?>
    </div>

    <?php if ($tabs): ?>
      <div class="tabs">
        <?php print $tabs; ?>
      </div>
    <?php endif; ?>

    <?php print $action_links; ?>

    <div class="container-fluid">
      <div class="row">
        <div class="col-md-12 l-topcolumn panel-panel">
          <?php print $content['top']; ?>
        </div>
      </div>

      <div class="row">
        <div class="col-md-6 l-half panel-panel">
          <?php print $content['half']; ?>
        </div>
        <div class="col-md-3 l-quarter1 panel-panel">
          <?php print $content['quarter1']; ?>
        </div>
        <div class="col-md-3 l-quarter2 panel-panel">
          <?php print $content['quarter2']; ?>
        </div>
      </div>

      <div class="row">
        <div class="col-md-12 l-bottom panel-panel">
          <?php print $content['bottom']; ?>
        </div>
      </div>
    </div>
  </div>
  </div>

  <?php if ($content['footer']): ?>
    <footer class="l-footer"  role="footer">
      <div class="container">
        <?php print $content['footer']; ?>
      </div>
    </footer>
  <?php endif; ?>
</div><!-- /.taylor -->
