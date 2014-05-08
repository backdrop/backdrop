<?php

/**
 * @file
 * Default theme implementation for printed version of book outline.
 *
 * Available variables:
 * - $title: Top level node title.
 * - $head: Header tags.
 * - $language: Language object.
 * - $language_rtl: TRUE or FALSE depending on right to left language scripts.
 * - $base_url: URL to home page.
 * - $contents: Nodes within the current outline rendered through
 *   book-node-export-html.tpl.php.
 *
 * @see template_preprocess_book_export_html()
 *
 * @ingroup themeable
 */
?>
<!DOCTYPE html >
<html<?php print $html_attributes; ?>>
  <head>
    <title><?php print $title; ?></title>
    <?php print $head; ?>
    <base href="<?php print $base_url; ?>" />
    <link type="text/css" rel="stylesheet" href="misc/print.css" />
    <?php if ($language_rtl): ?>
      <link type="text/css" rel="stylesheet" href="misc/print-rtl.css" />
    <?php endif; ?>
  </head>
  <body>
    <?php
    /**
     * The given node is /embedded to its absolute depth in a top level
     * section/. For example, a child node with depth 2 in the hierarchy is
     * contained in (otherwise empty) &lt;div&gt; elements corresponding to
     * depth 0 and depth 1. This is intended to support WYSIWYG output - e.g.,
     * level 3 sections always look like level 3 sections, no matter their
     * depth relative to the node selected to be exported as printer-friendly
     * HTML.
     */
    $div_close = '';
    ?>
    <?php for ($i = 1; $i < $depth; $i++): ?>
      <div class="section-<?php print $i; ?>">
      <?php $div_close .= '</div>'; ?>
    <?php endfor; ?>
    <?php print $contents; ?>
    <?php print $div_close; ?>
  </body>
</html>
