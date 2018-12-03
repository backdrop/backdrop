<?php

/**
 * @file
 *
 * "Welcome" dashboard block.
 */


class DashboardBlock extends Block {

  /**
   * The block title.
   *
   * @var string
   */
  var $block_title;

  /**
   * The block description.
   *
   * @var string
   */
  var $block_description;

  /**
   * Build a block using the legacy hook_block_view().
   */
  protected function buildBlock() {
    $block_info = dashboard_block_info();
    $block_data = array();
    if(isset($block_info[$this->delta])) {
      $block_data['block_title'] = $block_info[$this->delta]['info'];
      $block_data['block_description'] = $block_info[$this->delta]['description'];
    }
    return $block_data;
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    $title = parent::getTitle();

    // Use the default title if expected.
    if (empty($title) && $this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      $block_data = $this->buildBlock();
      if (isset($block_data['block_title'])) {
        $title = $block_data['block_title'];
      }
    }

    return $title;
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminTitle() {
      $block_data = $this->buildBlock();
      if (isset($block_data['block_title'])) {
        $title = $block_data['block_title'];
      }

    return $title;
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminPreview() {
      $block_data = $this->buildBlock();
      if (isset($block_data['block_description'])) {
        $block_description = $block_data['block_description'];
      }

    return $block_description;
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
