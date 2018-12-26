<?php
/**
 * @file
 * Template for outputting the styling of Message blocks within a layout.
 *
 * This template outputs the same content as the block.tpl.php template, but
 * also adds CSS classes to render the block styled as a message. The classes
 * added are 'messages', and one of 'info', 'status', 'warning', or 'error',
 * depending on what is selected in the "Style settings" section of the block
 * configuration dialog, when the block style is set to "Message". This allows
 * site-builders to configure text blocks to be rendered as messages, through
 * the user interface.
 *
 * Variables available:
 * - $message_type: The HTML tag to be used around the entire block.
 * - $classes: Array of classes that should be displayed on the block's wrapper.
 * - $attributes: Attributes that should be displayed on this block's wrapper.
 * - $title: The title of the block.
 * - $title_prefix/$title_suffix: A prefix and suffix for the title tag. This
 *   is important to print out as administrative links to edit this block are
 *   printed in these variables.
 * - $title_tag: The HTML tag to be used on the title of the block.
 * - $content_tag: The HTML tag to be used around the content.
 * - $content_attributes: Attributes that should be displayed on the content.
 * - $content: The actual content of the block.
 */
?>
<?php if ($message_type): ?>
  <div class="message-block messages <?php print $message_type; ?>"<?php print backdrop_attributes($attributes); ?>>
<?php endif; ?>

  <div class="block-content">
    <?php print render($content); ?>
  </div>
</div>
