<?php
/**
 * @file
 * Contains the comment RSS row style plugin.
 */

/**
 * Plugin which formats the comments as RSS items.
 */
class views_plugin_row_comment_rss extends views_plugin_row {
   var $base_table = 'comment';
   var $base_field = 'cid';

  function pre_render($result) {
    $cids = array();
    $nids = array();

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    $this->comments = comment_load_multiple($cids);
    foreach ($this->comments as &$comment) {
      $comment->depth = count(explode('.', $comment->thread)) - 1;
      $nids[] = $comment->nid;
    }

    $this->nodes = node_load_multiple($nids);
  }

  function render($row) {
    global $base_url;

    $cid = $row->{$this->field_alias};
    if (!is_numeric($cid)) {
      return;
    }

    $item_length = $this->options['item_length'];
    if ($item_length == 'default') {
      $item_length = variable_get('feed_item_length', 'teaser');
    }

    // Load the specified comment and its associated node:
    $comment = $this->comments[$cid];
    if (empty($comment) || empty($this->nodes[$comment->nid])) {
      return;
    }

    $item_text = '';

    $uri = entity_uri('comment', $comment);
    $comment->link = url($uri['path'], $uri['options'] + array('absolute' => TRUE));
    $comment->rss_namespaces = array();
    $comment->rss_elements = array(
      array(
        'key' => 'pubDate',
        'value' => gmdate('r', $comment->created),
      ),
      array(
        'key' => 'dc:creator',
        'value' => $comment->name,
      ),
      array(
        'key' => 'guid',
        'value' => 'comment ' . $comment->cid . ' at ' . $base_url,
        'attributes' => array('isPermaLink' => 'false'),
      ),
    );

    // The comment gets built and modules add to or modify
    // $comment->rss_elements and $comment->rss_namespaces.
    $build = comment_view($comment, $this->nodes[$comment->cid], 'rss');
    unset($build['#theme']);

    if (!empty($comment->rss_namespaces)) {
      $this->view->style_plugin->namespaces = array_merge($this->view->style_plugin->namespaces, $comment->rss_namespaces);
    }

    // Hide the links if desired.
    if (!$this->options['links']) {
      hide($build['links']);
    }

    if ($item_length != 'title') {
      // We render comment contents and force links to be last.
      $build['links']['#weight'] = 1000;
      $item_text .= drupal_render($build);
    }

    $item = new stdClass();
    $item->description = $item_text;
    $item->title = $comment->subject;
    $item->link = $comment->link;
    $item->elements = $comment->rss_elements;
    $item->cid = $comment->cid;

    return theme($this->theme_functions(), array(
      'view' => $this->view,
      'options' => $this->options,
      'row' => $item
    ));
  }
}
