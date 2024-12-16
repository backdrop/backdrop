<?php
/**
 * @file
 * Contains the default summary style plugin, which displays items in an HTML list.
 */

/**
 * The default style plugin for summaries.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_summary extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();

    $options['base_path'] = ['default' => ''];
    $options['count'] = ['default' => TRUE, 'bool' => TRUE];
    $options['override'] = ['default' => FALSE, 'bool' => TRUE];
    $options['items_per_page'] = ['default' => 25];

    return $options;
  }

  function query() {
    if (!empty($this->options['override'])) {
      $this->view->set_items_per_page(intval($this->options['items_per_page']));
    }
  }

  function options_form(&$form, &$form_state) {
    $form['base_path'] = [
      '#type' => 'textfield',
      '#title' => t('Base path'),
      '#default_value' => $this->options['base_path'],
      '#description' => t('Define the base path for links in this summary
        view, i.e. http://example.com/<strong>your_view_path/archive</strong>.
        Do not include beginning and ending forward slash. If this value
        is empty, views will use the first path found as the base path,
        in page displays, or / if no path could be found.'),
    ];
    $form['count'] = [
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['count']),
      '#title' => t('Display record count with link'),
    ];
    $form['override'] = [
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['override']),
      '#title' => t('Override number of items to display'),
    ];

    $form['items_per_page'] = [
      '#type' => 'textfield',
      '#title' => t('Items to display'),
      '#default_value' => $this->options['items_per_page'],
      '#states' => [
        'visible' => [
          ':input[name="options[summary][options][' . $this->definition['name'] . '][override]"]' => ['checked' => TRUE],
        ],
      ],
    ];
  }

  function render() {
    $rows = [];
    foreach ($this->view->result as $row) {
      // @todo: Include separator as an option.
      $rows[] = $row;
    }

    return theme($this->theme_functions(), [
      'view' => $this->view,
      'options' => $this->options,
      'rows' => $rows
    ]);
  }
}
