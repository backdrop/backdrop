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

    $this->settings += array(
      'title_tag' => 'h1',
      'title_class' => 'page-title',
      'tab_type' => 'both'
    );
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    $children = $this->getChildren();
    $title = $children[$this->childDelta]['info'];
    return strlen($this->settings['title']) ? check_plain($this->settings['title']) : $title;
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
      $form['title_wrapper'] = array(
        '#type' => 'fieldset',
        '#title' => t('Page title'),
      );
      $form['title_wrapper']['title_tag'] = array(
        '#title' => t('Title tag'),
        '#type' => 'select',
        '#options' => array(
          'h1' => 'H1',
          'h2' => 'H2',
          'h3' => 'H3',
          'h4' => 'H4',
          'h5' => 'H5',
          'h6' => 'H6',
          'div' => 'DIV',
        ),
        '#default_value' => $this->settings['title_tag'],
      );
      $form['title_wrapper']['title_classes'] = array(
        '#title' => t('Heading classes'),
        '#type' => 'textfield',
        '#default_value' => $this->settings['title_classes'],
        '#description' => t('Additional classes to be added to the page title.'),
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
          'hidden' => t('Hidden'),
        ),
        '#default_value' => $this->settings['tab_type'],
        '#description' => t('Tabs are commonly used to show links for editing content, viewing revisions, and accessing sub-pages of configuration.
'),
      );
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    if (isset($form_state['values']['title_wrapper']['title_tag'])) {
      $this->settings['title_tag'] = $form_state['values']['title_wrapper']['title_tag'];
    }
    if (isset($form_state['values']['title_wrapper']['title_classes'])) {
      $this->settings['title_classes'] = $form_state['values']['title_wrapper']['title_classes'];
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
        'info' => t('Tabs (Local tasks)'),
        'description' => t('Displays the links for menu local tasks, such as "View" or "Edit"'),
      ),
      'title' => array(
        'info' => t('Page title'),
        'description' => t('Displays the page title.'),
      ),
      'action_links' => array(
        'info' => t('Local actions'),
        'description' => t('Displays the local actions links, such as "Add content".'),
      ),
      'messages' => array(
        'info' => t('System messages'),
        'description' => t('Displays messages to the user after an action is performed, such as form validation errors or success messages.'),
      ),
    );
  }
}
