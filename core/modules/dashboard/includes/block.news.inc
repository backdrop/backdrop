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
    $settings = array(
      'title' => t('Backdrop News'),
      'url' => 'https://backdropcms.org/api/news',
    );
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
    $feed = backdrop_http_request($this->settings['url']);
    $news = json_decode($feed->data);
    $links = array();
    foreach ($news->results as $key => $result) {
      $links[] = l(
        $result->node_title,
        $base_url . '/' . $result->_field_data->nid->entity->path->alias
      );
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
      '#type' => 'textfield',
      '#size' => 60,
      '#default_value' => $this->settings['url'],
      '#title' => 'News API URL',
      '#description' => "The source of the news feed.",
    );

  }

  /**
   * {@inheritdoc}
   */
  public function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['url'] = $form_state['values']['url'];

  }
}
