<?php
/**
 * @file
 * Definition of views_handler_field_system_info.
 */

/**
 * A handler to display module and theme settings stored in the info column.
 *
 * Provides access to several settings stored as a serialized array in the info
 * column of the system table.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_system_info extends views_handler_field {
  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['info_setting'] = array('default' => '');
    $options['custom_label'] = array('default' => 'default');
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['custom_label']['#type'] = 'radios';
    $form['custom_label']['#options'] = array(
      'default' => t('Use setting label'),
      'custom' => t('Custom label'),
      'none' => t('No label'),
    );
    unset($form['custom_label']['#description']);

    $form['custom_label']['#default_value'] = $this->options['custom_label'];
    $form['label']['#states'] = array(
      'visible' => array(
        ':input[name="options[custom_label]"]' => array('value' => 'custom')
      ),
    );
    $form['label']['#title'] = t('Custom label');

    $form['info_setting'] = array(
      '#type' => 'select',
      '#title' => 'Field',
      '#options' => $this->_getSettingOptions(),
      '#default_value' => isset($this->options['info_setting']) ? $this->options['info_setting'] : '',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $value = $this->get_value($values);
    $info_array = unserialize($value);
    $info_setting = $this->options['info_setting'];
    if (!isset($info_array[$info_setting])) {
      return '';
    }
    switch ($info_setting) {
      case 'tags':
      case 'dependencies':
        return implode(', ', $info_array[$info_setting]);
      default:
        return $info_array[$info_setting];
    } 
  }

  /**
   * * Get this field's label based on the selected setting.
   */
  public function label() {
    if ($this->options['custom_label'] === 'default') {
      $setting_options = $this->_getSettingOptions();
      $label = !empty($setting_options[$this->options['info_setting']]) ? $setting_options[$this->options['info_setting']] : '';
      return $label;
    }
    elseif ($this->options['custom_label'] === 'custom' && isset($this->options['label'])) {
      return $this->options['label'];
    }
    return '';
  }

  /**
   * Helper function to return the option array.
   */
  protected function _getSettingOptions() {
    return array(
      'name' => t('Name'),
      'description' => t('Description'),
      'package' => t('Package'),
      'configure' => t('Configuration path'),
      'tags' => t('Tags'),
      'version' => t('Version'),
      'backdrop' => t('Backdrop version'),
      'dependencies' => t('Dependencies'),
    );
  }
}
