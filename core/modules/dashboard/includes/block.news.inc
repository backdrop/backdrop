<?php
/**
 * @file
 * Dashboard block displaying BackdropCMS.org news items.
 */

class DashboardNewsBlock extends Block {

  /**
   * {@inheritdoc}
   */
  public function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Backdrop News');
  }

  /**
   * {@inheritdoc}
   */
  public function getAdminTitle() {
    return t('Backdrop News');
  }

  /**
   * {@inheritdoc}
   */
  public function getAdminPreview() {
    return t('Displays a news feed from BackdropCMS.org.');
  }

  /**
   * {@inheritdoc}
   */
  public function getContent() {
    $feed = backdrop_http_request('https://backdropcms.org/api/notifications');
    $links = array();
    if ($feed->code === '200') {
      $news = json_decode($feed->data);
      foreach ($news->notifications as $key => $notifications) {
        $links[] = '<p>' . l(
            $notifications->notification->title,
            $notifications->notification->url
          ) . ' - ' . $notifications->notification->summary . '</p>';
      }
    }

    $build = array();
    if (empty($links)) {
      $build['list'] = array(
        '#type' => 'markup',
        '#markup' => t('No news at this time.'),
      );
    }
    else {
      $build['list'] = array(
        '#theme' => 'item_list',
        '#items' => $links,
      );
    }

    return $build;
  }

  /**
   *    * {@inheritdoc}
   */
  public function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['url'] = array(
      '#type' => 'help',
      '#markup' => t(
        'This block provides a news feed from BackdropCMS.org. Its contents can not be edited.'
      ),
    );
  }
}
