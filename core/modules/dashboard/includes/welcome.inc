<?php

/**
 * @file
 *
 * "Welcome" dashboard block.
 */


class DashboardWelcomeBlock extends Block {

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
    return t('Intro to Total Control');
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
    $content .= '  <p>' . t('Welcome to your administrative dashboard.  !panel 
      to add more content panes here, or configure those provided by default. 
      Views content panes can be created for you automatically according to your 
      !settings.  Make this dashboard the perfect hub for all your administrative 
      needs.', array(
      '!panel' => l(t('Edit this panel'), 'admin/structure/pages/nojs/operation/page-dashboard/handlers/page_dashboard_panel_context/content'),
      '!settings' => l(t('Total Control Settings'), 'admin/config/administration/control'),
    )) . '</p>';
    $content .= '</div>';

  return $content;
  }
}
