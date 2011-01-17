<?php

class ViewsUiNodeViewsWizard extends ViewsUiBaseViewsWizard {

  /**
   * @override
   */
  protected function default_display_options($form, $form_state) {
    $display_options = array();
    $display_options['access']['type'] = 'perm';
    $display_options['access']['perm'] = 'access content';
    $display_options['cache']['type'] = 'none';
    $display_options['query']['type'] = 'views_query';
    $display_options['exposed_form']['type'] = 'basic';
    $display_options['pager']['type'] = 'full';
    $display_options['style_plugin'] = 'default';
    $display_options['row_plugin'] = 'node';
    $display_options['row_options']['links'] = 1;
    return $display_options;
  }
}
