<?php

/**
 * @file
 * Theme files for Date API.
 */

/**
 * Returns HTML for a date timezone element.
 */
function theme_date_timezone($variables) {
  $element = $variables['element'];
  $attributes = $element['#attributes'];
  $wrapper_attributes = array();
  // Add an wrapper to mimic the way a single value field works, for ease in
  // using #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" ' . backdrop_attributes($wrapper_attributes) . '>' . $element['#children'] . '</div>';
  }
  return '<div ' . backdrop_attributes($attributes) . '>' . theme('form_element', $element) . '</div>';
}

/**
 * Returns HTML for a date select element.
 */
function theme_date_select($variables) {
  $element = $variables['element'];
  $attributes = !empty($element['#wrapper_attributes']) ? $element['#wrapper_attributes'] : array('class' => array());
  $attributes['class'][] = 'container-inline-date';
  $wrapper_attributes = array('class' => array('date-padding'));
  $wrapper_attributes['class'][] = 'clearfix';
  // Add an wrapper to mimic the way a single value field works, for ease in
  // using #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" ' . backdrop_attributes($wrapper_attributes) . '>' . $element['#children'] . '</div>';
  }
  return '<div ' . backdrop_attributes($attributes) . '>' . theme('form_element', $element) . '</div>';
}

/**
 * Returns HTML for a date text element.
 */
function theme_date_text($variables) {
  $element = $variables['element'];
  $attributes = !empty($element['#wrapper_attributes']) ? $element['#wrapper_attributes'] : array('class' => array());
  $attributes['class'][] = 'container-inline-date';
  // If there is no description, the floating date elements need some extra
  // padding below them.
  $wrapper_attributes = array('class' => array('date-padding'));
  if (empty($element['date']['#description'])) {
    $wrapper_attributes['class'][] = 'clearfix';
  }
  // Add an wrapper to mimic the way a single value field works, for ease in
  // using #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" ' . backdrop_attributes($wrapper_attributes) . '>' . $element['#children'] . '</div>';
  }
  return '<div ' . backdrop_attributes($attributes) . '>' . theme('form_element', $element) . '</div>';
}

/**
 * Returns HTML for a date select input form element.
 */
function theme_date_select_element($variables) {
  $element = $variables['element'];
  $parents = $element['#parents'];
  $part = array_pop($parents);
  return '<div class="date-' . $part . '">' . theme('select', $element) . '</div>';
}

/**
 * Returns HTML for a date textfield input form element.
 */
function theme_date_textfield_element($variables) {
  $element = $variables['element'];
  $parents = $element['#parents'];
  $part = array_pop($parents);
  return '<div class="date-' . $part . '">' . theme('textfield', $element) . '</div>';
}

/**
 * Returns HTML for a 'hour' date part prefix.
 */
function theme_date_part_hour_prefix($variables) {
  $element = $variables['element'];
  if ($element['#date_label_position'] != 'above') {
    return '<span class="form-item date-spacer">&nbsp;-&nbsp;</span>';
  }
}

/**
 * Returns HTML for a 'minutes and seconds' date part prefix.
 */
function theme_date_part_minsec_prefix($variables) {
  $element = $variables['element'];
  if ($element['#date_label_position'] != 'above') {
    return '<span class="form-item date-spacer">:</span>';
  }
}

/**
 * Returns HTML for a date_select 'year' label.
 */
function theme_date_part_label_year($variables) {
  return t('Year', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'month' label.
 */
function theme_date_part_label_month($variables) {
  return t('Month', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'day' label.
 */
function theme_date_part_label_day($variables) {
  return t('Day', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'hour' label.
 */
function theme_date_part_label_hour($variables) {
  return t('Hour', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'minute' label.
 */
function theme_date_part_label_minute($variables) {
  return t('Minute', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'second' label.
 */
function theme_date_part_label_second($variables) {
  return t('Second', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'ampm' label.
 */
function theme_date_part_label_ampm($variables) {
  return '&nbsp;';
}

/**
 * Returns HTML for a date_select 'timezone' label.
 */
function theme_date_part_label_timezone($variables) {
  return t('Timezone');
}

/**
 * Returns HTML for a date_select 'date' label.
 */
function theme_date_part_label_date($variables) {
  return t('Date', array(), array('context' => 'datetime'));
}

/**
 * Returns HTML for a date_select 'time' label.
 */
function theme_date_part_label_time($variables) {
  return t('Time', array(), array('context' => 'datetime'));
}


/**
 * Returns HTML for a date block that looks like a mini calendar day.
 *
 * Pass in a date object already set to the right timezone, format as a calendar
 * page date. The calendar styling is created in CSS.
 */
function theme_date_calendar_day($variables) {
  $output = '';
  $date = $variables['date'];
  if (!empty($date)) {
    $output .= '<div class="date-calendar-day">';
    $output .= '<span class="month">' . date_format_date($date, 'custom', 'M') . '</span>';
    $output .= '<span class="day">' . date_format_date($date, 'custom', 'j') . '</span>';
    $output .= '<span class="year">' . date_format_date($date, 'custom', 'Y') . '</span>';
    $output .= '</div>';
  }
  return $output;
}

/**
 * Returns HTML for a date in the format 'time ago'.
 */
function theme_date_time_ago($variables) {
  $start_date = $variables['start_date'];
  $end_date = $variables['end_date'];
  $interval = !empty($variables['interval']) ? $variables['interval'] : 2;
  $display = isset($variables['interval_display']) ? $variables['interval_display'] : 'time ago';

  // If no date is sent, then return nothing.
  if (empty($start_date) || empty($end_date)) {
    return;
  }

  // Time to compare dates to.
  $now = date_format(date_now(), DATE_FORMAT_UNIX);
  $start = date_format($start_date, DATE_FORMAT_UNIX);

  // Will be positive for a datetime in the past (ago), and negative for a datetime in the future (hence).
  $time_diff = $now - $start;

  // Uses the same options used by Views format_interval.
  switch ($display) {
    case 'raw time ago':
      return format_interval($time_diff, $interval);

    case 'time ago':
      return t('%time ago', array('%time' => format_interval($time_diff, $interval)));

    case 'raw time hence':
      return format_interval(-$time_diff, $interval);

    case 'time hence':
      return t('%time hence', array('%time' => format_interval(-$time_diff, $interval)));

    case 'raw time span':
      return ($time_diff < 0 ? '-' : '') . format_interval(abs($time_diff), $interval);

    case 'inverse time span':
      return ($time_diff > 0 ? '-' : '') . format_interval(abs($time_diff), $interval);

    case 'time span':
      return t(($time_diff < 0 ? '%time hence' : '%time ago'), array('%time' => format_interval(abs($time_diff), $interval)));

  }
}
