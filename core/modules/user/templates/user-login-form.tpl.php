<?php
/**
 * @file
 * Renders the user login form.
 *
 * Use render($user_login_form) to print all form items, or print a subset
 * such as render($user_login_form['name']). Always call
 * backdrop_render_children($user_login_form) at the end in order to print all
 * remaining items.
 *
 * Available variables:
 *   - $user_login_form: An array of form items. Use render() to print them.
 *
 * @since 1.29.0 template added
 * @ingroup themeable
 */
?>
<?php if ($user_login_page_simplified): ?>
  <div class="<?php print implode(' ', $classes); ?>">
    <div class="user-login-wrapper">
      <?php if ($logo_image): ?>
        <div class="login-page-logo">
          <?php print $logo_image; ?>
        </div>
      <?php endif; ?>
      <h1 class="page-title">
        <?php print $site_name; ?>
      </h1>
      <h2 class="login-page-page-title">
        <?php print $page_title; ?>
      </h2>
      <div class="login-page-messages">
        <?php print $messages; ?>
      </div>
<?php endif; ?>

<?php print backdrop_render_children($user_login_form); ?>

<?php if ($user_login_page_simplified): ?>
    </div>
  </div>
<?php endif; ?>
