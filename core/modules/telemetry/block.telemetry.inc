<?php
/**
 * @file
 * Displays informtion about site telemetry.
 */
class TelemetryBlock extends Block {
  /**
   * {@inheritdoc}
   */
/*   function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $all_types = array_keys(node_type_get_names());

    // Default status for comment types.
    $enabled_comment_types = array('post', 'article', 'blog');
    $enabled_comment_types = array_intersect_key($enabled_comment_types, $all_types);

    $this->settings += array(
      'types' => array(),
      'comment_enabled' => TRUE,
      'comment_include_unapproved' => TRUE,
      'comment_types' => $enabled_comment_types,
    );
  }
 */
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Telemetry');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Telemetry');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Displays telemetry information.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $items = array();
    if (user_access('access site reports')) {
      $items[] = l(t('See what information is being sent to Backdrop CMS'), 'admin/reports/telemetry');
    }
    if (user_access('administer modules')) {
      $items[] = l(t('Disable the telemetry module'), 'admin/modules');
    }
    $items[] = l(t('View the Backdrop CMS data collection policy'), 'https://backdropcms.org/philosophy');
    $items[] = l(t('View public Backdrop CMS data'), 'https://backdropcms.org/project/backdrop/telemetry');

    $panel = array();
    $panel['enabled'] = array(
      '#type' => 'help',
      '#markup' => t('Telemetry is enabled'),
    );
    $panel['list'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
    );

    return $panel;
  }

}
