<?php

class ViewsUiCommentViewsWizard extends ViewsUiBaseViewsWizard {

  protected function row_style_options($type) {
    $options = array();
    $options['comment'] = t('Comments');
    $options['fields'] = t('Fields');
    return $options;
  }

  protected function build_form_style(&$form, &$form_state, $type) {
    parent::build_form_style($form, $form_state, $type);
    $style_form =& $form['displays'][$type]['options']['style'];
    $row_style = isset($form_state['values'][$type]['style']['row_style']) ? $form_state['values'][$type]['style']['row_style'] : 'comment';
    switch ($row_style) {
      case 'comment':
        $style_form['row_style_options']['with_links'] = array(
          '#type' => 'select',
          '#title_display' => 'invisible',
          '#title' => t('Should links be displayed below each node'),
          '#options' => array(
            1 => t('with links (allow users to add comments, etc.)'),
            0 => t('without links'),
          ),
          '#default_value' => 1,
        );
        break;
    }
  }

  protected function default_display_options($form, $form_state) {
    $display_options = parent::default_display_options($form, $form_state);

    // Add permission-based access control.
    $display_options['access']['type'] = 'perm';

    // Add a relationship to nodes.
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
}
