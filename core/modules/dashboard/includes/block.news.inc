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
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Backdrop News');
  }

  /**
   * {@inheritdoc}
   */
  public function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Displays a news feed from BackdropCMS.org.');
  }

  /**
   * {@inheritdoc}
   */
  public function getContent() {
    global $user;

    $news = $this->getNews();

    $links = array();
    if ($news) {
      $dashboard_news_last_seen = $user->dashboard_news_seen;
      foreach ($news as $key => $notifications) {
        $is_new = $dashboard_news_last_seen < $notifications['timestamp'];
        $is_new_marker = $is_new ? '<span class="marker">' . t('new') . '</span> ' : '';

        $notification_date = format_date($notifications['timestamp'], 'custom', 'M j, Y');
        $notification_link = l($notifications['title'], $notifications['url'], array('attributes' => array('target'=>'_blank')));
        $notification_summary = filter_xss($notifications['summary']);

        $notification = array(
          'data' => '<p>' . $is_new_marker . $notification_date . ': ' . $notification_link . ' - ' . $notification_summary . '</p>',
        );

        if ($is_new) {
          $notification['class'] = array('is-new');
        }

        $links[] = $notification;
      }

      // Log the time the current user has seen the 'Backdrop News' block.
      $user->dashboard_news_seen = REQUEST_TIME;
      db_update('users')
        ->fields(array('dashboard_news_seen' => $user->dashboard_news_seen))
        ->condition('uid', $user->uid)
        ->execute();
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

  /**
   * Return the current list of news items.
   *
   * This response is cached. If there is no recent cache, a new one will be
   * generated on-the-fly. If the first request fails, the cache will persist
   * with FALSE as the saved value until the next time the cache is cleared or
   * cron updates the news list.
   *
   * @return FALSE|array
   */
  public static function getNews() {
    $cache = cache()->get('dashboard_news_feed');
    // If no cache is set at all, fetch results immediately.
    if ($cache === FALSE) {
      $news = DashboardNewsBlock::refreshNewsFeed();
    }
    else {
      $news = $cache->data;
    }
    return $news;
  }

  /**
   * Fetch news from the news source and set the cache for future responses.
   *
   * @return FALSE|array
   */
  public static function refreshNewsFeed() {
    $news = FALSE;
    $feed_url = config_get('dashboard.settings', 'news_feed_url');

    // A short timeout means that we won't delay loading the dashboard if the
    // cache is cleared and no network is available.
    if ($feed_url) {
      $feed = backdrop_http_request($feed_url, array(
        'timeout' => 2,
      ));
      if ($feed->code === '200') {
        $news = json_decode($feed->data, TRUE);
      }
    }

    // Cache for one week, though the feed will be updated more frequently by
    // dashboard_cron().
    cache()->set('dashboard_news_feed', $news, REQUEST_TIME + 604800);

    return $news;
  }
}
