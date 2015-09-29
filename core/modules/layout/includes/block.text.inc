<?php

/**
 *  TextBlock extends Block
 *  
 *  This class allows us to create text blocks.
 */
class TextBlock extends Block {
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return t('Text block');
  }

  /**
   *  Sets description text on draggable block in Layout builder.
   */
  function getAdminPreview() {
    $label_text = !empty($this->settings['subject']) ? ': ' . check_plain($this->settings['subject']) : '';
    $preview = '<p>' . t('Text') . $label_text . '</p>';
    $preview .= $this->getAdminConditionsPreview();

    return $preview;
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['subject']) ? check_plain($this->settings['subject']) : '';
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
    $form['subject'] = array(
      '#type' => 'textfield',
      '#title' => t('Display title'),
      '#default_value' => !empty($this->settings['subject']) ? $this->settings['subject'] : '',
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
    $this->settings['subject'] = $form_state['values']['subject'];
    $this->settings['content'] = $form_state['values']['content']['value'];
    $this->settings['format'] = $form_state['values']['content']['format'];
  }
}
