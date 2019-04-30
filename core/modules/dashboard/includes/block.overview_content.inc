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

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $all_types = array_keys(node_type_get_names());
    $this->settings += array(
      'spam' => TRUE,
      'types' => $all_types,
      'comments' => $all_types,
    );
  }

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
    $settings = $this->settings;

    // Not cached since comment and node CRUD operations are likely to be more
    // frequent than Dashboard access events.
    $items = array();
    $types = node_type_get_types();
    $comments_exist = module_exists('comment');
    foreach ($types as $type => $object) {
      // Compare against type option on block settings.
      if (in_array($type, $settings['types']) || empty($settings['types'])) {
        $type_count = db_query("SELECT count(*) FROM {node} WHERE type = :type and status = 1", array(':type' => $type))->fetchField();
        $content_data[$type] = format_plural($type_count, '1 @type item', '@count @type items', array('@type' => $object->name));
        // Check if comments module is enabled.
        if ($comments_exist) {
          // Compare against comment options on block settings.
          if (in_array($type, $settings['comments'])) {
            $comment_count = db_query("SELECT count(DISTINCT cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 1 AND n.status = 1", array(':type' => $type))->fetchField();
            $content_data[$type . '_comments'] =  format_plural($comment_count, '1 comment', '@count comments');
            // Compare against spam option checkbox on block settings.
            if (isset($settings['spam']) && $settings['spam'] == 1) {
              $spam_count = db_query("SELECT count(DISTINCT c.cid) FROM {comment} c INNER JOIN {node} n ON c.nid = n.nid WHERE n.type = :type and c.status = 0 AND n.status = 1", array(':type' => $type))->fetchField();
              $content_data[$type . '_comments_spam'] = format_plural($spam_count, '1 spam', '@count spam');
            }
          }
        }
        $line =  $content_data[$type];
        $line .= (isset($content_data[$type . '_comments'])) ? ' with ' . $content_data[$type . '_comments'] : '';
        $line .= (isset($content_data[$type . '_comments_spam'])) ? ' (' . $content_data[$type . '_comments_spam'] . ')' : '';
        $items[] = $line;
      }
    }
    return theme('dashboard_overview_content', array('overview' => $items));
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $types = node_type_get_types();
    $type_options = array();
    $type_defaults = array();
    $comment_defaults = array();
    // Set defaults based on block settings.
    if (isset($settings['types'])) {
      $type_defaults = $settings['types'];
    }
    if (isset($settings['comments'])) {
      $comment_defaults = $settings['comments'];
    }
    foreach ($types as $machine_type => $type) {
      $type_options[$machine_type] = $type->name;
      // Display new content types by default.
      if (!array_key_exists($machine_type, $type_defaults)) {
        $type_defaults[$machine_type] = $machine_type;
      }
      // Do not display comments on new types by default unless it's a blog or a forum.
      if (!array_key_exists($machine_type, $comment_defaults)) {
        if ($machine_type == 'blog' || $machine_type == 'forum topic') {
          $comment_defaults[$machine_type] = $machine_type;
        }
        else {
          $comment_defaults[$machine_type] = 0;
        }
      }
    }
    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Show <strong>post</strong> counts for the following content types'),
      '#options' => $type_options,
      '#default_value' => $type_defaults,
    );
    if (module_exists('comment')) {
      $form['comments'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Show <strong>comment</strong> counts for the following content types'),
        '#options' => $type_options,
        '#default_value' => $comment_defaults,
      );
      $form['spam'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include spam counts with comments'),
        '#default_value' => $settings['spam'],
      );
    }
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['spam'] = $form_state['values']['spam'];
    $this->settings['types'] = array_keys(array_filter($form_state['values']['types']));
    $this->settings['comments'] = array_keys(array_filter($form_state['values']['comments']));
  }
}
