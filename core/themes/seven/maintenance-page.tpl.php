<!DOCTYPE html>
<html lang="<?php print $language->langcode ?>" dir="<?php print $language->dir ?>">
  <head>
    <title><?php print $head_title; ?></title>
    <?php print $head; ?>
    <?php print $styles; ?>
    <?php print $scripts; ?>
  </head>
  <body class="<?php print $classes; ?>">
    <?php print $page_top; ?>
    <div id="page">

    <?php if ($sidebar_first): ?>
      <div id="sidebar-first" class="sidebar">
        <?php if ($logo): ?>
          <img id="logo" src="<?php print $logo ?>" alt="<?php print $site_name ?>" />
        <?php endif; ?>
        <?php print $sidebar_first ?>
      </div>
    <?php endif; ?>

    <main id="content" class="clearfix">
      <?php if ($title): ?><h1 class="page-title"><?php print $title; ?></h1><?php endif; ?>
      <?php if ($messages): ?>
        <div id="console"><?php print $messages; ?></div>
      <?php endif; ?>
      <?php print $content; ?>
    </main>
  </div>

  <footer role="contentinfo">
    <?php print $page_bottom; ?>
  </footer>

  </body>
</html>
