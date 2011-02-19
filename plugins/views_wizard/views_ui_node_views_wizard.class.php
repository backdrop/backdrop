<?php

class ViewsUiNodeViewsWizard extends ViewsUiBaseViewsWizard {

  protected function row_style_options($type) {
    $options = array();
    $options['full_posts'] = t('full posts');
    $options['teasers'] = t('teasers');
    $options['titles_linked'] = t('titles linked');
    $options['titles'] = t('titles');
    $options['fields'] = t('Fields');
    return $options;
  }

  protected function build_form_style(&$form, &$form_state, $type) {
    parent::build_form_style($form, $form_state, $type);
    $style_form =& $form['displays'][$type]['options']['style'];
    $row_plugin = isset($form_state['values'][$type]['style']['row_plugin']) ? $form_state['values'][$type]['style']['row_plugin'] : 'full_posts';
    // Some style plugins doesn't support row plugins so stop here and don't add some row plugins.
    if (!isset($style_form['row_plugin'])) {
      return;
    }
    switch ($row_plugin) {
      case 'full_posts':
      case 'teasers':
        $style_form['row_options']['links'] = array(
          '#type' => 'select',
          '#title_display' => 'invisible',
          '#title' => t('Should links be displayed below each node'),
          '#options' => array(
            1 => t('with links (allow users to add comments, etc.)'),
            0 => t('without links'),
          ),
          '#default_value' => 1,
        );
        $style_form['row_options']['comments'] = array(
          '#type' => 'select',
          '#title_display' => 'invisible',
          '#title' => t('Should comments be displayed below each node'),
          '#options' => array(
            1 => t('with comments'),
            0 => t('without comments'),
          ),
          '#default_value' => 1,
        );
        break;
    }
  }

  /**
   * @override
   */
  protected function default_display_options($form, $form_state) {
    $display_options = parent::default_display_options($form, $form_state);

    // Add permission-based access control.
    $display_options['access']['type'] = 'perm';

    // Add the title field, so that the display has content if the user switches
    // to a row style that uses fields.
    /* Field: Content: Title */
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

  protected function page_display_options($form, $form_state) {
    $display_options = parent::page_display_options($form, $form_state);
    $row_plugin = $form_state['values']['page']['style']['row_plugin'];
    $row_options = isset($form_state['values']['page']['style']['row_options']) ? $form_state['values']['page']['style']['row_options'] : array();
    $this->display_options_row($display_options, $row_plugin, $row_options);
    return $display_options;
  }

  protected function block_display_options($form, $form_state) {
    $display_options = parent::block_display_options($form, $form_state);
    $row_plugin = $form_state['values']['block']['style']['row_plugin'];
    $row_options = isset($form_state['values']['block']['style']['row_options']) ? $form_state['values']['block']['style']['row_options'] : array();
    $this->display_options_row($display_options, $row_plugin, $row_options);
    return $display_options;
  }

  /**
   * Set the row style and row style plugins to the display_options.
   */
  protected  function display_options_row(&$display_options, $row_plugin, $row_options) {
    switch ($row_plugin) {
      case 'full_posts':
        $display_options['row_plugin'] = 'node';
        $display_options['row_options']['build_mode'] = 'full';
        $display_options['row_options']['links'] = !empty($row_options['links']);
        $display_options['row_options']['comments'] = !empty($row_options['comments']);
        break;
      case 'teasers':
        $display_options['row_plugin'] = 'node';
        $display_options['row_options']['build_mode'] = 'teaser';
          $display_options['row_options']['links'] = !empty($row_options['links']);
        $display_options['row_options']['comments'] = !empty($row_options['comments']);
        break;
      case 'titles_linked':
        $display_options['row_plugin'] = 'fields';
        $display_options['field']['title']['link_to_node'] = 1;
        break;
      case 'titles':
        $display_options['row_plugin'] = 'fields';
        $display_options['field']['title']['link_to_node'] = 0;
        break;
    }
  }
}
