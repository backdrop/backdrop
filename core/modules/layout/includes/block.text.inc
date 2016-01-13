<?php

/**
 *  TextBlock extends Block
 *  
 *  This class allows us to create text blocks.
 */
class BlockText extends Block {
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Custom block');
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
  }

  /**
   *  Sets block content on block view.
   */
  function getContent() {
    if (!empty($this->settings['content'])) {
      return check_markup(token_replace($this->settings['content']), $this->settings['format'], '', TRUE);
    }
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Display title'),
      '#default_value' => !empty($this->settings['title']) ? $this->settings['title'] : '',
    );
    $form['content'] = array(
      '#type' => 'text_format',
      '#title' => t('Block content'),
      '#default_value' => !empty($this->settings['content']) ? $this->settings['content'] : '',
      '#format' => !empty($this->settings['format']) ? $this->settings['format'] : NULL,
      '#rows' => 5,
    );
  }
  
  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    $this->settings['title'] = $form_state['values']['title'];
    $this->settings['content'] = $form_state['values']['content']['value'];
    $this->settings['format'] = $form_state['values']['content']['format'];
  }
}
