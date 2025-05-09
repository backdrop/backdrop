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
class views_plugin_style_grid extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['columns'] = array('default' => '4');
    $options['alignment'] = array('default' => 'horizontal');
    $options['fill_single_line'] = array('default' => TRUE, 'bool' => TRUE);
    $options['summary'] = array('default' => '');
    $options['caption'] = array('default' => '');
    $options['deprecated_table'] = array('default' => FALSE, 'bool' => TRUE);
    $options['responsive'] = array('default' => FALSE, 'bool' => TRUE);
    $options['cell_min_width'] = array('default' => '100');
    $options['grid_gutter'] = array('default' => '10');

    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['columns'] = array(
      '#type' => 'number',
      '#title' => t('Number of columns'),
      '#default_value' => $this->options['columns'],
      '#required' => TRUE,
      '#min' => 1,
      '#step' => 1,
    );
    $form['responsive'] = array(
      '#type' => 'checkbox',
      '#title' => t('Responsive'),
      '#description' => t('The grid cells will reflow to have fewer columns on narrower screens.'),
      '#default_value' => $this->options['responsive'],
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
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[responsive]"]' => array('checked' => FALSE),
        ),
      ),
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
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[responsive]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['alignment'] = array(
      '#type' => 'radios',
      '#title' => t('Alignment'),
      '#options' => array('horizontal' => t('Horizontal'), 'vertical' => t('Vertical')),
      '#default_value' => $this->options['alignment'],
    );
    $form['alignment']['horizontal']['#description'] = t('Places items starting in the upper left and moving right.');
    $form['alignment']['vertical']['#description'] = t('Places items starting in the upper left and moving down.');

    $form['unresponsive'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use unresponsive grid'),
      '#description' => t('The grid cells will have the same number of columns regardless of screen size.'),
      '#default_value' => !empty($this->options['unresponsive']),
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[deprecated_table]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['deprecated_table'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use HTML table (deprecated)'),
      '#description' => t('For sites with views grids that need to use the deprecated HTML tables.'),
      '#default_value' => !empty($this->options['deprecated_table']),
    );

    $form['fill_single_line'] = array(
      '#type' => 'checkbox',
      '#title' => t('Fill up single line'),
      '#description' => t('If you disable this option, a grid with only one row will have the same number of table cells (<code>&lt;td&gt;</code>) as items. Disabling it can cause problems with your CSS.'),
      '#default_value' => !empty($this->options['fill_single_line']),
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[deprecated_table]"]' => array('checked' => FALSE),
        ),
      ),
    );

    $form['caption'] = array(
      '#type' => 'textfield',
      '#title' => t('Short description of table'),
      '#description' => t('Include a caption for better accessibility of your table.'),
      '#default_value' => $this->options['caption'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[deprecated_table]"]' => array('checked' => FALSE),
        ),
      ),
    );

    $form['summary'] = array(
      '#type' => 'textfield',
      '#title' => t('Table summary'),
      '#description' => t('This value will be displayed as table-summary attribute in the html. Use this to give a summary of complex tables.'),
      '#default_value' => $this->options['summary'],
      '#states' => array(
        'invisible' => array(
          ':input[name="style_options[deprecated_table]"]' => array('checked' => FALSE),
        ),
      ),
    );
  }
}
