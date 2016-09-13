<?php
/**
 * @file
 * Display generic site information such as logo, site name, etc.
 *
 * Available variables:
 *
 * - $base_path: The base URL path of the Backdrop installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $front_page: The URL of the front page. Use this instead of $base_path, when
 *   linking to the front page. This includes the language domain or prefix.
 * - $site_name: The name of the site, empty when display has been disabled.
 * - $site_slogan: The site slogan, empty when display has been disabled.
 * - $menu: The menu for the header (if any), as an HTML string.
 */
?>

<?php
/**
 * Add class for tall or wide logo
 * @todo  tried doing this code in template_preprocess_header, new index in variables
 * didn't make it into this template :(
 */
$header_logo_classes = '';

if (!empty($logo)) {
  $logo_size = getimagesize($logo);
  if (!empty($logo_size)) {
    if ($logo_size[0] < $logo_size[1]) {
      $header_logo_classes = ' header-logo-tall';
    }
  }
}
?>

<?php if ($site_name || $site_slogan || $logo): ?>
  <div class="header-identity-wrapper">
      <div class="header-site-name-wrapper">
        <?php // Strong class only added for semantic value ?>
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" class="header-site-name-link" rel="home">
          <?php if ($logo): ?>
            <div class="header-logo-wrapper<?php print $header_logo_classes; ?>">
              <img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" class="header-logo" />
            </div>
          <?php endif; ?>
          <strong class="semantic">
            <?php print $site_name; ?>
          </strong>
        </a>
      </div>
      <?php if ($site_slogan): ?>
        <div class="header-site-slogan"><?php print $site_slogan; ?></div>
      <?php endif; ?>
  </div>
<?php endif; ?>

<?php if ($menu): ?>
  <nav class="header-menu">
    <?php print $menu; ?>
  </nav>
<?php endif; ?>
