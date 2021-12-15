<?php

/**
 * @file
 * Theme implementation for referenced entities.
 *
 * Available variables:
 * - $content: An array of comment items. Use render($content) to print them all, or
 *   print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $title: The (sanitized) entity label.
 * - $url: Direct url of the current entity if specified.
 * - $page: Flag for the full page state.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. By default the following classes are available, where
 *   the parts enclosed by {} are replaced by the appropriate values:
 *   - entityreference-entity-{ENTITY_TYPE}
 *   - entityreference-entity-{ENTITY_TYPE}-{BUNDLE}
 *
 * @see template_preprocess()
 * @see template_preprocess_entityreference_entity()
 */

?>
<div class="<?php print implode(' ', $classes); ?> clearfix"<?php (empty($attributes)) ? '' : print backdrop_attributes($attributes); ?>>
  <?php if (!$page): ?>
    <h2<?php (empty($title_attributes)) ? '' : print backdrop_attributes($title_attributes); ?>>
      <?php if ($url): ?>
        <a href="<?php print $url; ?>"><?php print $title; ?></a>
      <?php else: ?>
        <?php print $title; ?>
      <?php endif; ?>
    </h2>
  <?php endif; ?>

  <div class="content clearfix"<?php (empty($content_attributes)) ? '' : print backdrop_attributes($content_attributes); ?>>
    <?php
      // We hide the links now so that we can render them later.
      hide($content['links']);
      print render($content);
    ?>
  </div>

  <?php print render($content['links']); ?>
</div>
