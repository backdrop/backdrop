<?php
/**
 * @file
 * Contains the list style plugin.
 */

/**
 * Style plugin to render each item in an ordered or unordered list.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_list extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['type'] = ['default' => 'ul'];
    $options['class'] = ['default' => ''];
    $options['wrapper_class'] = ['default' => 'item-list'];

    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['type'] = [
      '#type' => 'radios',
      '#title' => t('List type'),
      '#options' => ['ul' => t('Unordered list'), 'ol' => t('Ordered list')],
      '#default_value' => $this->options['type'],
    ];
    $form['wrapper_class'] = [
      '#title' => t('Wrapper class'),
      '#description' => t('The class to provide on the wrapper, outside the list.'),
      '#type' => 'textfield',
      '#size' => '30',
      '#default_value' => $this->options['wrapper_class'],
    ];
    $form['class'] = [
      '#title' => t('List class'),
      '#description' => t('The class to provide on the list element itself.'),
      '#type' => 'textfield',
      '#size' => '30',
      '#default_value' => $this->options['class'],
    ];
  }
}
