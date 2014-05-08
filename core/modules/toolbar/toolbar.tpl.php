<?php

/**
 * @file
 * Default template for admin toolbar.
 *
 * Available variables:
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default value has the following:
 *   - toolbar: The current template type, i.e., "theming hook".
 * - $toolbar['toolbar_user']: User account / logout links.
 * - $toolbar['toolbar_menu']: Top level management menu links.
 * - $toolbar['toolbar_drawer']: A place for extended toolbar content.
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * @see template_preprocess()
 * @see template_preprocess_toolbar()
 *
 * @ingroup themeable
 */
?>
<nav id="toolbar" role="navigation" class="<?php print $classes; ?> clearfix">
  <div class="toolbar-menu clearfix">
    <?php print render($toolbar['toolbar_home']); ?>
    <?php print render($toolbar['toolbar_user']); ?>
    <?php print render($toolbar['toolbar_menu']); ?>
  </div>
</nav>
