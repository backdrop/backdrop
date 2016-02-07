<?php

/**
 *  NodeBlock  extends Block
 *  
 *  This class allows us to display a particular node in a block.
 */
class NodeBlock extends Block {
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return t('Content block');
  }

  /**
   * Return a preview for this block.
   */
  function getAdminPreview() {
    return t('Displays content of node ID: @nid', array('@nid' => $this->settings['nid']));
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    $title = NULL;
    $node = node_load($this->settings['nid']);
    if ($this->settings['title_display'] === LAYOUT_TITLE_CUSTOM && $this->settings['title']) {
      $title = check_plain($this->settings['title']);
    }
    elseif ($this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      $title = $node->title;
    }

    if ($title && $this->settings['link_node_title']) {
      $title = l($title, 'node/' . $node->nid);
    }

    return $title;
  }

  /**
   *  Sets block content on block view.
   */
  function getContent() {
    $node = node_load($this->settings['nid']);

    // Prevent display of no access to the node.
    if (!node_access('view', $node)) {
      return;
    }

    // Use a clone of the node so we can hide the title safely.
    $clone = clone($node);

    // Hide node title if not required.
    if (empty($this->settings['leave_node_title'])) {
      $clone->title = '';
    }

    $content = node_view($clone, $this->settings['view_mode']);

    // Hide links if they've been suppressed.
    if (empty($this->settings['links'])) {
      $content['links']['#access'] = FALSE;
    }

    // Add 'submitted by' settings.
    if (!empty($this->settings['display_submitted'])) {
      $content['#display_submitted'] = TRUE;
    }

    // Add theme hook suggestion.
    $content['#theme'] = 'node__content-block-' . $this->settings['nid'];

    return $content;
  }

  /**
   *  Builds the block's settings configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['title_display']['title_display']['#options'][LAYOUT_TITLE_DEFAULT] = t('Use node title');
    $form['title_display']['title_display']['#weight'] = -50;
    $settings = $this->settings;

    $form['nid'] = array(
      '#title' => t('Enter the title or NID of a node'),
      '#type' => 'textfield',
      '#maxlength' => 512,
      '#default_value' => !empty($settings['nid']) ? $settings['nid'] : NULL,
      '#autocomplete_path' => 'node/autocomplete',
    );

    $form['display_submitted'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['display_submitted']),
      '#title' => t('Show author and date information'),
      '#description' => t('Advanced: if checked, display the node author and date information.'),
    );

    $form['leave_node_title'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['leave_node_title']),
      '#title' => t('Show node title in block content'),
      '#description' => t('Advanced: if checked, display the node title; this can cause the node title to appear twice unless you select "none" for the block title type.'),
    );

    $form['link_node_title'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['link_node_title']),
      '#title' => t('Link the block title to the node'),
      '#description' => t('Check this box if you would like the block title to link to the node.'),
    );

    $form['links'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['links']),
      '#title' => t('Include node links for "add comment", "read more" etc.'),
    );

    $entity = entity_get_info('node');
    $view_mode_options = array();
    foreach ($entity['view modes'] as $mode => $option) {
      $view_mode_options[$mode] = $option['label'];
    }

    $form['view_mode'] = array(
      '#title' => t('View mode'),
      '#type' => 'select',
      '#description' => t('Select a view mode for this node.'),
      '#options' => $view_mode_options,
      '#default_value' => isset($settings['view_mode']) ? $settings['view_mode'] : 'full',
    );
  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    module_load_include('inc', 'node', 'node.pages');
    if ($nid = node_autocomplete_validate($form_state['values']['nid'])) {
      $form_state['values']['nid'] = $nid;
    }
    else {
      form_error($form['nid'], t('Sorry, no node matching <em>%title</em> was found.', array('%title' => $form_state['values']['nid'])));
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    foreach (array('nid', 'links', 'leave_node_title', 'link_node_title', 'view_mode', 'display_submitted') as $key) {
      $this->settings[$key] = $form_state['values'][$key];
    }
  }
}
