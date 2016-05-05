<?php
/**
 * @file
 * Class providing forms and settings for the Layout region style.
 */
class LayoutStyleRegion extends LayoutStyle {
  /**
   * Indicator as to whether this is a style on the region or on a block.
   *
   * @var boolean
   */
  var $is_region = TRUE;

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

    $this->settings['element'] = isset($data['layout']->settings['style_settings'][$data['region']]['element']) ? $data['layout']->settings['style_settings'][$data['region']]['element'] : '';
    $this->settings['classes'] = isset($data['layout']->settings['style_settings'][$data['region']]['classes']) ? $data['layout']->settings['style_settings'][$data['region']]['classes'] : '';
    $this->is_region = TRUE;
  }

  /**
   * Specifies the settings form for configuring the style.
   */
  function form(&$form, &$form_state) {
    if ($this->is_region) {
      $options = array(
        '' => 'No inner wrapper',
        'div' => 'DIV',
        'aside' => 'ASIDE',
        'section' => 'SECTION',
      );
      $form['element'] = array(
        '#title' => t('HTML element for inner wrapper'),
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => $this->settings['element'],
      );
      $form['classes'] = array(
        '#title' => t('CSS classes for inner wrapper'),
        '#type' => 'textfield',
        '#default_value' => $this->settings['classes'],
        '#description' => t('Separate multiple classes with spaces.'),
        '#states' => array(
          'invisible' => array(
            ':input[name="style_settings[element]"]' => array('value' => ''),
          ),
        ),
      );
    }
  }

  /**
   * Validate handler to validate the form settings.
   */
  function formValidate($form, $form_state) {
    // No additional validation needed yet.
  }

  /**
   * Submit handler for saving the form settings.
   */
  function formSubmit($form, $form_state) {
    $region_name = $form_state['region_name'];
    $form_state['layout']->settings['style_settings'][$region_name]['element'] = $form_state['values']['style_settings']['element'];
    $form_state['layout']->settings['style_settings'][$region_name]['classes'] = $form_state['values']['style_settings']['classes'];
    $form_state['layout']->settings[$region_name]['style'] = $form_state['values']['style'];
  }
}
