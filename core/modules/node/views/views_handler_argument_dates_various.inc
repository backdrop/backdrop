<?php

/**
 * @file
 * Handlers for various date arguments.
 *
 * @ingroup views_argument_handlers
 */

/**
 * Argument handler for a full date (CCYYMMDD)
 */
class views_handler_argument_node_created_fulldate extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->format = 'F j, Y';
    $this->arg_format = 'Ymd';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateFormatSql($this->arg_format, "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return format_date(strtotime($created . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    return format_date(strtotime($this->argument . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }
}

/**
 * Argument handler for a year (CCYY)
 */
class views_handler_argument_node_created_year extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->arg_format = 'Y';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateExtractSql('YEAR', "***table***.$this->real_field");
  }

}

/**
 * Argument handler for a year plus month (CCYYMM)
 */
class views_handler_argument_node_created_year_month extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->format = 'F Y';
    $this->arg_format = 'Ym';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateFormatSql($this->arg_format, "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return format_date(strtotime($created . "15" . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    return format_date(strtotime($this->argument . "15" . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }
}

/**
 * Argument handler for a month (MM)
 */
class views_handler_argument_node_created_month extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->format = 'F';
    $this->arg_format = 'm';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateExtractSql('MONTH', "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $month = str_pad($data->{$this->name_alias}, 2, '0', STR_PAD_LEFT);
    return format_date(strtotime("2005" . $month . "15" . " 00:00:00 UTC" ), 'custom', $this->format, 'UTC');
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    $month = str_pad($this->argument, 2, '0', STR_PAD_LEFT);
    return format_date(strtotime("2005" . $month . "15" . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }

  function summary_argument($data) {
    // Make sure the argument contains leading zeroes.
    return str_pad($data->{$this->base_alias}, 2, '0', STR_PAD_LEFT);
  }
}

/**
 * Argument handler for a day (DD)
 */
class views_handler_argument_node_created_day extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->format = 'j';
    $this->arg_format = 'd';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateExtractSql('DAY', "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $day = str_pad($data->{$this->name_alias}, 2, '0', STR_PAD_LEFT);
    // strtotime respects server timezone, so we need to set the time fixed as utc time
    return format_date(strtotime("2005" . "05" . $day . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    $day = str_pad($this->argument, 2, '0', STR_PAD_LEFT);
    return format_date(strtotime("2005" . "05" . $day . " 00:00:00 UTC"), 'custom', $this->format, 'UTC');
  }

  function summary_argument($data) {
    // Make sure the argument contains leading zeroes.
    return str_pad($data->{$this->base_alias}, 2, '0', STR_PAD_LEFT);
  }
}

/**
 * Argument handler for a week.
 */
class views_handler_argument_node_created_week extends views_handler_argument_date {
  /**
   * Constructor implementation
   */
  function construct() {
    parent::construct();
    $this->arg_format = 'w';
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->formula = $this->query->database_connection()->dateExtractSql('WEEK', "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return t('Week @week', array('@week' => $created));
  }
}
