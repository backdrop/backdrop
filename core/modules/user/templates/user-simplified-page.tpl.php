<?php
/**
 * @file
 * Theme template for the login forms (for example, "Log in" and "Create new
 * Account") when shown on simplified (layout suppressed) pages.
 *
 * Available variables:
 *   - $form: The form render array.
 *   - $links: A render array with related form links.
 *
 * @since 1.30.0 Template added
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
      <?php else: ?>
        <div class="site-name">
          <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
        </div>
      <?php endif; ?>
      <h1 class="user-simplified-page-title">
        <?php print $page_title; ?>
      </h1>
      <div class="user-simplified-page-messages">
        <?php print $messages; ?>
      </div>

      <?php print render($form); ?>
      <div class="user-form-links">
        <?php print render($links); ?>
      </div>

    </div>
  </div>
</div>
