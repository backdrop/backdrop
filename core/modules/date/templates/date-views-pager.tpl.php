<?php
/**
 * @file
 * Template to display the Views date pager.
 *
 * Available variables:
 * - $plugin: The pager plugin object. This contains the view.
 * - $plugin->view: The view object for this navigation.
 * - $classes: Array of classes that can be used to style contextually through
 *   CSS.
 * - $nav_title: The formatted title for this view. In the case of block
 *   views, it will be a link to the full view, otherwise it will
 *   be the formatted name of the year, month, day, or week.
 * - $prev_url: Url for the previous calendar page. The link is created
 *    in the template to make it easier to change the text, add images, etc.
 * - $next_url: Url for the next calendar page. The link is created
 *    in the template to make it easier to change the text, add images, etc.
 * - $prev_options,
 * - $next_options: Query strings and other options for the links that need to
 *   be used in the l() function, including rel=nofollow.
 */
?>
<?php if (!empty($pager_prefix)) : ?>
<?php print $pager_prefix; ?>
<?php endif; ?>
<div class="<?php print implode(' ', $classes); ?> clearfix">
  <div class="date-nav">
    <div class="date-heading">
      <h3><?php print $nav_title ?></h3>
    </div>
    <ul class="pager">
    <?php if (!empty($prev_url)) : ?>
      <li class="date-prev">
        <?php
        $text = '&laquo;';
        $text .= $mini ? '' : ' ' . t('Prev', array(), array('context' => 'date_nav'));
        print l(t($text), $prev_url, $prev_options);
        ?>
      </li>
    <?php endif; ?>
    <?php if (!empty($next_url)) : ?>
      <li class="date-next">
        <?php print l(($mini ? '' : t('Next', array(), array('context' => 'date_nav')) . ' ') . '&raquo;', $next_url, $next_options); ?>
      </li>
    <?php endif; ?>
    </ul>
  </div>
</div>
