<?php
/**
 * @file
 * Template for a 2 column flipped layout.
 *
 * This template provides a two column layout with the sidebar on the left and a roughly
 * 60/40 split.
 *
 * Variables:
 * - $title: The page title, for use in the actual HTML content.
 * - $messages: Status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links: Array of actions local to the page, such as 'Add menu' on
 *   the menu administration interface.
 * - $classes: Array of CSS classes to be added to the layout wrapper.
 * - $attributes: Array of additional HTML attributes to be added to the layout
 *     wrapper. Flatten using backdrop_attributes().
 * - $content: An array of content, each item in the array is keyed to one
 *   region of the layout. This layout supports the following sections:
 *   - $content['header']
 *   - $content['top']
 *   - $content['content']
 *   - $content['sidebar-flipped']
 *   - $content['footer']
 */
?>
<div class="layout-two-column-flipped <?php print implode(' ', $classes); ?>"<?php print backdrop_attributes($attributes); ?>>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>
  <?php if ($content['header']): ?>
    <header id="header" role="banner" aria-label="<?php print t('Site header'); ?>"><div class="section clearfix">
      <?php print $content['header']; ?>
    </div></header>
  <?php endif; ?>

  <?php if ($content['top']): ?>
    <div id="top"><div class="section clearfix">
      <?php print $content['top']; ?>
    </div></div> <!-- /.section, /#top -->
  <?php endif; ?>

  <?php if ($messages): ?>
    <div id="messages"><div class="section clearfix">
      <?php print $messages; ?>
    </div></div> <!-- /.section, /#messages -->
  <?php endif; ?>

  <div id="main-wrapper" class="clearfix"><div id="main" class="clearfix">
    <main id="content" class="column" role="main"><div class="section">
      <a id="main-content"></a>
      <?php print render($title_prefix); ?>
      <?php if ($title): ?>
        <h1 class="title" id="page-title">
          <?php print $title; ?>
        </h1>
      <?php endif; ?>
      <?php print render($title_suffix); ?>

      <?php if ($tabs): ?>
        <div class="tabs">
          <?php print $tabs; ?>
        </div>
      <?php endif; ?>

      <?php print $action_links; ?>
      <?php print $content['content']; ?>
    </div></main> <!-- /.section, /#content -->

    <?php if ($content['sidebar_flipped']): ?>
    <div id="sidebar-flipped" class="column sidebar"><div class="section">
      <?php print $content['sidebar_flipped']; ?>
    </div></div> <!-- /.section, /#sidebar-first -->
    <?php endif; ?>

  </div></div><!-- /#main, /#main-wrapper -->

  <?php if ($content['footer']): ?>
    <div id="footer" class="clearfix"><div class="section">
      <?php print $content['footer']; ?>
    </div></div><!-- /.section, /#footer -->
  <?php endif; ?>
</div>
