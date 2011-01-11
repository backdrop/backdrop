<?php

class ViewsUiTaxonomyTermViewsWizard extends ViewsUiBaseViewsWizard {

  protected function default_display_options($form, $form_state) {
    $display_options = array();
    $display_options['access']['type'] = 'perm';
    $display_options['access']['perm'] = 'access content';
    $display_options['cache']['type'] = 'none';
    $display_options['query']['type'] = 'views_query';
    $display_options['exposed_form']['type'] = 'basic';
    $display_options['pager']['type'] = 'full';
    $display_options['style_plugin'] = 'default';
    $display_options['row_plugin'] = 'fields';
    /* Field: Taxonomy: Term */
    $display_options['fields']['name']['id'] = 'name';
    $display_options['fields']['name']['table'] = 'taxonomy_term_data';
    $display_options['fields']['name']['field'] = 'name';
    $display_options['fields']['name']['alter']['alter_text'] = 0;
    $display_options['fields']['name']['alter']['make_link'] = 0;
    $display_options['fields']['name']['alter']['absolute'] = 0;
    $display_options['fields']['name']['alter']['trim'] = 0;
    $display_options['fields']['name']['alter']['word_boundary'] = 0;
    $display_options['fields']['name']['alter']['ellipsis'] = 0;
    $display_options['fields']['name']['alter']['strip_tags'] = 0;
    $display_options['fields']['name']['alter']['html'] = 0;
    $display_options['fields']['name']['hide_empty'] = 0;
    $display_options['fields']['name']['empty_zero'] = 0;
    $display_options['fields']['name']['link_to_taxonomy'] = 1;
    return $display_options;
  }
}
