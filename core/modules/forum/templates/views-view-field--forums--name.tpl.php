<?php
/**
 * @file
 * This template is used to print the forum name.
 *
 * Variables available:
 * - $view: The view object
 * - $field: The field handler object that can process the input
 * - $row: The raw SQL result that can be used
 * - $output: The processed output that will normally be used.
 * - $icon_title: The icon title.
 * - $icon_class: The icon CSS class.
 *
 * When fetching output from the $row, this construct should be used:
 * $data = $row->{$field->field_alias}
 *
 * The above will guarantee that you'll always get the correct data,
 * regardless of any changes in the aliasing that might happen if
 * the view is modified.
 */
?>
<div class="forum-name">
  <div class="icon forum-status-<?php print $icon_class; ?>" title="<?php print $icon_title; ?>">
    <span class="element-invisible"><?php print $icon_title; ?></span>
  </div>
  <span class="forum-name"><?php print $output; ?></span>
</div>
    
