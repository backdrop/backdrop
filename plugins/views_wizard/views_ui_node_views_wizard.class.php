<?php

class ViewsUiNodeViewsWizard extends ViewsUiBaseViewsWizard {

  /**
   * @override
   */
  protected function default_display_options($form, $form_state) {
    $display_options = parent::default_display_options($form, $form_state);

    // Add permission-based access control.
    $display_options['access']['type'] = 'perm';

    // Display full nodes by default, rather than fields.
    $display_options['row_plugin'] = 'node';
    $display_options['row_options']['links'] = 1;

    // Add the title field, so that the display has content if the user switches
    // to a row style that uses fields.
    /* Field: Node: Title */
    $display_options['fields']['title']['id'] = 'title';
    $display_options['fields']['title']['table'] = 'node';
    $display_options['fields']['title']['field'] = 'title';
    $display_options['fields']['title']['alter']['alter_text'] = 0;
    $display_options['fields']['title']['alter']['make_link'] = 0;
    $display_options['fields']['title']['alter']['absolute'] = 0;
    $display_options['fields']['title']['alter']['trim'] = 0;
    $display_options['fields']['title']['alter']['word_boundary'] = 0;
    $display_options['fields']['title']['alter']['ellipsis'] = 0;
    $display_options['fields']['title']['alter']['strip_tags'] = 0;
    $display_options['fields']['title']['alter']['html'] = 0;
    $display_options['fields']['title']['hide_empty'] = 0;
    $display_options['fields']['title']['empty_zero'] = 0;
    $display_options['fields']['title']['link_to_node'] = 1;

    return $display_options;
  }
}
