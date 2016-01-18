<?php
/**
 * @file
 * Class providing forms and settings for menu block styles.
 */
abstract class MenuStyle {
  /**
   * Indicator as to whether this is a style on the region or on a block.
   *
   * @var boolean
   */
  var $is_region = FALSE;

  /**
   * Constructor for a Layout style class.
   *
   * @param array $plugin_name
   *   The name of the plugin actually being used, which may be pointing at
   *   a parent class of this one.
   * @param array $data
   *   An array of configuration data.
   */
  function __construct($config) {}

  /**
   * Specifies the settings form for configuring the style.
   */
  function form($config) {}

  /**
   * Validate handler to validate the form settings.
   */
  function formValidate($form, $form_state) {}

  /**
   * Submit handler for saving the form settings.
   */
  function formSubmit($form, $form_state) {}
}
