<?php
/**
 * @file
 * Contains the grid style plugin.
 */

/**
 * Style plugin to render each item in a grid cell.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_grid_responsive extends views_plugin_style {
  /**
   * Set default options
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['columns'] = array('default' => '4');
    $options['alignment'] = array('default' => 'horizontal');
    $options['cell_min_width'] = ['default' => '100'];
    $options['grid_gutter'] = ['default' => '10'];

    return $options;
  }

  /**
   * Render the given style.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['columns'] = array(
      '#type' => 'number',
      '#title' => t('Maximum number of columns'),
      '#default_value' => $this->options['columns'],
      '#required' => TRUE,
      '#min' => 1,
      '#step' => 1,
      '#weight' => -2,
    );
    $form['cell_min_width'] = array(
      '#type' => 'number',
      '#title' => t('Minimum grid cell width'),
      '#field_suffix' => 'px',
      '#attributes' => array('style' => 'width: 6em;'),
      '#description' => t('The minimum width of the grid cells. If the grid container becomes narrow, the grid cells will reflow onto the next row as needed.'),
      '#default_value' => $this->options['cell_min_width'],
      '#required' => TRUE,
      '#min' => 1,
    );
    $form['grid_gutter'] = array(
      '#type' => 'number',
      '#title' => t('Grid gutter spacing'),
      '#field_suffix' => 'px',
      '#attributes' => array('style' => 'width: 6em;'),
      '#description' => t('The spacing between the grid cells.'),
      '#default_value' => $this->options['grid_gutter'],
      '#required' => TRUE,
      '#min' => 0,
    );
    $form['alignment'] = array(
      '#type' => 'radios',
      '#title' => t('Alignment'),
      '#options' => array(
        'horizontal' => t('Horizontal'),
        'vertical' => t('Vertical'),
      ),
      '#default_value' => $this->options['alignment'],
    );
    $form['alignment']['horizontal']['#description'] = t('Places items starting in the upper left and moving right.');
    $form['alignment']['vertical']['#description'] = t('Places items starting in the upper left and moving down.');
  }
}
