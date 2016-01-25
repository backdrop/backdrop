<?php
/**
 * @file
 * Class providing forms and settings for menu block styles.
 */
abstract class MenuStyle {

  /**
   * Specifies the settings form for configuring the style.
   */
  function form($config) {}

  /**
   * Build settings.
   */
  function buildSettings($settings = array()) {}
}
