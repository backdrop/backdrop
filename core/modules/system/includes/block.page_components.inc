<?php

/**
 * PageComponents extends Block
 *
 * This class allows us to display the page title, local task tabs, 
 * local actions links, and messages in a block.
 */
class PageComponents extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    $children = $this->getChildren();
    $title = $children[$this->childDelta]['info'];
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : $title;
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  function getContent() {
    return theme('page_components', array('child_delta' => $this->childDelta, 'settings' => $this->settings));
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    if ($this->childDelta == 'title_combo' || $this->childDelta == 'title') {
      $form['title'] = array(
        '#type' => 'fieldset',
        '#title' => t('Page title'),
      );
      $form['title']['title_markup'] = array(
        '#title' => t('Title tag'),
        '#type' => 'select',
        '#options' => array(
          'none' => t('- No tag -'),
          'h1' => t('h1'),
          'h2' => t('h2'),
          'h3' => t('h3'),
          'h4' => t('h4'),
          'h5' => t('h5'),
          'h6' => t('h6'),
          'div' => t('div'),
        ),
        '#default_value' => empty($this->settings['title_markup']) ? 'h1' : $this->settings['title_markup'],
      );
      $form['title']['title_class'] = array(
        '#title' => t('CSS class to use'),
        '#type' => 'textfield',
        '#default_value' => empty($this->settings['title_class']) ? '' : $this->settings['title_class'],
      );
    }

    if ($this->childDelta == 'title_combo' || $this->childDelta == 'tabs') {
      $form['tabs'] = array(
        '#type' => 'fieldset',
        '#title' => t('Local task tabs'),
      );
      $form['tabs']['tab_type'] = array(
        '#title' => t('Tabs type'),
        '#type' => 'select',
        '#options' => array(
          'both' => t('Primary and secondary'),
          'primary' => t('Primary'),
          'secondary' => t('Secondary'),
        ),
        '#default_value' => $this->settings['tab_type'],
      );
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    if (isset($form_state['values']['title']['title_markup'])) {
      $this->settings['title_markup'] = $form_state['values']['title']['title_markup'];
    }
    if (isset($form_state['values']['title']['title_class'])) {
      $this->settings['title_class'] = $form_state['values']['title']['title_class'];
    }
    if (isset($form_state['values']['tabs']['tab_type'])) {
      $this->settings['tab_type'] = $form_state['values']['tabs']['tab_type'];
    }
  }

  /**
   * Creates an array of "child blocks" for display in the "Add Block" dialog.
   *
   * This allows a single entry in hook_block_info() to provide multiple
   * block instances.
   */
  function getChildren() {
    return array(
      'title_combo' => array(
        'info' => t('Title combo'),
        'description' => t('Displays the page title, local task tabs, local actions links, and messages.'),
      ),
      'tabs' => array(
        'info' => t('Local task tabs'),
        'description' => t('Displays the local task tabs.'),
      ),
      'title' => array(
        'info' => t('Page title'),
        'description' => t('Displays the page title.'),
      ),
      'action_links' => array(
        'info' => t('Local actions'),
        'description' => t('Displays the local actions links.'),
      ),
      'messages' => array(
        'info' => t('System messages'),
        'description' => t('Displays the system messages.'),
      ),
    );
  }

}
