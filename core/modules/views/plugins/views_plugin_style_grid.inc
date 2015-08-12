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
   * Set default options.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['columns'] = array('default' => '4');
    $options['alignment'] = array('default' => 'horizontal');
    $options['wrapper_classes'] = array('default' => '');
    $options['column_classes'] = array('default' => 'views-column');
    $options['first_column_classes'] = array('default' => 'views-column-first');
    $options['last_column_classes'] = array('default' => 'views-column-last');
    $options['row_classes'] = array('default' => 'views-row');
    $options['first_row_classes'] = array('default' => 'views-row-first');
    $options['last_row_classes'] = array('default' => 'views-row-last');
    $options['default_classes'] = array('default' => 0);
    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    
    // Flatten options to deal with the various hierarchy changes.
    $options = views_grid_get_options($this->options);

    $form['columns'] = array(
      '#type' => 'textfield',
      '#title' => t('Number of columns'),
      '#default_value' => $options['columns'],
      '#required' => TRUE,
      '#element_validate' => array('views_element_validate_integer'),
    );
    $form['alignment'] = array(
      '#type' => 'radios',
      '#title' => t('Alignment'),
      '#options' => array('horizontal' => t('Horizontal'), 'vertical' => t('Vertical')),
      '#default_value' => $options['alignment'],
      '#description' => t('Horizontal alignment will place items starting in the upper left and moving right. Vertical alignment will place items starting in the upper left and moving down.'),
    );
    $form['default_classes'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remove default views classes'),
      '#default_value' => $options['default_classes'],
    );
    $form['grid']= array(
      '#type' => 'fieldset',
      '#title' => t('Grid Classes'),
      '#weight' => 70,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['wrapper_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Wrapper'),
      '#default_value' => $options['wrapper_classes'],
      '#fieldset' => 'grid',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['column_class_wrapper']= array(
      '#type' => 'fieldset',
      '#title' => t('Column Classes'),
      '#weight' => 80,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['column_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Every Column'),
      '#default_value' => $options['column_classes'],
      '#fieldset' => 'column_class_wrapper',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['first_column_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('First Column'),
      '#default_value' => $options['first_column_classes'],
      '#fieldset' => 'column_class_wrapper',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['last_column_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Last Column'),
      '#default_value' => $options['last_column_classes'],
      '#fieldset' => 'column_class_wrapper',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['rows']= array(
      '#type' => 'fieldset',
      '#title' => t('Row Classes'),
      '#weight' => 90,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['row_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Every Row'),
      '#default_value' => $options['row_classes'],
      '#fieldset' => 'rows',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['first_row_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('First Row'),
      '#default_value' => $options['first_row_classes'],
      '#fieldset' => 'rows',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
    $form['last_row_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Last Row'),
      '#default_value' => $options['last_row_classes'],
      '#fieldset' => 'rows',
      '#prefix' => '<div class="views-left-30">',
      '#suffix' => '</div>',
    );
  }
}
