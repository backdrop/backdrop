<?php

/**
 * @file
 * Date Devel Generate code.
 */

/**
 * Implements hook_devel_generate().
 *
 * Included only when needed.
 */
function date_devel_generate($entity, $field, $instance, $bundle) {

  $entity_field = array();
  if (isset($instance['widget']['settings']['year_range'])) {
    $split = explode(':', $instance['widget']['settings']['year_range']);
    // Determine how much to go back and forward depending on whether a relative
    // number of years (with - or + sign) or an absolute year is given.
    $back = strpos($split[0], '-') === 0
      ? str_replace('-', '', $split[0])
      : date_format(date_now(), 'Y') - $split[0];
    $forward = strpos($split[1], '+') === 0
      ? str_replace('+', '', $split[1])
      : $split[1] - date_format(date_now(), 'Y');
  }
  else {
    $back = 2;
    $forward = 2;
  }
  // Pick a random year within the time range,
  // and a random second within that year.
  $year = date_format(date_now(), 'Y') - $back + mt_rand(0, ($forward + $back));
  $start = new DateObject($year . '-01-01 00:00:00', date_get_timezone_db($field['settings']['tz_handling']));
  $leap = date_format($start, 'L');
  $max_days = $leap ? 366 : 365;
  $seconds = mt_rand(0, ($max_days * 86400));
  date_modify($start, "+$seconds seconds");
  $increment = $instance['widget']['settings']['increment'];
  date_increment_round($start, $increment);

  // Modify End date by 1 hour to 3 days, shorter for repeating dates
  // longer for others.
  $start2 = clone($start);
  $max = !empty($field['settings']['repeat']) ? 720 : 4320;
  $max = 240;
  date_modify($start2, '+' . mt_rand(60, $max) . ' minutes');
  date_increment_round($start2, $increment);

  if ($field['settings']['tz_handling'] == 'date') {
    // Choose a random timezone.
    // Not all keys exist, so we have to check.
    $timezones = array_keys(date_timezone_names(TRUE));
    $key = mt_rand(0, count($timezones) - 1);
    if (!array_key_exists($key, $timezones)) {
      $timezone = date_default_timezone();
    }
    else {
      $timezone = $timezones[$key];
    }
  }
  else {
    $timezone = date_get_timezone($field['settings']['tz_handling']);
  }

  switch ($field['type']) {
    case 'date':
      $format = DATE_FORMAT_ISO;
      break;

    case 'datestamp':
      $format = DATE_FORMAT_UNIX;
      break;

    case 'datetime':
      $format = DATE_FORMAT_DATETIME;
      break;
  }
  $entity_field['value'] = date_format($start, $format);
  if ($field['settings']['todate']) {
    $entity_field['value2'] = date_format($start2, $format);
  }
  date_timezone_set($start, timezone_open($timezone));
  $entity_field['timezone'] = $timezone;
  $entity_field['offset'] = date_offset_get($start);
  date_timezone_set($start2, timezone_open($timezone));
  $entity_field['offset2'] = date_offset_get($start2);
  return $entity_field;

}
