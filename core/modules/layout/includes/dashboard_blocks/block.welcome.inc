<?php

/**
 * @file
 *
 * "Welcome" dashboard block.
 */


class DashboardWelcomeBlock extends DashboardBlock {

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Welcome');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Introduction to the dashboard');
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    // Build the content.
    $content =  '<div class="total-control-site-overview">';
    $content .= '  <p>' . t('Welcome to your administrative dashboard.  !layout 
      to add more content panes here, or configure those provided by default.', array(
      '!layout' => l(t('Edit this layout'), 'admin/structure/layouts/manage/dashboard'),
    )) . '</p>';
    $content .= '</div>';

  return $content;
  }
}
