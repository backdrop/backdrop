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
  var $comments = array();

  /**
   * Stores all nodes of all comments which are preloaded.
   */
  var $nodes = array();

  function summary_title() {
    if (empty($this->options['links'])) {
      return t('Hide links');
    }
    return t('Display links');
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['links'] = array('default' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
  }

  function pre_render($result) {
    $cids = array();

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    // Load all comments.
    $cresult = comment_load_multiple($cids);
    $nids = array();
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

