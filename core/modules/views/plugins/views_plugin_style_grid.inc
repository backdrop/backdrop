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

    $options['columns'] = ['default' => '4'];
    $options['alignment'] = ['default' => 'horizontal'];
    $options['fill_single_line'] = ['default' => TRUE, 'bool' => TRUE];
    $options['summary'] = ['default' => ''];
    $options['caption'] = ['default' => ''];
    $options['deprecated_table'] = ['default' => FALSE, 'bool' => TRUE];

    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['columns'] = [
      '#type' => 'number',
      '#title' => t('Number of columns'),
      '#default_value' => $this->options['columns'],
      '#required' => TRUE,
      '#min' => 1,
      '#step' => 1,
      '#weight' => -2,
    ];
    $form['alignment'] = [
      '#type' => 'radios',
      '#title' => t('Alignment'),
      '#options' => ['horizontal' => t('Horizontal'), 'vertical' => t('Vertical')],
      '#default_value' => $this->options['alignment'],
    ];
    $form['alignment']['horizontal']['#description'] = t('Places items starting in the upper left and moving right.');
    $form['alignment']['vertical']['#description'] = t('Places items starting in the upper left and moving down.');

    $form['deprecated_table'] = [
      '#type' => 'checkbox',
      '#title' => t('Use HTML table (deprecated)'),
      '#description' => t('For sites with views grids that need to use the deprecated HTML tables.'),
      '#default_value' => !empty($this->options['deprecated_table']),
    ];

    $form['fill_single_line'] = [
      '#type' => 'checkbox',
      '#title' => t('Fill up single line'),
      '#description' => t('If you disable this option, a grid with only one row will have the same number of table cells (<code>&lt;td&gt;</code>) as items. Disabling it can cause problems with your CSS.'),
      '#default_value' => !empty($this->options['fill_single_line']),
      '#states' => [
        'invisible' => [
          ':input[name="style_options[deprecated_table]"]' => ['checked' => FALSE],
        ],
      ],
    ];

    $form['caption'] = [
      '#type' => 'textfield',
      '#title' => t('Short description of table'),
      '#description' => t('Include a caption for better accessibility of your table.'),
      '#default_value' => $this->options['caption'],
      '#states' => [
        'invisible' => [
          ':input[name="style_options[deprecated_table]"]' => ['checked' => FALSE],
        ],
      ],
    ];

    $form['summary'] = [
      '#type' => 'textfield',
      '#title' => t('Table summary'),
      '#description' => t('This value will be displayed as table-summary attribute in the html. Use this to give a summary of complex tables.'),
      '#default_value' => $this->options['summary'],
      '#states' => [
        'invisible' => [
          ':input[name="style_options[deprecated_table]"]' => ['checked' => FALSE],
        ],
      ],
    ];
  }
}
