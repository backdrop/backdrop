<?php
/**
 * @file
 * Theme wrapper for the "Log in", "Reset password", and "Create new Account"
 * forms when shown on simplified (layout suppressed) pages.
 *
 * Available variables:
 *   - $form: The rendered HTML for the wrapped form.
 *
 * @since 1.30.0 template added
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

      <?php print render($form); ?>
      <div class="user-tabs-links">
        <?php print render($links); ?>
      </div>

    </div>
  </div>
</div>
