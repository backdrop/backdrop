<?php

/**
 * @file
 * Provides an HTML container for comments.
 *
 * Available variables:
 * - $content: The array of content-related elements for the node. Use
 *   render($content) to print them all, or
 *   print a subset such as render($content['comment_form']).
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default value has the following:
 *   - comment-wrapper: The current template type, i.e., "theming hook".
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * The following variables are provided for contextual information.
 * - $node: Node entity the comments are attached to.
 * The constants below the variables show the possible values and should be
 * used for comparison.
 * - $display_mode
 *   - COMMENT_MODE_FLAT
 *   - COMMENT_MODE_THREADED
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * @see template_preprocess_comments()
 *
 * @ingroup themeable
 */
?>
<section id="comments" class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php if ($content['comments']): ?>
    <?php print render($title_prefix); ?>
    <h2 class="title"><?php print t('Comments'); ?></h2>
    <?php print render($title_suffix); ?>
  <?php endif; ?>

  <?php foreach ($content as $comment):   ?>
    <article class="<?php print $comment['classes']; ?> clearfix"<?php print $comment['attributes']; ?>>

      <?php print render($comment['title_prefix']); ?>
      <?php if ($comment['new']): ?>
        <mark class="new"><?php print $comment['new']; ?></mark>
      <?php endif; ?>
      <h3<?php print $comment['title_attributes']; ?>><?php print $comment['title']; ?></h3>
      <?php print render($comment['title_suffix']); ?>

      <footer>
        <?php print $comment['user_picture']; ?>
        <p class="submitted"><?php print $comment['submitted']; ?></p>
        <?php print $comment['permalink']; ?>
      </footer>

      <div class="content"<?php print $comment['content_attributes']; ?>>
        <?php
          // We hide the links now so that we can render them later.
          hide($comment['content']['links']);
          print render($comment['content']);
        ?>
        <?php if ($comment['signature']): ?>
        <div class="user-signature">
          <?php print $comment['signature']; ?>
        </div>
        <?php endif; ?>
      </div>

      <?php print render($comment['content']['links']) ?>
    </article>
  <?php endforeach; ?>
  
  <?php if ($content['comment_form']): ?>
    <h2 class="title comment-form"><?php print t('Add new comment'); ?></h2>
    <?php print render($content['comment_form']); ?>
  <?php endif; ?>
</section>
