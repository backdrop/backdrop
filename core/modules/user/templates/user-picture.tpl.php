<?php
/**
 * @file
 * Default theme implementation to present a picture configured for the
 * user's account.
 *
 * Available variables:
 * - $user_picture: Image set by the user or the site's default. Will be linked
 *   depending on the viewer's permission to view the user's profile page.
 * - $classes: Array of CSS classes to be added to the container.
 * - $account: Array of account information. Potentially unsafe. Be sure to
 *   check_plain() before use.
 *
 * @see template_preprocess_user_picture()
 *
 * @ingroup themeable
 */
?>
<?php if ($user_picture): ?>
  <div class="<?php print implode(', ', $classes); ?>">
    <?php print $user_picture; ?>
  </div>
<?php endif; ?>
