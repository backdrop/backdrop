<?php
// $Id$
/**
 * @file views-view-summary-unformatted.tpl.php
 * Default simple view template to display a group of summary lines
 *
 * This wraps items in a span if set to inline, or a div if not.
 *
 * @ingroup views_templates
 */
?>
<?php foreach ($rows as $row): ?>
  <?php print (!empty($options['inline']) ? '<span' : '<div') . ' class="views-summary views-summary-unformatted">'; ?>
    <?php if (!empty($row->separator)) { print $row->separator; } ?>
    <a href="<?php print $row->url; ?>"><?php print $row->link; ?></a>
    <?php if (!empty($options['count'])): ?>
      (<?php print $row->count; ?>)
    <?php endif; ?>
  <?php print !empty($options['inline']) ? '</span>' : '</div>'; ?>
<?php endforeach; ?>
