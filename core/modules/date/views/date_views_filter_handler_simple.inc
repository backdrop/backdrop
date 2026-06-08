<?php
/**
 * @file
 * A standard Views filter for a single date field,
 * using Date API form selectors and sql handling.
 */
class date_views_filter_handler_simple extends views_handler_filter_date {
  /**
   * @var date_sql_handler
   */
  protected $date_handler = NULL;

  /**
   * @var string
   */
  protected $format;

  /**
   * @var string
   */
  protected $base_table;

  /**
   * @var int
   */
  public $element_number = 1;

  /**
   * @var views_plugin_query_default
   */
  var $query;

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->date_handler = new date_sql_handler(DATE_UNIX);
    if (!empty($this->definition['field_name'])) {
      $field = field_info_field($this->definition['field_name']);
      if (!empty($field) && !empty($field['type'])) {
        $this->date_handler->date_type = $field['type'];
      }
      $this->date_handler->db_timezone = date_get_timezone_db($field['settings']['tz_handling']);
      $this->date_handler->local_timezone = date_get_timezone($field['settings']['tz_handling']);
    }
    $this->date_handler->granularity = isset($options['granularity']) ? $options['granularity'] : 'day';
    $this->format = $this->date_handler->views_formats($this->options['granularity'], 'sql');

    // Identify the base table for this field.
    // It will be used to call for the right query field options.
    $this->base_table = $this->table;
  }

  // Set default values for the date filter.
  function option_definition() {
    $options = parent::option_definition();
    $options['granularity'] = array('default' => 'day');
    $options['form_type'] = array('default' => 'date_select');
    $options['default_date'] = array('default' => '');
    $options['default_to_date'] = array('default' => '');
    $options['year_range'] = array('default' => '-3:+3');
    $options['add_delta'] = array('default' => '');
    return $options;
  }

  function operators() {
    $operators = parent::operators();
    $operators['contains'] = array(
      'title' => t('Contains'),
      'method' => 'op_contains',
      'short' => t('contains'),
      'values' => 1,
    );
    return $operators;
  }

  /**
   * Helper function to find a default value.
   */
  function date_default_value($prefix, $options = NULL) {
    if (empty($options)) {
      if ($this->options['is_grouped']) {
        if ('All' == $this->options['group_info']['default_group']) {
          return '';
        }
        $options = $this->options['group_info']['group_items'][$this->options['group_info']['default_group']]['value'][$prefix . '_group'];
      }
      else {
        $options = $this->options;
      }
    }
    // If this is a remembered value, use the value from the SESSION.
    if (!empty($this->options['expose']['remember'])) {
      $display_id = ($this->view->display_handler->is_defaulted('filters')) ? 'default' : $this->view->current_display;
      if (!empty($_SESSION['views'][$this->view->name][$display_id][$this->options['expose']['identifier']][$prefix])) {
        return $_SESSION['views'][$this->view->name][$display_id][$this->options['expose']['identifier']][$prefix];
      }
    }

    $default_option = $prefix == 'max' ? $options['default_to_date'] : $options['default_date'];
    $is_relative = ($this->options['is_grouped'] && isset($options[$prefix . '_choose_input_type']) && $options[$prefix . '_choose_input_type'] == 'relative') || (!empty($this->options['value']['type']) && $this->options['value']['type'] == 'relative');
    if ($is_relative) {
      $default_date = $default_option;
    }
    // This a fixed date.
    else {
      if ($this->options['is_grouped']) {
        $default_date = empty($options[$prefix]) ? '' : $options[$prefix];
      }
      else {
        $default_date = isset($options['value'][$prefix]) ? $options['value'][$prefix] : '';
      }
    }
    return $default_date;
  }

  /**
   * Helper function to see if we need to swap in the default value.
   *
   * Views exposed filters treat everything as submitted, so if it's an empty value we have to
   * see if anything actually was submitted. If nothing has really been submitted, we need
   * to swap in our default value logic.
   */
  function get_filter_value($prefix, $input) {
    // All our date widgets provide datetime values but we use ISO in our SQL
    // for consistency between the way filters and arguments work (arguments
    // cannot contain spaces).

    // If this is a relative date, we need to convert it to an absolute date.
    // If this input comes from a group, we then must check if it's a relative
    // string before sending it to the database.
    $is_relative = ($this->options['value']['type'] == 'relative' || $this->options['is_grouped']);

    if ($is_relative && !empty($input)) {
      if (is_array($input)) {
        // Find the first non-empty value in the array.
        $flattened = array_filter($input);
        $input = !empty($flattened) ? reset($flattened) : '';
      }
      if (is_string($input)) {
        $input = trim($input);
      }
      if (is_string($input) && $input !== '') {
        $date = date_create($input, date_default_timezone_object());
        if ($date) {
          $input = $date->format($this->format);
        }
      }
    }

    if (!isset($input) || $input === '') {
      $prefix_val = $this->date_default_value($prefix);
      if (is_null($prefix_val)) {
        $prefix_val = '';
      }
      if (empty($this->options['exposed'])) {
        if (is_array($prefix_val)) {
          $prefix_val = reset($prefix_val);
        }
        return str_replace(' ', 'T', (string) $prefix_val);
      }
      // If it's exposed and we have input for this identifier, it means
      // the user explicitly submitted an empty value.
      elseif (isset($this->view->exposed_raw_input[$this->options['expose']['identifier']])) {
        return '';
      }
      elseif ($this->options['is_grouped'] && isset($this->view->exposed_raw_input[$this->options['group_info']['identifier']])) {
        $identifier = $this->view->exposed_raw_input[$this->options['group_info']['identifier']];
        $group_item = $this->options['group_info']['group_items'][$identifier];
        $options = $group_item['value'][$prefix . '_group'];
        $prefix_val = $this->date_default_value($prefix, $options);

        // If the group item is configured as a relative date, resolve it here.
        $is_item_relative = (isset($options[$prefix . '_choose_input_type']) &&
          $options[$prefix . '_choose_input_type'] == 'relative');
        if ($is_item_relative && !empty($prefix_val)) {
          if (is_array($prefix_val)) {
            $flattened = array_filter($prefix_val);
            $prefix_val = !empty($flattened) ? reset($flattened) : '';
          }
          if (is_string($prefix_val) && $prefix_val !== '') {
            $date = date_create($prefix_val, date_default_timezone_object());
            if ($date) {
              $prefix_val = $date->format($this->format);
            }
          }
        }

        if (is_array($prefix_val)) {
          $prefix_val = reset($prefix_val);
        }
        return str_replace(' ', 'T', (string) $prefix_val);
      }
      elseif (isset($this->options['expose']['identifier']) && !isset($this->view->exposed_raw_input[$this->options['expose']['identifier']]['value'])) {
        if (is_array($prefix_val)) {
          $prefix_val = reset($prefix_val);
        }
        return str_replace(' ', 'T', (string) $prefix_val);
      }
    }

    if (is_array($input)) {
      $flattened = array_filter($input);
      $input = !empty($flattened) ? reset($flattened) : '';
    }

    return str_replace(' ', 'T', (string) $input);
  }

  function accept_exposed_input($input) {
    if (!empty($this->options['exposed'])) {
      $identifier = $this->options['expose']['identifier'];

      // If there is no input for this filter, and it is not a grouped filter,
      // or if it's a grouped filter and there is no input for the group info,
      // it's a "cold start" (initial view load).
      $is_cold_start = FALSE;
      if ($this->options['is_grouped']) {
        if (!isset($input[$this->options['group_info']['identifier']])) {
          $is_cold_start = TRUE;
        }
      }
      else {
        if (!isset($input[$identifier])) {
          $is_cold_start = TRUE;
        }
      }

      if ($is_cold_start) {
        // Cold start: populate $this->value with configured defaults.
        if ($this->options['is_grouped']) {
          // Grouped filters handle their own defaults in parent/logic.
          return parent::accept_exposed_input($input);
        }
        else {
          foreach (array('value', 'min', 'max') as $key) {
            $this->value[$key] = $this->date_default_value($key);
          }
          return TRUE;
        }
      }

      if (!isset($input[$identifier])) {
        return parent::accept_exposed_input($input);
      }
      $element_input = $input[$identifier];
      if ($this->options['is_grouped']) {
        foreach (array('value', 'min', 'max') as $key) {
          $element_input[$key] = '';
          if (!empty($element_input[$key . '_group'])) {
            $val = $this->date_default_value($key, $element_input[$key . '_group']);
            $element_input[$key] = $this->get_filter_value($key, $val);
          }
        }
      }
      else {
        $is_relative = ($this->options['value']['type'] == 'relative');
        $element_input['value'] = isset($element_input['value']) ? $element_input['value'] : '';
        $element_input['min'] = isset($element_input['min']) ? $element_input['min'] : '';
        $element_input['max'] = isset($element_input['max']) ? $element_input['max'] : '';

        // Trim input for relative dates to prevent issues with spaces.
        if ($is_relative) {
          foreach (array('value', 'min', 'max') as $key) {
            if (isset($element_input[$key])) {
              $element_input[$key] = is_array($element_input[$key]) ? reset($element_input[$key]) : $element_input[$key];
              $element_input[$key] = trim($element_input[$key]);
            }
          }
        }
        else {
          $element_input['value'] = $this->get_filter_value('value', $element_input['value']);
          $element_input['min'] = $this->get_filter_value('min', $element_input['min']);
          $element_input['max'] = $this->get_filter_value('max', $element_input['max']);
        }
      }
      if (is_array($element_input) && isset($element_input['default_date'])) {
        unset($element_input['default_date']);
      }
      if (is_array($element_input) && isset($element_input['default_to_date'])) {
        unset($element_input['default_to_date']);
      }

      $input[$this->options['expose']['identifier']] = $element_input;
    }
    return parent::accept_exposed_input($input);

  }

  function op_between($field) {

    // Add the delta field to the view so we can later find the value that matched our query.
    list($table_name, $field_name) = explode('.', $field);
    if (!empty($this->options['add_delta']) && (substr($field_name, -6) == '_value' || substr($field_name, -7) == '_value2')) {
      $this->query->add_field($table_name, 'delta');
      $real_field_name = str_replace(array('_value', '_value2'), '', $this->real_field);
      $this->query->add_field($table_name, 'entity_id', 'date_id_' . $real_field_name);
      $this->query->add_field($table_name, 'delta', 'date_delta_' . $real_field_name);
    }

    $min_value = $this->get_filter_value('min', $this->value['min']);
    $min_comp_date = new BackdropDateTime($min_value, date_default_timezone(), $this->format);
    $max_value = $this->get_filter_value('max', $this->value['max']);
    $max_comp_date = new BackdropDateTime($max_value, date_default_timezone(), $this->format);

    if (is_array($min_value)) {
      $flattened = array_filter($min_value);
      $min_value = !empty($flattened) ? reset($flattened) : '';
    }
    if (is_array($max_value)) {
      $flattened = array_filter($max_value);
      $max_value = !empty($flattened) ? reset($flattened) : '';
    }

    $field_min = $this->date_handler->sql_field($field, NULL, $min_comp_date);
    $field_min = $this->date_handler->sql_format($this->format, $field_min);
    $field_max = $this->date_handler->sql_field($field, NULL, $max_comp_date);
    $field_max = $this->date_handler->sql_format($this->format, $field_max);
    $placeholder_min = $this->placeholder();
    $placeholder_max = $this->placeholder();
    $group = !empty($this->options['date_group']) ? $this->options['date_group'] : $this->options['group'];
    if ($this->operator == 'between') {
      $this->query->add_where_expression($group, "$field_min >= $placeholder_min AND $field_max <= $placeholder_max", array($placeholder_min => $min_value, $placeholder_max => $max_value));
    }
    else {
      $this->query->add_where_expression($group, "$field_min < $placeholder_min OR $field_max > $placeholder_max", array($placeholder_min => $min_value, $placeholder_max => $max_value));
    }
  }

  function op_simple($field) {

    // Add the delta field to the view so we can later find the value that matched our query.
    list($table_name, $field_name) = explode('.', $field);
    if (!empty($this->options['add_delta']) && (substr($field_name, -6) == '_value' || substr($field_name, -7) == '_value2')) {
      $this->query->add_field($table_name, 'delta');
      $real_field_name = str_replace(array('_value', '_value2'), '', $this->real_field);
      $this->query->add_field($table_name, 'entity_id', 'date_id_' . $real_field_name);
      $this->query->add_field($table_name, 'delta', 'date_delta_' . $real_field_name);
    }

    $value = $this->get_filter_value('value', $this->value['value']);
    $comp_date = new BackdropDateTime($value, date_default_timezone(), $this->format);

    if (is_array($value)) {
      $flattened = array_filter($value);
      $value = !empty($flattened) ? reset($flattened) : '';
    }

    $field = $this->date_handler->sql_field($field, NULL, $comp_date);
    $field = $this->date_handler->sql_format($this->format, $field);
    $placeholder = $this->placeholder();
    $group = !empty($this->options['date_group']) ? $this->options['date_group'] : $this->options['group'];
    $this->query->add_where_expression($group, "$field $this->operator $placeholder", array($placeholder => $value));
  }

  function op_contains($field) {

    // Add the delta field to the view so we can later find the value that matched our query.
    list($table_name, $field_name) = explode('.', $field);
    if (!empty($this->options['add_delta']) && (substr($field_name, -6) == '_value' || substr($field_name, -7) == '_value2')) {
      $this->query->add_field($table_name, 'delta');
    }

    $value = $this->get_filter_value('value', $this->value['value']);
    $comp_date = new BackdropDateTime($value, date_default_timezone(), $this->format);

    if (is_array($value)) {
      $flattened = array_filter($value);
      $value = !empty($flattened) ? reset($flattened) : '';
    }

    $fields = date_views_fields($this->base_table);
    $fields = $fields['name'];
    $fromto = $fields[$field]['fromto'];
    $field_min = $this->date_handler->sql_field($fromto[0], NULL, $comp_date);
    $field_min = $this->date_handler->sql_format($this->format, $field_min);
    $field_max = $this->date_handler->sql_field($fromto[1], NULL, $comp_date);
    $field_max = $this->date_handler->sql_format($this->format, $field_max);
    $placeholder_min = $this->placeholder();
    $placeholder_max = $this->placeholder();
    $group = !empty($this->options['date_group']) ? $this->options['date_group'] : $this->options['group'];
    $this->query->add_where_expression($group, "$field_max >= $placeholder_min AND $field_min <= $placeholder_max", array($placeholder_min => $value, $placeholder_max => $value));
  }

  /**
   * Set the granularity of the date parts to use in the filter.
    */
  function has_extra_options() { return TRUE; }

  /**
   * Date selection options.
   */
  function widget_options() {
    $options = array(
      'date_select' => t('Select'),
      'date_text' => t('Text'),
      'date_popup' => t('Popup'),
    );
    return $options;
  }

  function year_range() {
    $year_range = explode(':', $this->options['year_range']);
    if (substr($this->options['year_range'], 0, 1) == '-' || $year_range[0] < 0) {
      $this_year = date_format(date_now(), 'Y');
      $year_range[0] = $this_year + $year_range[0];
      $year_range[1] = $this_year + $year_range[1];
    }
    return $year_range;
  }

  function extra_options_form(&$form, &$form_state) {
    parent::extra_options_form($form, $form_state);
    $form['form_type'] = array(
      '#type' => 'radios',
      '#title' => t('Date selection form element'),
      '#default_value' => $this->options['form_type'],
      '#options' => $this->widget_options(),
      );

    $form['granularity'] = $this->date_handler->granularity_form($this->options['granularity']);
    $form['granularity']['#title'] = t('Filter granularity');

    $form['year_range'] = array(
      '#type' => 'date_year_range',
      '#default_value' => $this->options['year_range'],
    );

    if (!empty($this->definition['field_name'])) {
      $field = field_info_field($this->definition['field_name']);
    }
    $form['add_delta'] = array(
      '#type' => 'radios',
      '#title' => t('Add multiple value identifier'),
      '#default_value' => $this->options['add_delta'],
      '#options' => array('' => t('No'), 'yes' => t('Yes')),
      '#description' => t('Add an identifier to the view to show which multiple value date fields meet the filter criteria. Note: This option may introduce duplicate values into the view. Required when using multiple value fields in a Calendar or any time you want the node view of multiple value dates to display only the values that match the view filters.'),
      // Only let mere mortals tweak this setting for multi-value fields
      '#access' => !empty($field) ? $field['cardinality'] != 1 : 0,
    );
  }

  function extra_options_validate($form, &$form_state) {
    if (!preg_match('/^(?:[\+\-][0-9]{1,4}|[0-9]{4}):(?:[\+\-][0-9]{1,4}|[0-9]{4})$/', $form_state['values']['options']['year_range'])) {
      form_error($form['year_range'], t('Date year range must be in the format -9:+9, 2005:2010, -9:2010, or 2005:+9'));
    }
  }

  /**
   * Add the selectors to the value form using the date handler.
   */
  function value_form(&$form, &$form_state) {
    // We use different values than the parent form, so we must
    // construct our own form element.
    $form['value'] = array();
    $form['value']['#tree'] = TRUE;
    $form['value']['#id'] = 'date_views_exposed_filter-' . bin2hex(backdrop_random_bytes(16));

    // Below section copied from views_handler_filter_numeric.inc.
    $which = 'all';
    $source = '';
    if (!empty($form['operator'])) {
      $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : '#edit-options-operator';
    }

    $identifier = $this->options['expose']['identifier'];
    if (!empty($form_state['exposed'])) {

      if (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator_id'])) {
        // exposed and locked.
        $which = in_array($this->operator, $this->operator_values(2)) ? 'minmax' : 'value';
      }
      else {
        $source = '#edit-' . backdrop_html_id($this->options['expose']['operator_id']);
      }
    }

    if ($which == 'all' || $which == 'value') {
      $form['value'] += $this->date_parts_form($form_state, 'value', $source, $which, $this->operator_values(1), $identifier, 'default_date', $this->element_number);
    }

    if ($which == 'all' || $which == 'minmax') {
      $value2 = $this->operator_values(2);

      foreach (array(
        'min' => 'default_date',
        'max' => 'default_to_date',
      ) as $type => $prop) {
        $form['value'] += $this->date_parts_form($form_state, $type, $source, $which, $value2, $identifier, $prop, $this->element_number);
      }
    }

    $this->element_number++;

    // Add some extra validation for the select widget to be sure that
    // the user inputs all parts of the date.
    if ($this->options['form_type'] == 'date_select') {
      $form['value']['#element_validate'] = array(array($this, 'date_select_validate'));
    }

  }

  /**
   * A form element to select date part values.
   *
   * @param string $prefix
   *   A prefix for the date values, 'value', 'min', or 'max' .
   * @param string $source
   *   An HTML selector for the operator for this element.
   * @param string $which
   *   Which element to provide, 'all', 'value', or 'minmax' .
   * @param array $operator_values
   *   An array of the allowed operators for this element.
   * @param string $identifier
   *   Identifier of the exposed element.
   * @param string $relative_id
   *   Form element ID to use for the relative date field.
   *
   * @return array
   *   The form date part element for this instance.
   */
  public function date_parts_form(array &$form_state, $prefix, $source, $which, array $operator_values, $identifier, $relative_id, $element_no = 0) {
    module_load_include('inc', 'date', 'date.elements');
    switch ($prefix) {
      case 'min':
        $label = t('Start date');
        $relative_label = t('Relative start date');
        break;
      case 'max':
        $label = t('End date');
        $relative_label = t('Relative end date');
        break;
      default:
        $label = '';
        $relative_label = t('Relative date');
        break;
    }

    $type = $this->options['form_type'];
    $format = $this->date_handler->views_formats($this->options['granularity'], 'display');
    $granularity = array_keys($this->date_handler->date_parts($this->options['granularity']));
    $default_date = $this->date_default_value($prefix);
    $relative_value_key = $prefix == 'max' ? 'default_to_date' : 'default_date';
    $relative_value = $this->options[$relative_value_key];

    if (!empty($form_state['exposed'])) {
      // UI when the date selector is exposed.
      $identifier = $this->options['expose']['identifier'];
      $is_relative = ($this->options['value']['type'] == 'relative');
      if ($is_relative) {
        $default_relative_value = '';
        if (isset($this->view->exposed_input[$identifier][$prefix]) && is_string($this->view->exposed_input[$identifier][$prefix]) && trim($this->view->exposed_input[$identifier][$prefix]) !== '') {
          $default_relative_value = trim($this->view->exposed_input[$identifier][$prefix]);
        }
        elseif (isset($this->value[$prefix]) && is_string($this->value[$prefix]) && trim($this->value[$prefix]) !== '') {
          $default_relative_value = trim($this->value[$prefix]);
        }
        elseif (!empty($relative_value)) {
          $default_relative_value = $relative_value;
        }

        if (is_array($default_relative_value)) {
          $default_relative_value = reset($default_relative_value);
        }
        $form[$prefix] = array(
          '#type' => 'textfield',
          '#title' => check_plain($relative_label),
          '#size' => 20,
          '#default_value' => (string) $default_relative_value,
          '#description' => t('Examples: now, now +1 day, 12AM today, Monday next week.'),
        );
      }
      else {
        $form_default_value = '';
        if (isset($this->view->exposed_input[$identifier][$prefix]) && $this->view->exposed_input[$identifier][$prefix] !== '') {
          $form_default_value = $this->view->exposed_input[$identifier][$prefix];
        }
        elseif (isset($this->value[$prefix]) && $this->value[$prefix] !== '') {
          $form_default_value = $this->value[$prefix];
        }
        elseif (!empty($default_date)) {
          $form_default_value = $default_date;
        }

        if (is_array($form_default_value)) {
          $flattened = array_filter($form_default_value);
          $form_default_value = !empty($flattened) ? reset($flattened) : '';
        }

        $form[$prefix] = array(
          '#title' => check_plain($label),
          '#type' => $type,
          '#size' => 20,
          '#default_value' => (string) $form_default_value,
          '#date_format' => date_limit_format($format, $granularity),
          '#date_label_position' => 'within',
          '#date_year_range' => $this->options['year_range'],
          '#process' => array($type . '_element_process'),
        );
      }
      if ($which == 'all') {
        $form[$prefix]['#states']['visible'] = array();
        foreach ($operator_values as $operator) {
          $form[$prefix]['#states']['visible'][] = array(
            $source => array('value' => $operator),
          );
        }
      }
      if (!$is_relative && !isset($form_state['input'][$identifier][$prefix])) {
        // Handle bogus input from the query string to prevent fatal errors.
        if (isset($form_state['input'][$identifier]) && !is_array($form_state['input'][$identifier])) {
          $form_state['input'][$identifier] = array();
        }
        // Ensure these exist.
        foreach ($granularity as $key) {
          $form_state['input'][$identifier][$prefix][$key] = NULL;
        }
      }
    }
    else {
      // UI when the date selector is on the views configuration screen.
      // Fix for grouped between filter. Input class has to be different
      // for each group and default value needs to be taken from group item.
      $input_type_key = $prefix . '_choose_input_type';
      if ($this->options['is_grouped']) {
        $source = "#edit-options-group-info-group-items-{$element_no}-operator";
        $input_class = "$prefix-$element_no-choose-input-type";
        $default_select_date_value = !empty($this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$prefix]) ? $this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$prefix] : $default_date;
        $relative_value = !empty($this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$relative_value_key]) ? $this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$relative_value_key] : $this->options[$relative_value_key];
        $default_select_date_type = !empty($this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$input_type_key]) ? $this->options['group_info']['group_items'][$element_no]['value'][$prefix . '_group'][$input_type_key] : 'date';
      }
      else {
        $input_class = "$prefix-choose-input-type";
        $default_select_date_value = !empty($this->value[$prefix]) ? $this->value[$prefix] : $default_date;
        $default_select_date_type = !empty($relative_value) ? 'relative' : 'date';
      }

      if (is_array($default_select_date_value)) {
        $flattened = array_filter($default_select_date_value);
        $default_select_date_value = !empty($flattened) ? reset($flattened) : '';
      }

      $form[$prefix . '_group'] = array(
        '#type' => 'fieldset',
        '#attributes' => array('class' => array('date-views-filter-fieldset')),
      );
      $form[$prefix . '_group'][$input_type_key] = array(
        '#title' => check_plain($label),
        '#type' => 'select',
        '#options' => array(
          'date' => t('Select a date'),
          'relative' => ('Enter a relative date'),
        ),
        '#attributes' => array(
          'class' => array(
            $input_class,
          ),
        ),
        '#default_value' => $default_select_date_type,
      );
      $form[$prefix . '_group'][$prefix] = array(
        '#title' => t('Select a date'),
        '#type' => $type,
        '#size' => 20,
        '#default_value' => $default_select_date_value,
        '#date_format' => date_limit_format($format, $granularity),
        '#date_label_position' => 'within',
        '#date_year_range' => $this->options['year_range'],
        '#process' => array($type . '_element_process'),
        '#states' => array(
          'visible' => array(
            ":input.$input_class" => array('value' => 'date'),
          ),
        ),
      );
      $form[$prefix . '_group'][$relative_id] = array(
        '#type' => 'textfield',
        '#title' => check_plain($relative_label),
        '#default_value' => $relative_value,
        '#description' => t('Relative dates are computed when the view is displayed. Examples: now, now +1 day, 12AM today, Monday next week. <a href="https://www.php.net/manual/datetime.formats.php#datetime.formats.relative">More examples of relative date formats in the PHP documentation</a>.'),
        '#states' => array(
          'visible' => array(
            ":input.$input_class" => array(
              'value' => 'relative',
            ),
          ),
        ),
      );
      if ($which == 'all') {
        $form[$prefix . '_group']['#states']['visible'] = array();
        foreach ($operator_values as $operator) {
          $form[$prefix . '_group']['#states']['visible'][] = array(
            $source => array('value' => $operator),
          );
        }
      }
    }
    return $form;
  }

  /**
   * Value validation.
   *
   * TODO add in more validation.
   *
   * We are setting an extra option using a value form
   * because it makes more sense to set it there.
   * That's not the normal method, so we have to manually
   * transfer the selected value back to the option.
   */
  function value_validate($form, &$form_state) {

    if (!isset($form_state['values']['options']['operator'])) {
      return;
    }

    $options = &$form_state['values']['options'];

    if ($options['operator'] == 'between' || $options['operator'] == 'not between') {
      if ($options['value']['min_group']['min_choose_input_type'] == 'relative') {
        if (empty($options['value']['min_group']['default_date'])) {
          form_set_error('options][value][min_group][default_date', t('Relative start date not specified.'));
        }
        else {
          $this->options['default_date'] = $options['value']['min_group']['default_date'];
          // NULL out the value field, user wanted the relative value to take hold.
          $options['value']['min_group']['min'] = NULL;
          $this->options['value']['type'] = 'relative';
        }
      }
      // If an absolute date was used, be sure to wipe the relative date.
      else {
        $this->options['default_date'] = '';
        $this->options['value']['type'] = 'date';
      }
      if ($options['value']['max_group']['max_choose_input_type'] == 'relative') {
        if (empty($options['value']['max_group']['default_to_date'])) {
          form_set_error('options][value][max_group][default_to_date', t('Relative end date not specified.'));
        }
        else {
          $this->options['default_to_date'] = $options['value']['max_group']['default_to_date'];
          // NULL out the value field, user wanted the relative value to take hold.
          $options['value']['max_group']['max'] = NULL;
          $this->options['value']['type'] = 'relative';
        }
      }
      // If an absolute date was used, be sure to wipe the relative date.
      else {
        $this->options['default_to_date'] = '';
        $this->options['value']['type'] = 'date';
      }
    }
    elseif (in_array($options['operator'], array('<', '<=', '=', '!=', '>=', '>'))) {
      if ($options['value']['value_group']['value_choose_input_type'] == 'relative') {
        if (empty($options['value']['value_group']['default_date'])) {
          form_set_error('options][value][value_group][default_date', t('Relative date not specified.'));
        }
        else {
          $this->options['default_date'] = $options['value']['value_group']['default_date'];
          // NULL out the value field, user wanted the relative value to take hold.
          $options['value']['value_group']['value'] = NULL;
          $this->options['value']['type'] = 'relative';
        }
      }
      // If an absolute date was used, be sure to wipe the relative date.
      else {
        $this->options['default_date'] = '';
        $this->options['value']['type'] = 'date';
      }
    }
    // Flatten the form structure for views, so the values can be saved.
    foreach (array('value', 'min', 'max') as $key) {
      $options['value'][$key] = $options['value'][$key . '_group'][$key];
    }
  }

  /**
   * Validate that the time values convert to something usable.
   */
  function validate_valid_time(&$form, $operator, $value) {
    // Override the core date filter validation.
    // Our date widgets do their own validation.
  }

  // Update the summary values to provide
  // meaningful information for each option.
  function admin_summary() {
    $parts = $this->date_handler->date_parts();
    $widget_options = $this->widget_options();
    // If the filter is exposed, display the granularity.
    if ($this->options['exposed']) {
      $output = t('<strong>Exposed</strong> @widget @format', array(
        '@format' => $parts[$this->date_handler->granularity],
        '@widget' => $widget_options[$this->options['form_type']],
      ));
      if (!empty($this->options['value']['type']) && $this->options['value']['type'] == 'relative') {
        $output .= ' ' . t('(Relative)');
      }
      return $output;
    }
    // If not exposed, display the value.
    $output = '';
    if (in_array($this->operator, $this->operator_values(2))) {
      $min = check_plain(!empty($this->options['default_date']) ? $this->options['default_date'] : $this->options['value']['min']);
      $max = check_plain(!empty($this->options['default_to_date']) ? $this->options['default_to_date'] : $this->options['value']['max']);
      $output .= t('@min and @max', array('@min' => $min, '@max' => $max));
    }
    else {
      $output .= check_plain(!empty($this->options['default_date']) ? $this->options['default_date'] : $this->options['value']['value']);
    }
    if (!empty($this->options['value']['type']) && $this->options['value']['type'] == 'relative') {
      $output .= ' ' . t('(Relative)');
    }
    return $output;
  }

  /**
   * Validation hook for exposed filters that use the select widget.
   *
   * This is to ensure the the user completes all parts of the date not just some parts. Only needed for the select widget.
   */
  function date_select_validate(&$form, &$form_state) {
    // If there are no values just return.
    if (empty($form['value']) && empty($form['min'])) {
      return;
    }
    // Determine granularity from the date format.
    $granularity = array();
    if (!empty($form['value']['#date_format'])) {
      $granularity = date_format_order($form['value']['#date_format']);
    }
    elseif (!empty($form['min']['#date_format'])) {
      $granularity = date_format_order($form['min']['#date_format']);
    }
    // If no granularity is found (e.g., for relative date textfields), return.
    if (empty($granularity)) {
      return;
    }
    $filled = array();
    $value = backdrop_array_get_nested_value($form_state['input'], $form['#parents']);
    foreach ($granularity as $part) {
      if (isset($value['value']) && is_numeric($value['value'][$part])) {
        $filled[] = $part;
      }
    }
    if (!empty($filled) && count($filled) != count($granularity)) {
      $unfilled = array_diff($granularity, $filled);
      foreach ($unfilled as $part) {
        switch ($part) {
          case 'year':
            form_error($form['value'][$part], t('Please choose a year.'));
            break;

          case 'month':
            form_error($form['value'][$part], t('Please choose a month.'));
            break;

          case 'day':
            form_error($form['value'][$part], t('Please choose a day.'));
            break;

          case 'hour':
            form_error($form['value'][$part], t('Please choose an hour.'));
            break;

          case 'minute':
            form_error($form['value'][$part], t('Please choose a minute.'));
            break;

          case 'second':
            form_error($form['value'][$part], t('Please choose a second.'));
            break;
        }
      }
    }
  }
}
