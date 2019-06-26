<?php
/**
 * @file
 * Dashboard block displaying BackdropCMS.org news items.
 */

class DashboardNewsBlock extends Block {
  /**
   * {@inheritdoc}
   */
  public function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
  }

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
    return t('Displays a feed of news from BackdropCMS.org.');
  }

  /**
   * {@inheritdoc}
   */
  public function getContent() {
    $feed = backdrop_http_request('https://backdropcms.org/api/backdrop-news');
    $news = json_decode($feed->data);
    $links = array();

    if ($feed->code === '200') {
      foreach ($news->nodes as $key => $node) {
        $links[] = l(
          $node->node->title,
          $node->node->field_link
        ) . ' - ' . $node->node->field_tagline;
      }
    }

    $build = array(
      '#theme' => 'dashboard_panel__news',
    );
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
   * {@inheritdoc}
   */
  public function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['url'] = array(
      '#type' => 'markup',
      '#markup' => t(
        'This block is providing a news feed from BackdropCMS.org and the
        contents can not be editted. You can remove it from the dashboard if you
        wish.'
      ),
    );

  }

  /**
   * {@inheritdoc}
   */
  public function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
  }
}
