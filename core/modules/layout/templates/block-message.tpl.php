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
 * * Variables available:
 * - $message_type: 'info', 'status', 'warning', or 'error' - set from the user
 *   interface.
 * - $attributes: Attributes that should be displayed on this block's wrapper.
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
