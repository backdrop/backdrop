<?php

/**
 * @file
 * Default theme implementation for entities.
 *
 * Available variables:
 * - $content: An array of comment items. Use render($content) to print them all, or
 *   print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $title: The (sanitized) entity label.
 * - $url: Direct url of the current entity if specified.
 * - $attributes: Array of additional HTML attributes that should be added to
 *   the wrapper element. Flatten with backdrop_attributes().
 * - $content_attributes: An array of content attributes to wrap around content.
 * - $classes: Array of classes that can be used to style contextually through
 *   CSS. By default the following classes are available, where the parts
 *   enclosed by {} are replaced by the appropriate values:
 *   - entity-{ENTITY_TYPE}
 *   - {ENTITY_TYPE}-{BUNDLE}
 *   - view-mode-{VIEWMODE}
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * Other variables:
 * - $entity: Full entity. Contains data that may not be safe.
 * - $entity_type: Entity type.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 *
 * Entity status variables:
 * - $view_mode: Display mode, e.g. 'full', or 'teaser'.
 * - $page: Flag for the full page state.
 * - $is_front: Flags true when presented in the home page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * @see template_preprocess()
 * @see template_preprocess_entity()
 */
?>
<div id="<?php print $entity_type; ?>-<?php print $entity_id; ?>" class="<?php print implode(' ', $classes); ?>"<?php (empty($attributes)) ? '' : print backdrop_attributes($attributes); ?>>
  <?php print render($title_prefix); ?>
    <?php if (!$page && !empty($title)): ?>
      <h2><a href="<?php print $url; ?>" rel="bookmark"><?php print $title; ?></a></h2>
    <?php endif; ?>
  <?php print render($title_suffix); ?>

  <div class="content clearfix"<?php (empty($content_attributes)) ? '' : print backdrop_attributes($content_attributes); ?>>
    <?php
      print render($content);
    ?>
  </div>
</div>
