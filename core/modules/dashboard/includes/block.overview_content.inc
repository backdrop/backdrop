<?php
/**
 * @file
 * Dashboard block displaying informtion about content, including:
 * - number pieces of each type of content
 * - number of comments
 * - number of unapproved comments
 * - configuration links
 */
class DashboardOverviewContentBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
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

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Content overview');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Overview of content');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Displays a summary of content statistics.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $settings = $this->settings;

    // Not cached since comment and node CRUD operations are likely to be more
    // frequent than Dashboard access events.
    $items = array();
    $types = node_type_get_types();
    $comment_enabled = module_exists('comment') && $settings['comment_enabled'];

    foreach ($types as $machine => $node_type) {
      // Compare against node type option on block settings.
      if (in_array($machine, $settings['types']) || empty($settings['types'])) {
        $type_count = db_query("SELECT count(*) FROM {node} WHERE type = :type and status = 1", array(':type' => $machine))->fetchField();
        $content_data[$machine] = format_plural($type_count, '1 @type item', '@count @type items', array('@type' => t($node_type->name)));

        // Check if Comment module is enabled.
        if ($comment_enabled) {
          // Compare against comment options on block settings.
          if (in_array($machine, $settings['comment_types'])) {
            $comment_count = db_query("SELECT count(DISTINCT cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 1 AND n.status = 1", array(':type' => $machine))->fetchField();

            // Compare against unapproved option checkbox on block settings.
            if ($settings['comment_include_unapproved']) {
              $unapproved_count = db_query("SELECT count(DISTINCT c.cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 0 AND n.status = 1", array(':type' => $machine))->fetchField();
              $content_data[$machine . '_comments_unapproved'] = format_plural($unapproved_count, '1 unapproved', '@count unapproved');
              $comment_count += $unapproved_count;
            }

            $content_data[$machine . '_comments'] =  format_plural($comment_count, '1 comment', '@count comments');
          }
        }
        $line =  $content_data[$machine];
        $line .= (isset($content_data[$machine . '_comments'])) ? ' with ' . $content_data[$machine . '_comments'] : '';
        $line .= (isset($content_data[$machine . '_comments_unapproved'])) ? ' (' . $content_data[$machine . '_comments_unapproved'] . ')' : '';
        $items[] = $line;
      }
    }

    $panel = array(
      '#theme' => 'dashboard_panel__overview_content',
    );
    $panel['list'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
    );
    if (user_access('administer nodes') || user_access('access content overview')) {
      $panel['link'] = array(
        '#theme' => 'link',
        '#path' => 'admin/content',
        '#text' => t('Manage content'),
      );
    }

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $settings = $this->settings;

    $node_types = node_type_get_types();
    $options = array();
    foreach ($node_types as $machine_name => $node_type) {
      $options[$machine_name] = check_plain(t($node_type->name));
    }

    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display <strong>post</strong> counts for the following content types'),
      '#options' => $options,
      '#default_value' => empty($settings['types']) ? array_keys($options) : $settings['types'],
    );

    if (module_exists('comment')) {
      $form['comment'] = array(
        '#type' => 'fieldset',
        '#title' => t('Comment counts'),
        '#tree' => FALSE,
        '#collapsible' => TRUE,
      );
      $form['comment']['comment_enabled'] = array(
        '#type' => 'checkbox',
        '#title' => t('Display comment counts'),
        '#default_value' => $settings['comment_enabled'],
      );
      $form['comment']['comment_include_unapproved'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include unapproved comments in comment counts'),
        '#default_value' => $settings['comment_include_unapproved'],
        '#states' => array(
          'visible' => array(
            '[name="comment_enabled"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['comment']['comment_types'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Display <strong>comment</strong> counts for the following content types'),
        '#options' => $options,
        '#default_value' => empty($settings['comment_types']) ? array_keys($options) : $settings['comment_types'],
        '#states' => array(
          'visible' => array(
            '[name="comment_enabled"]' => array('checked' => TRUE),
          ),
        ),
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['types'] = array_keys(array_filter($form_state['values']['types']));
    $this->settings['comment_enabled'] = (bool) $form_state['values']['comment_enabled'];
    $this->settings['comment_include_unapproved'] = (bool) $form_state['values']['comment_include_unapproved'];
    $this->settings['comment_types'] = array_keys(array_filter($form_state['values']['comment_types']));

    // If all options are selected, save an empty array.
    $type_count = count($form['types']['#options']);
    if ($type_count == count($this->settings['types'])) {
      $this->settings['types'] = array();
    }
    if ($type_count == count($this->settings['comment_types'])) {
      $this->settings['comment_types'] = array();
    }
  }
}
