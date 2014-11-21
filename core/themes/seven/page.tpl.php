<?php

/**
 * @file
 * Seven's theme implementation to display a single Backdrop page.
 *
 * The doctype, html, head, and body tags are not in this template. Instead
 * they can be found in the html.tpl.php template normally located in the
 * core/modules/system directory.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 * @see seven_preprocess_page()
 *
 * @ingroup themeable
 */
?>

  <header id="branding" class="clearfix" role="navigation">
    <?php print $breadcrumb; ?>
    <?php print render($title_prefix); ?>
    <?php if ($title): ?>
      <h1 class="page-title"><?php print $title; ?></h1>
    <?php endif; ?>
    <?php print render($title_suffix); ?>
    <?php print render($primary_local_tasks); ?>
  </header>

  <div id="page">
    <?php if ($secondary_local_tasks): ?>
      <div class="tabs-secondary clearfix"><?php print render($secondary_local_tasks); ?></div>
    <?php endif; ?>

    <main id="content" class="clearfix" role="main">
      <div class="element-invisible"><a id="main-content"></a></div>
      <?php if ($messages): ?>
        <div id="console" class="clearfix"><?php print $messages; ?></div>
      <?php endif; ?>
      <?php if ($action_links): ?><ul class="action-links"><?php print render($action_links); ?></ul><?php endif; ?>
      <?php print render($page['content']); ?>
    </main>

    <footer id="footer" role="contentinfo">
      <?php print $feed_icons; ?>
    </footer>

  </div>
