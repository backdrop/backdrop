<?php
/**
 * @file
 * Class providing forms and settings for menu block styles.
 */
class MenuStyleSimple extends MenuStyle {
  /**
   * Constructor for a Layout style class.
   *
   * @param array $plugin_name
   *   The name of the plugin actually being used, which may be pointing at
   *   a parent class of this one.
   * @param array $data
   *   An array of configuration data.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
  }

  /**
   * Specifies the settings form for configuring the style.
   */
  function form($settings) {
    //parent::form($form, $form_state);
    $config = config('system.menu_block_style_simple');
    $form['absolute'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use absolute positioning?'),
      '#default_value' => isset($settings['absolute']) ? $settings['absolute'] : 1,
      '#description' => t('Using absolute, the menu will open over the page rather than pushing it down.'),
    );
    $form['disable_mouse_events'] = array(
      '#type' => 'checkbox',
      '#title' => t('Disable other mouse events?'),
      '#default_value' => isset($settings['disable_mouse_events']) ? $settings['disable_mouse_events'] : 0,
      '#description' => t('Disable things like drop-down menus on hover.'),
    );
    $form['remove_attributes'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remove other classes & IDs when responded?'),
      '#default_value' => isset($settings['remove_attributes']) ? $settings['remove_attributes'] : 1,
      '#description' => t('Helps to ensure styling of menu.'),
    );
    $form['selectors'] = array(
      '#type' => 'textarea',
      '#title' => t('Selectors for which menus to responsify'),
      '#default_value' => isset($settings['selectors']) ? $settings['selectors'] : '#main-menu',
      '#description' => t('Enter CSS/jQuery selectors of menus to responsify.  Comma separated or 1 per line'),
    );
    $form['toggler_text'] = array(
      '#type' => 'textarea',
      '#title' => t('Text or HTML for the menu toggle button'),
      '#default_value' => isset($settings['toggler_text']) ? $settings['toggler_text'] : '? Menu',
    );
    $form['media_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Screen width to respond to'),
      '#size' => 5,
      '#default_value' => isset($settings['media_size']) ? $settings['media_size'] : 768,
      '#description' => t('Width when we swap out responsive menu e.g. 768'),
    );
    $form['media_unit'] = array(
      '#type' => 'select',
      '#title' => t('Width unit'),
      '#default_value' => isset($settings['media_unit']) ? $settings['media_unit'] : 'px',
      '#options' => array('px' => 'px', 'em' => 'em'),
      '#description' => t('Unit for the width above'),
    );
    
    return $form;
  }

  /**
   * Validate handler to validate the form settings.
   */
  function formValidate($form, $form_state) {}

  /**
   * Submit handler for saving the form settings.
   */
  function formSubmit($form, $form_state) {
    $this->settings = $form_state['values']['style_settings'];
  }
}
