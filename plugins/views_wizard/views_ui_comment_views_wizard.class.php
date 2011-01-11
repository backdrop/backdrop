<?php

class ViewsUiCommentViewsWizard extends ViewsUiBaseViewsWizard {

  protected function default_display_options($form, $form_state) {
    $display_options = array();
    $display_options['access']['type'] = 'perm';
    $display_options['access']['perm'] = 'access comments';
    $display_options['cache']['type'] = 'none';
    $display_options['query']['type'] = 'views_query';
    $display_options['exposed_form']['type'] = 'basic';
    $display_options['pager']['type'] = 'full';
    $display_options['style_plugin'] = 'default';
    $display_options['row_plugin'] = 'fields';
    $display_options['relationships']['nid']['id'] = 'nid';
    $display_options['relationships']['nid']['table'] = 'comment';
    $display_options['relationships']['nid']['field'] = 'nid';
    $display_options['relationships']['nid']['required'] = 1;
    /* Field: Comment: Title */
    $display_options['fields']['subject']['id'] = 'subject';
    $display_options['fields']['subject']['table'] = 'comment';
    $display_options['fields']['subject']['field'] = 'subject';
    $display_options['fields']['subject']['alter']['alter_text'] = 0;
    $display_options['fields']['subject']['alter']['make_link'] = 0;
    $display_options['fields']['subject']['alter']['absolute'] = 0;
    $display_options['fields']['subject']['alter']['trim'] = 0;
    $display_options['fields']['subject']['alter']['word_boundary'] = 0;
    $display_options['fields']['subject']['alter']['ellipsis'] = 0;
    $display_options['fields']['subject']['alter']['strip_tags'] = 0;
    $display_options['fields']['subject']['alter']['html'] = 0;
    $display_options['fields']['subject']['hide_empty'] = 0;
    $display_options['fields']['subject']['empty_zero'] = 0;
    $display_options['fields']['subject']['link_to_comment'] = 1;

    return $display_options;
  }

  /**
   * @override
   */
  protected function page_feed_display_options($form, $form_state) {
    $display_options = array();
    $display_options['defaults']['title'] = FALSE;
    $display_options['title'] = $form_state['values']['page']['title'];
    $display_options['pager']['type'] = 'some';
    $display_options['style_plugin'] = 'rss';
    $display_options['style_options']['mission_description'] = 1;
    $display_options['row_plugin'] = $form_state['values']['page']['feed_properties']['row_plugin'];
    $display_options['path'] = $form_state['values']['page']['feed_properties']['path'];
    return $display_options;
  }
}
