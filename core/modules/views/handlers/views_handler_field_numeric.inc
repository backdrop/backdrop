<?php
/**
 * @file
 * Definition of views_handler_field_numeric.
 */

/**
 * Render a field as a numeric value
 *
 * Definition terms:
 * - float: If true this field contains a decimal value. If unset this field
 *          will be assumed to be integer.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_numeric extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();

    $options['set_precision'] = array('default' => FALSE, 'bool' => TRUE);
    $options['precision'] = array('default' => 0);
    $options['decimal'] = array('default' => '.', 'translatable' => TRUE);
    $options['separator'] = array('default' => ',', 'translatable' => TRUE);
    $options['format_plural'] = array('default' => FALSE, 'bool' => TRUE);
    $options['format_plural_singular'] = array('default' => '1');
    $options['format_plural_plural'] = array('default' => '@count');
    $options['prefix'] = array('default' => '', 'translatable' => TRUE);
    $options['suffix'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    if (!empty($this->definition['float'])) {
      $form['set_precision'] = array(
        '#type' => 'checkbox',
        '#title' => t('Round'),
        '#description' => t('If checked, the number will be rounded.'),
        '#default_value' => $this->options['set_precision'],
      );
      $form['precision'] = array(
        '#type' => 'textfield',
        '#title' => t('Precision'),
        '#default_value' => $this->options['precision'],
        '#description' => t('Specify how many digits to print after the decimal point.'),
        '#states' => array(
          'visible' => array(
            ':input[name="options[set_precision]"]' => array('checked' => TRUE),
          ),
        ),
        '#size' => 2,
      );
      $form['decimal'] = array(
        '#type' => 'textfield',
        '#title' => t('Decimal point'),
        '#default_value' => $this->options['decimal'],
        '#description' => t('What single character to use as a decimal point.'),
        '#size' => 2,
      );
    }
    $form['separator'] = array(
      '#type' => 'select',
      '#title' => t('Thousands marker'),
      '#options' => array(
        '' => t('- None -'),
        ',' => t('Comma'),
        ' ' => t('Space'),
        '.' => t('Decimal'),
        '\'' => t('Apostrophe'),
      ),
      '#default_value' => $this->options['separator'],
      '#description' => t('What single character to use as the thousands separator.'),
      '#size' => 2,
    );
    $form['format_plural'] = array(
      '#type' => 'checkbox',
      '#title' => t('Format plural'),
      '#description' => t('If checked, special handling will be used for plurality.'),
      '#default_value' => $this->options['format_plural'],
    );
    $form['format_plural_singular'] = array(
      '#type' => 'textfield',
      '#title' => t('Singular form'),
      '#default_value' => $this->options['format_plural_singular'],
      '#description' => t('Text to use for the singular form.'),
      '#states' => array(
        'visible' => array(
          ':input[name="options[format_plural]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['format_plural_plural'] = array(
      '#type' => 'textfield',
      '#title' => t('Plural form'),
      '#default_value' => $this->options['format_plural_plural'],
      '#description' => t('Text to use for the plural form, @count will be replaced with the value.'),
      '#states' => array(
        'visible' => array(
          ':input[name="options[format_plural]"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Prefix'),
      '#default_value' => $this->options['prefix'],
      '#description' => t('Text to put before the number, such as currency symbol.'),
    );
    $form['suffix'] = array(
      '#type' => 'textfield',
      '#title' => t('Suffix'),
      '#default_value' => $this->options['suffix'],
      '#description' => t('Text to put after the number, such as currency symbol.'),
    );

    parent::options_form($form, $form_state);
  }

  function render($values) {
    $value = $this->get_value($values);

    // Output nothing if the value is null.
    if (is_null($value)) {
      return '';
    }

    // Check to see if hiding should happen before adding prefix and suffix.
    if ($this->options['hide_empty'] && empty($value) && ($value !== 0 || $this->options['empty_zero'])) {
      return '';
    }

    if (!empty($this->options['set_precision'])) {
      $precision = $this->options['precision'];
    }
    elseif ($decimal_position = strpos($value, '.')) {
      $precision = strlen(rtrim($value, '0')) - $decimal_position - 1;
    }
    else {
      $precision = 0;
    }

    // Use round first to avoid negative zeros.
    $value = round($value, $precision);
    // Test against both integer zero and float zero.
    if ($this->options['empty_zero'] && ($value === 0 || $value === 0.0)) {
       return '';
     }

    $value = number_format($value, $precision, $this->options['decimal'], $this->options['separator']);

    // Should we format as a plural.
    if (!empty($this->options['format_plural'])) {
      $value = format_plural($value, $this->options['format_plural_singular'], $this->options['format_plural_plural']);
    }

    return $this->sanitize_value($this->options['prefix'], 'xss')
      . $this->sanitize_value($value)
      . $this->sanitize_value($this->options['suffix'], 'xss');
  }
}
