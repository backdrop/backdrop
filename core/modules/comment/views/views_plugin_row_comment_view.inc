<?php
/**
 * @file
 * Contains the node RSS row style plugin.
 */

/**
 * Plugin which performs a comment_view on the resulting object.
 */
class views_plugin_row_comment_view extends views_plugin_row {
  var $base_field = 'cid';
  var $base_table = 'comment';

  /**
   * Stores all comments which are preloaded.
   */
  var $comments = [];

  /**
   * Stores all nodes of all comments which are preloaded.
   */
  var $nodes = [];

  function summary_title() {
    return t('Settings');
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['links'] = ['default' => TRUE, 'bool' => TRUE];
    $options['view_mode'] = ['default' => 'full'];
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $options = $this->options_form_summary_options();
    $form['view_mode'] = [
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Display mode'),
      '#default_value' => $this->options['view_mode'],
     ];

    $form['links'] = [
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    ];
  }


  /**
   * Return the main options, which are shown in the summary title.
   */
  function options_form_summary_options() {
    $entity_info = entity_get_info('comment');
    $options = [];
    if (!empty($entity_info['view modes'])) {
      foreach ($entity_info['view modes'] as $mode => $settings) {
        $options[$mode] = $settings['label'];
      }
    }
    if (empty($options)) {
      $options = [
        'full' => t('Full content')
      ];
    }

    return $options;
  }

  function pre_render($result) {
    $cids = [];

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    // Load all comments.
    $cresult = comment_load_multiple($cids);
    $nids = [];
    foreach ($cresult as $comment) {
      $comment->depth = count(explode('.', $comment->thread)) - 1;
      $this->comments[$comment->cid] = $comment;
      $nids[] = $comment->nid;
    }

    // Load all nodes of the comments.
    $nodes = node_load_multiple(array_unique($nids));
    foreach ($nodes as $node) {
      $this->nodes[$node->nid] = $node;
    }
  }
}
