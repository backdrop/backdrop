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

<div class="<?php print implode(' ', $classes); ?>">
  <div class="user-simplified-page-wrapper">
    <div class="user-simplified-page-wrapper-inner">
      <?php if ($logo_image): ?>
        <div class="user-simplified-page-logo">
          <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" class="logo">
            <?php print $logo_image; ?>
          </a>
        </div>
      <?php endif; ?>
      <h1 class="site-name">
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
      </h1>
      <h2 class="user-simplified-page-title">
        <?php print $page_title; ?>
      </h2>
      <div class="user-simplified-page-messages">
        <?php print $messages; ?>
      </div>

      <?php print $form; ?>

    </div>
  </div>
</div>
