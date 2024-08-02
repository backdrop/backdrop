<?php
/**
 * @file
 * Definition of views_handler_field_math.
 */

/**
 * Render a mathematical expression as a numeric value
 *
 * Definition terms:
 * - float: If true this field contains a decimal value. If unset this field
 *   will be assumed to be integer.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_math extends views_handler_field_numeric {
  function option_definition() {
    $options = parent::option_definition();
    $options['expression'] = array('default' => '');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['expression'] = array(
      '#type' => 'textarea',
      '#title' => t('Expression'),
      '#description' => t("Enter mathematical expressions such as <code>2 + 2</code> or <code>sqrt(5)</code>. You may assign variables and create mathematical functions and evaluate them. Use the <code>;</code> to separate these. For example: <code>f(x) = x + 2; f(2)</code>. The result of the mathematical expression of the previous row can be accessed by using the <code>[expression]</code> token itself."),
      '#default_value' => $this->options['expression'],
    );

    // Create a place for the help
    $form['expression_help'] = array();
    parent::options_form($form, $form_state);

    // Then move the existing help:
    $form['expression_help'] = $form['alter']['help'];
    unset($form['expression_help']['#states']);
    unset($form['alter']['help']);
  }

  function render($values) {
    include_once BACKDROP_ROOT . '/core/includes/evalmath.inc';
    $tokens = array_map('floatval', $this->get_render_tokens(array()));
    $value = strtr($this->options['expression'], $tokens);
    $expressions = explode(';', $value);
    $math = new EvalMath;
    foreach ($expressions as $expression) {
      if ($expression !== '') {
        $value = $math->evaluate($expression);
      }
    }

    // The rest is directly from views_handler_field_numeric but because it
    // does not allow the value to be passed in, it is copied.

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
    if (!empty($this->options['format_plural']) && ($value != 0 || !$this->options['empty_zero'])) {
      $value = format_plural($value, $this->options['format_plural_singular'], $this->options['format_plural_plural']);
    }

    return $this->sanitize_value($this->options['prefix'] . $value . $this->options['suffix']);
  }

  function query() { }
}
