<?php
/**
 * @file
 * Theme and preprocess functions for output by Date module.
 */

/**
 * @addtogroup themeable
 * @{
 */

/**
 * Returns HTML for a date timezone element.
 */
function theme_date_timezone($variables) {
  $element = $variables['element'];
  $attributes = $element['#attributes'];

  // Wrap in a div with an ID for #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" class="container-inline-date">' . $element['#children'] . '</div>';
  }
  return '<div ' . backdrop_attributes($attributes) . '>' . theme('form_element', $element) . '</div>';
}

/**
 * Returns HTML for a date text element.
 */
function theme_date_text($variables) {
  $element = $variables['element'];

  // Wrap in a div with an ID for #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" class="container-inline-date">' . $element['#children'] . '</div>';
  }
  return theme('form_element', $element);
}

/**
 * Returns HTML for a "date_select" element, including all child elements.
 */
function theme_date_select($variables) {
  $element = $variables['element'];

  // Wrap in a div with an ID for #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" class="container-inline-date">' . $element['#children'] . '</div>';
  }
  return theme('form_element', $element);
}

/**
 * Returns HTML for a date select input form element.
 *
 * This is for an individual select element (e.g. Year, Month, Day). The
 * complete element is wrapped in theme_date_select().
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
 * Returns HTML for a date in the format 'time ago'.
 */
function theme_date_time_ago($variables) {
  $start_date = $variables['start_date'];
  $end_date = $variables['end_date'];
  $interval = !empty($variables['interval']) ? $variables['interval'] : 2;
  $display = isset($variables['interval_display']) ? $variables['interval_display'] : 'time ago';

  // If no date is sent, then return nothing.
  if (empty($start_date) || empty($end_date)) {
    return '';
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

  return '';
}

/**
 * Returns HTML for a date element formatted as a Start/End combination.
 *
 *  $entity->date_id
 *    If set, this will show only an individual date on a field with
 *    multiple dates. The value should be a string that contains
 *    the following values, separated with periods:
 *    - module name of the module adding the item
 *    - node nid
 *    - field name
 *    - delta value of the field to be displayed
 *    - other information the module's custom theme might need
 *
 *    Used by the calendar module and available for other uses.
 *    example: 'date.217.field_date.3.test'
 *
 *  $entity->date_repeat_show_all
 *    If true, tells the theme to show all the computed values of a repeating
 *    date. If not true or not set, only the start date and the repeat rule
 *    will be displayed.
 *
 *  $dates['format']
 *    The format string used on these dates
 *  $dates['value']['local']['object']
 *    The local date object for the Start date
 *  $dates['value2']['local']['object']
 *    The local date object for the End date
 *  $dates['value']['local']['datetime']
 *    The datetime value of the Start date database (GMT) value
 *  $dates['value2']['local']['datetime']
 *    The datetime value of the End date database (GMT) value
 *  $dates['value']['formatted']
 *    Formatted Start date, i.e. 'February 15, 2007 2:00 pm';
 *  $dates['value']['formatted_date']
 *    Only the date part of the formatted Start date
 *  $dates['value']['formatted_time']
 *    Only the time part of the formatted Start date
 *  $dates['value2']['formatted']
 *    Formatted End date, i.e. 'February 15, 2007 6:00 pm';
 *  $dates['value2']['formatted_date']
 *    Only the date part of the formatted End date
 *  $dates['value2']['formatted_time']
 *    Only the time part of the formatted End date
 */
function theme_date_display_combination($variables) {
  $entity_type = $variables['entity_type'];
  $entity      = $variables['entity'];
  $field       = $variables['field'];
  $instance    = $variables['instance'];
  $langcode    = $variables['langcode'];
  $item        = $variables['item'];
  $delta       = $variables['delta'];
  $display     = $variables['display'];
  $field_name  = $field['field_name'];
  $formatter   = $display['type'];
  $options     = $display['settings'];
  $dates       = $variables['dates'];
  $attributes  = $variables['attributes'];
  $precision   = date_granularity_precision($field['settings']['granularity']);
  $show_remaining_days = $variables['show_remaining_days'];

  $output = '';
  // If date_id is set for this field and delta doesn't match, don't display it.
  if (!empty($entity->date_id)) {
    foreach ((array) $entity->date_id as $key => $id) {
      list($module, $nid, $field_name, $item_delta, $other) = explode('.', $id . '.');
      if ($field_name == $field['field_name'] && isset($delta) && $item_delta != $delta) {
        return $output;
      }
    }
  }

  // If this is a full node or a pseudo node created by grouping multiple
  // values, see exactly which values are supposed to be visible.
  if (isset($entity->$field_name)) {
    $entity = date_prepare_entity($formatter, $entity_type, $entity, $field, $instance, $langcode, $item, $display);
    // Did the current value get removed by formatter settings?
    if (empty($entity->{$field_name}[$langcode][$delta])) {
      return $output;
    }
    // Adjust the $element values to match the changes.
    $element['#entity'] = $entity;
  }

  switch ($options['fromto']) {
    case 'value':
      $date1 = $dates['value']['formatted'];
      $date2 = $date1;
      break;

    case 'value2':
      $date2 = $dates['value2']['formatted'];
      $date1 = $date2;
      break;

    default:
      $date1 = $dates['value']['formatted'];
      $date2 = $dates['value2']['formatted'];
      break;
  }

  // Pull the timezone, if any, out of the formatted result and tack it back on
  // at the end, if it is in the current formatted date.
  $timezone = $dates['value']['formatted_timezone'];
  if (date_timezone_is_valid($timezone) && $date1 !== $date2) {
    // We only handle timezone identifiers.
    // Start and end can have different offsets (DST).
    // Removing an offset in seconds (Z) creates false positives.
    // And we only do this, if we really have a range,
    // single dates are left untouched.
    $date1 = str_replace($timezone, '', $date1);
    $date2 = str_replace($timezone, '', $date2);
    $timezone = ' ' . $timezone;
  }
  else {
    $timezone = '';
  }
  $time1 = preg_replace('`^([\(\[])`', '', $dates['value']['formatted_time']);
  $time1 = preg_replace('([\)\]]$)', '', $time1);
  $time2 = preg_replace('`^([\(\[])`', '', $dates['value2']['formatted_time']);
  $time2 = preg_replace('([\)\]]$)', '', $time2);

  // A date with a granularity of 'hour' has a time string that is an integer
  // value. We can't use that to replace time strings in formatted dates.
  $has_time_string = date_has_time($field['settings']['granularity']);
  if ($precision == 'hour') {
    $has_time_string = FALSE;
  }

  // Check remaining days.
  $show_remaining_days = '';
  if (!empty($variables['show_remaining_days'])) {
    // Create a date object from the timestamp.
    $date = new DateTime('@' . strtotime($variables['dates']['value']['formatted_iso']));
    // Using date_diff()->days allows to get the days diff, without having to
    // use complex math to calculate it. It also takes into account leap years.
    $remaining_days = date_diff($date, date_now())->days;
    // Show remaining days only for future events.
    if ($date->format('U') > REQUEST_TIME) {
      $show_remaining_days = theme('date_display_remaining', array(
        'remaining_days' => $remaining_days,
      ));
    }
  }

  // No date values, display nothing.
  if (empty($date1) && empty($date2)) {
    $output .= '';
  }
  // Start and End dates match or there is no End date, display a complete
  // single date.
  elseif ($date1 == $date2 || empty($date2)) {
    $output .= theme('date_display_single', array(
      'date' => $date1,
      'timezone' => $timezone,
      'attributes' => $attributes,
      'dates' => $dates,
      'show_remaining_days' => $show_remaining_days,
    ));
  }
  // Same day, different times, don't repeat the date but show both Start and
  // End times. We can NOT do this if the replacement value is an integer
  // instead of a time string.
  elseif ($has_time_string && $dates['value']['formatted_date'] == $dates['value2']['formatted_date']) {
    // Replace the original time with the start/end time in the formatted start
    // date. Make sure that parentheses or brackets wrapping the time will be
    // retained in the final result.
    $time = theme('date_display_range', array(
      'date1' => $time1,
      'date2' => $time2,
      'timezone' => $timezone,
      'attributes' => $attributes,
      'dates' => $dates,
      'show_remaining_days' => '',
    ));
    $replaced = str_replace($time1, $time, $date1);
    $output .= theme('date_display_single', array(
      'date' => $replaced,
      'timezone' => $timezone,
      'attributes' => array(),
      'dates' => $dates,
      'show_remaining_days' => $show_remaining_days,
    ));
  }
  // Different days, display both in their entirety.
  else {
    $output .= theme('date_display_range', array(
      'date1' => $date1,
      'date2' => $date2,
      'timezone' => $timezone,
      'attributes' => $attributes,
      'dates' => $dates,
      'show_remaining_days' => $show_remaining_days,
    ));
  }

  return $output;
}

/**
 * Returns HTML for a date element formatted as a single date.
 */
function theme_date_display_single($variables) {
  $date = $variables['date'];
  $attributes = $variables['attributes'];
  $show_remaining_days = isset($variables['show_remaining_days']) ? $variables['show_remaining_days'] : '';

  // Wrap the result with the attributes.
  $output = '<span class="date-display-single"' . backdrop_attributes($attributes) . '>' . $date . '</span>';

  // Add remaining message and return.
  return $output . $show_remaining_days;
}

/**
 * Returns HTML for a date element formatted as a range.
 */
function theme_date_display_range($variables) {
  $date1 = $variables['date1'];
  $date2 = $variables['date2'];
  $timezone = $variables['timezone'];
  $attributes_start = $variables['attributes_start'];
  $attributes_end = $variables['attributes_end'];
  $show_remaining_days = $variables['show_remaining_days'];

  $start_date = '<span class="date-display-start"' . backdrop_attributes($attributes_start) . '>' . $date1 . '</span>';
  $end_date = '<span class="date-display-end"' . backdrop_attributes($attributes_end) . '>' . $date2 . $timezone . '</span>';

  // Wrap the result with the attributes.
  $output = '<span class="date-display-range">' . t('!start-date to !end-date', array(
    '!start-date' => $start_date,
    '!end-date' => $end_date,
  )) . '</span>';

  // Add remaining message and return.
  return $output . $show_remaining_days;
}

/**
 * Returns HTML for a date element formatted as an interval.
 */
function theme_date_display_interval($variables) {
  $entity = $variables['entity'];
  $options = $variables['display']['settings'];
  $dates = $variables['dates'];
  $attributes = $variables['attributes'];

  // Get the formatter settings, either the default settings for this node type
  // or the View settings stored in $entity->date_info.
  if (!empty($entity->date_info) && !empty($entity->date_info->formatter_settings)) {
    $options = $entity->date_info->formatter_settings;
  }

  $time_ago_vars = array(
    'start_date' => $dates['value']['local']['object'],
    'end_date' => $dates['value2']['local']['object'],
    'interval' => $options['interval'],
    'interval_display' => $options['interval_display'],
  );

  if ($return = theme('date_time_ago', $time_ago_vars)) {
    return '<span class="date-display-interval"' . backdrop_attributes($attributes) . ">$return</span>";
  }
  else {
    return '';
  }
}

/**
 * Returns HTML for a start/end date combination on form.
 */
function theme_date_combo($variables) {
  $element = $variables['element'];

  // Group start/end items together in fieldset.
  $fieldset = array(
    '#title' => field_filter_xss(t($element['#title'])) . ' ' . ($element['#delta'] > 0 ? intval($element['#delta'] + 1) : ''),
    '#value' => '',
    '#description' => !empty($element['#fieldset_description']) ? $element['#fieldset_description'] : '',
    '#attributes' => array('class' => array('date-combo')),
    '#children' => $element['#children'],
  );
  // Add marker to required date fields.
  if ($element['#required']) {
    $fieldset['#title'] .= " " . theme('form_required_marker');
  }
  return theme('fieldset', array('element' => $fieldset));
}

/**
 * Returns HTML for the text/select options for date parts in a table.
 */
function theme_date_text_parts($variables) {
  $element = $variables['element'];
  $rows = array();
  foreach (date_granularity_names() as $key => $part) {
    if ($element[$key]['#type'] == 'hidden') {
      $rows[] = backdrop_render($element[$key]);
    }
    else {
      $rows[] = array(
        $part,
        backdrop_render($element[$key][0]),
        backdrop_render($element[$key][1]),
      );
    }
  }
  if ($element['year']['#type'] == 'hidden') {
    return implode($rows) . backdrop_render_children($element);
  }
  else {
    $header = array(t('Date part'), t('Select list'), t('Text field'));
    return theme('table', array('header' => $header, 'rows' => $rows)) . backdrop_render_children($element);
  }
}

/**
 * Format a date popup element.
 *
 * Use a class that will float date and time next to each other.
 */
function theme_date_popup($variables) {
  $element = $variables['element'];

  // Wrap in a div with an ID for #states.
  if (isset($element['#children'])) {
    $element['#children'] = '<div id="' . $element['#id'] . '" class="container-inline-date">' . $element['#children'] . '</div>';
  }
  return theme('form_element', $element);
}

/**
 * Returns HTML for remaining message.
 */
function theme_date_display_remaining($variables) {
  $remaining_days = $variables['remaining_days'];
  if ($remaining_days) {
    $show_remaining_text = format_plural($remaining_days, '1 day remaining to event.', '@count days remaining to event.');
  }
  else {
    $show_remaining_text = t('Less than 1 day remaining to event.');
  }

  return '<div class="date-display-remaining"><span class="date-display-remaining">' . $show_remaining_text . '</span></div>';
}

/**
 * Preprocessor for Date Views filter form.
 */
function template_preprocess_date_views_filter_form(&$variables) {
  $form = $variables['form'];
  $variables['date'] = backdrop_render($form['valuedate']);
  $variables['mindate'] = backdrop_render($form['mindate']);
  $variables['maxdate'] = backdrop_render($form['maxdate']);
  $variables['adjustment'] = backdrop_render($form['valueadjustment']);
  $variables['minadjustment'] = backdrop_render($form['minadjustment']);
  $variables['maxadjustment'] = backdrop_render($form['maxadjustment']);
  $variables['description'] = backdrop_render($form['description']) . backdrop_render($form);
}

/**
 * Implements template_preprocess_views_view().
 *
 * Display the pager above view content, below or both.
 */
function date_preprocess_views_view(&$variables) {
  $view = $variables['view'];
  if (!empty($view->date_info) && !empty($view->date_info->date_pager_position)) {
    switch ($view->date_info->date_pager_position) {
      case 'top':
        $variables['header'] .= $variables['pager'];
        $variables['pager'] = '';
        break;

      case 'both':
        $variables['header'] .= $variables['pager'];
        break;

      default:
        // Already on the bottom.
    }
  }
}

/**
 * Preprocess function for Date pager template.
 */
function template_preprocess_date_views_pager(&$variables) {
  backdrop_add_css(backdrop_get_path('module', 'date') . "/css/date-views.css");

  $plugin = $variables['plugin'];
  $input = $variables['input'];
  $view = $plugin->view;

  $variables['nav_title'] = '';
  $variables['next_url'] = '';
  $variables['prev_url'] = '';
  $variables['classes'] = array('date-nav-wrapper');

  if (empty($view->date_info) || empty($view->date_info->min_date)) {
    return;
  }
  $date_info = $view->date_info;
  // Make sure we have some sort of granularity.
  $granularity = !empty($date_info->granularity) ? $date_info->granularity : 'month';
  $pos = $date_info->date_arg_pos;
  if (!empty($input)) {
    $id = $plugin->options['date_id'];
    if (array_key_exists($id, $input) && !empty($input[$id])) {
      $view->args[$pos] = $input[$id];
    }
  }

  $next_args = $view->args;
  $prev_args = $view->args;
  $min_date = $date_info->min_date;
  $max_date = $date_info->max_date;

  // Set up the pager link format. Setting the block identifier
  // will force pager style links.
  if ((isset($date_info->date_pager_format) && $date_info->date_pager_format != 'clean') || !empty($date_info->mini)) {
    if (empty($date_info->block_identifier)) {
      $date_info->block_identifier = $date_info->pager_id;
    }
  }

  if (empty($date_info->hide_nav)) {
    $prev_date = $date_info->prev_date;
    $next_date = $date_info->next_date;

    $format = array('year' => 'Y', 'month' => 'Y-m', 'day' => 'Y-m-d', 'hour' => 'Y-m-d\TH');
    if (!empty($prev_date)) {
      switch ($granularity) {
        case 'week':
          $prev_week = date_week(date_format($prev_date, 'Y-m-d'));
          $prev_arg = date_format($prev_date, 'o-\W') . date_pad($prev_week);
          break;
        default:
          $prev_arg = date_format($prev_date, $format[$granularity]);
      }
      $prev_path = str_replace($date_info->date_arg, $prev_arg, $date_info->url);
      $prev_args[$pos] = $prev_arg;
      $variables['prev_url'] = date_pager_url($view, NULL, $prev_arg);
    }
    if (!empty($next_date)) {
      switch ($granularity) {
        case 'week':
          $next_week = date_week(date_format($next_date, 'Y-m-d'));
          $next_arg = date_format($next_date, 'o-\W') . date_pad($next_week);
          break;
        default:
          $next_arg = date_format($next_date, $format[$granularity]);
      }
      $next_path = str_replace($date_info->date_arg, $next_arg, $date_info->url);
      $next_args[$pos] = $next_arg;
      $variables['next_url'] = date_pager_url($view, NULL, $next_arg);
    }

    $variables['next_options'] = $variables['prev_options'] = array();
  }
  else {
    $next_path = '';
    $prev_path = '';
    $variables['next_url'] = '';
    $variables['prev_url'] = '';
    $variables['next_options'] = $variables['prev_options'] = array();
  }

  // Check whether navigation links would point to
  // a date outside the allowed range.
  if (!empty($next_date) && !empty($variables['next_url']) && date_format($next_date, 'Y') > $date_info->limit[1]) {
    $variables['next_url'] = '';
  }
  if (!empty($prev_date) && !empty($variables['prev_url']) && date_format($prev_date, 'Y') < $date_info->limit[0]) {
    $variables['prev_url'] = '';
  }
  $variables['prev_options'] += array('attributes' => array());
  $variables['next_options'] += array('attributes' => array());
  $prev_title = '';
  $next_title = '';

  // Build next/prev link titles.
  switch ($granularity) {
    case 'year':
      $prev_title = t('Navigate to previous year');
      $next_title = t('Navigate to next year');
      break;

    case 'month':
      $prev_title = t('Navigate to previous month');
      $next_title = t('Navigate to next month');
      break;

    case 'week':
      $prev_title = t('Navigate to previous week');
      $next_title = t('Navigate to next week');
      break;

    case 'day':
      $prev_title = t('Navigate to previous day');
      $next_title = t('Navigate to next day');
      break;
  }
  $variables['prev_options']['attributes'] += array('title' => $prev_title);
  $variables['next_options']['attributes'] += array('title' => $next_title);

  // Add nofollow for next/prev links.
  $variables['prev_options']['attributes'] += array('rel' => 'nofollow');
  $variables['next_options']['attributes'] += array('rel' => 'nofollow');

  // Need this so we can use '&laquo;' or images in the links.
  $variables['prev_options'] += array('html' => TRUE);
  $variables['next_options'] += array('html' => TRUE);

  $link = FALSE;
  // Month navigation titles are used as links in the block view.
  if (!empty($date_info->mini) && $granularity == 'month') {
    $link = TRUE;
  }
  $params = array(
    'granularity' => $granularity,
    'view' => $view,
    'link' => $link,
  );
  $nav_title = theme('date_nav_title', $params);
  $variables['nav_title'] = $nav_title;
  $variables['mini'] = !empty($date_info->mini);
}

/**
 * Theme the calendar title.
 */
function theme_date_nav_title($params) {
  $date_views_config = config('date_views.settings');
  $title  = '';
  $granularity = $params['granularity'];
  $view = $params['view'];
  $date_info = $view->date_info;
  $link = !empty($params['link']) ? $params['link'] : FALSE;
  $format = !empty($params['format']) ? $params['format'] : NULL;
  $format_with_year = $date_views_config->get("date_views_{$granularity}_format_with_year");
  $format_without_year = $date_views_config->get("date_views_{$granularity}_format_without_year");
  switch ($granularity) {
    case 'year':
      $title = $date_info->year;
      $date_arg = $date_info->year;
      break;

    case 'month':
      $format = !empty($format) ? $format : (empty($date_info->mini) ? $format_with_year : $format_without_year);
      $title = date_format_date($date_info->min_date, 'custom', $format);
      $date_arg = $date_info->year . '-' . date_pad($date_info->month);
      break;

    case 'day':
      $format = !empty($format) ? $format : (empty($date_info->mini) ? $format_with_year : $format_without_year);
      $title = date_format_date($date_info->min_date, 'custom', $format);
      $date_arg = $date_info->year;
      $date_arg .= '-';
      $date_arg .= date_pad($date_info->month);
      $date_arg .= '-';
      $date_arg .= date_pad($date_info->day);
      break;

    case 'week':
      $format = !empty($format) ? $format : (empty($date_info->mini) ? $format_with_year : $format_without_year);
      $title = t('Week of @date', array(
        '@date' => date_format_date($date_info->min_date, 'custom', $format),
      ));
      $date_arg = $date_info->year . '-W' . date_pad($date_info->week);
      break;
  }
  if (!empty($date_info->mini) || $link) {
    // Month navigation titles are used as links in the mini view.
    $attributes = array('title' => t('View full page month'));
    $url = date_pager_url($view, $granularity, $date_arg, TRUE);
    return l($title, $url, array('attributes' => $attributes));
  }
  else {
    return $title;
  }
}

/** @} End of addtogroup themeable */
