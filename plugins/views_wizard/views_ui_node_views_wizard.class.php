<?php

class ViewsUiNodeViewsWizard extends ViewsUiBaseViewsWizard {

  /**
   * @override
   */
  protected function default_display_options($from, $form_state) {
    $display_options = array();
    $display_options['access']['type'] = 'none';
    $display_options['cache']['type'] = 'none';
    $display_options['query']['type'] = 'views_query';
    $display_options['exposed_form']['type'] = 'basic';
    $display_options['pager']['type'] = 'full';
    $display_options['style_plugin'] = 'default';
    $display_options['row_plugin'] = 'node';
    $display_options['row_options']['links'] = 1;
    return $display_options;
  }

  /**
   * @override
   */
  protected function page_feed_display_options($from, $form_state) {
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
