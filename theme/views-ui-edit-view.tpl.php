<?php
// $Id: views-ui-edit-view.tpl.php,v 1.10.6.2 2010/10/16 14:55:13 dereine Exp $
/**
 * @file views-ui-edit-view.tpl.php
 * Template for the primary view editing window.
 */
?>
<div class="views-edit-view">
  <?php if ($locked): ?>
    <div class="view-locked">
       <?php print t('This view is being edited by user !user, and is therefore locked from editing by others. This lock is !age old. Click here to <a href="!break">break this lock</a>.', array('!user' => $locked, '!age' => $lock_age, '!break' => $break)); ?>
    </div>
  <?php endif; ?>

  <?php /* print $tabs; */ ?>

  <?php /* print $save_button; */ ?>

  <h2><?php print t('Live preview'); ?></h2>
  <div id='views-live-preview'>
    <?php print $preview ?>
  </div>
</div>
