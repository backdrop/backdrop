<?php
/**
 * @file
 * Handler for node path field.
 */

/**
 * Field handler to present the path to the node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_path extends views_handler_field {

  /**
   * Default value for the path field option.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['absolute'] = array('default' => FALSE, 'bool' => TRUE);

    return $options;
  }

  /**
   * Override construct function to provide the langcode additional field.
   */
  public function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
    if (module_exists('translation')) {
      $this->additional_fields['langcode'] = array(
        'table' => 'node',
        'field' => 'langcode',
      );
    }
  }

  /**
   * Provide additional options for the path field.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['absolute'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use absolute link (begins with "http://")'),
      '#default_value' => $this->options['absolute'],
      '#description' => t('Enable this option to output an absolute link. Required if you want to use the path as a link destination (as in "output this field as a link" above).'),
      '#fieldset' => 'alter',
    );
  }

  /**
   * Provide necessary query segments.
   */
  public function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  /**
   * Render the path field.
   */
  public function render($values) {
    $nid = $this->get_value($values, 'nid');
    $options = array();
    $options['absolute'] = $this->options['absolute'];
    if (isset($this->aliases['langcode'])) {
      $languages = language_list();
      $language = $this->get_value($values, 'langcode');
      if (isset($languages[$language])) {
        $options['language'] = $languages[$language];
      }
    }
    return url("node/$nid", $options);
  }
}
