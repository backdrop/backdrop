<?php
/**
 * @file
 * Contains the node RSS row style plugin.
 */

/**
 * Plugin which performs a comment_view on the resulting object.
 */
class views_plugin_row_comment_view extends views_plugin_row {
  var $base_table = 'comment';
  var $base_field = 'cid';

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
    $this->comments = array();

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    $cresult = comment_load_multiple($cids);
    foreach ($cresult as $comment) {
      $comment->depth = count(explode('.', $comment->thread)) - 1;
      $this->comments[$comment->cid] = $comment;
    }
  }
}

