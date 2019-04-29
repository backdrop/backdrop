<?php
/**
 * @file
 *
 * Dashboard block which displays a summary of content statistics including:
 * - number pieces of each type of content
 * - number of comments
 * - number of blocked comments (spam)
 * - configuration links
 */
class DashboardOverviewContentBlock extends Block {

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Content');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Overview of content');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Displays a summary of content statistics.');

  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    $items = dashboard_get_content_overview($this->settings);
    return theme('dashboard_overview_content', array('overview' => $items));

  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    // Add content settings to form.
    dashboard_add_content_block_settings($form, $this->settings);
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['spam'] = $form_state['values']['spam'];
    foreach (array_keys($form_state['values']['types']) as $key) {
      $this->settings['types'][$key] = $form_state['values']['types'][$key];
    }
    foreach (array_keys($form_state['values']['comments']) as $key) {
      $this->settings['comments'][$key] = $form_state['values']['comments'][$key];
    }
  }
}
