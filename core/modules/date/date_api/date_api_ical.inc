<?php

/**
 * @file
 * Parse iCal data.
 *
 * This file must be included when these functions are needed.
 */

/**
 * Return an array of iCalendar information from an iCalendar file.
 *
 *   No timezone adjustment is performed in the import since the timezone
 *   conversion needed will vary depending on whether the value is being
 *   imported into the database (when it needs to be converted to UTC), is being
 *   viewed on a site that has user-configurable timezones (when it needs to be
 *   converted to the user's timezone), if it needs to be converted to the
 *   site timezone, or if it is a date without a timezone which should not have
 *   any timezone conversion applied.
 *
 *   Properties that have dates and times are converted to sub-arrays like:
 *      'datetime'   => date in YYYY-MM-DD HH:MM format, not timezone adjusted
 *      'all_day'    => whether this is an all-day event
 *      'tz'         => the timezone of the date, could be blank for absolute
 *                      times that should get no timezone conversion.
 *
 *   Exception dates can have muliple values and are returned as arrays
 *   like the above for each exception date.
 *
 *   Most other properties are returned as PROPERTY => VALUE.
 *
 *   Each item in the VCALENDAR will return an array like:
 *   [0] => Array (
 *     [TYPE] => VEVENT
 *     [UID] => 104
 *     [SUMMARY] => An example event
 *     [URL] => http://example.com/node/1
 *     [DTSTART] => Array (
 *       [datetime] => 1997-09-07 09:00:00
 *       [all_day] => 0
 *       [tz] => US/Eastern
 *     )
 *     [DTEND] => Array (
 *       [datetime] => 1997-09-07 11:00:00
 *       [all_day] => 0
 *       [tz] => US/Eastern
 *     )
 *     [RRULE] => Array (
 *       [FREQ] => Array (
 *         [0] => MONTHLY
 *       )
 *       [BYDAY] => Array (
 *         [0] => 1SU
 *         [1] => -1SU
 *       )
 *     )
 *     [EXDATE] => Array (
 *       [0] = Array (
 *         [datetime] => 1997-09-21 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *       [1] = Array (
 *         [datetime] => 1997-10-05 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *     )
 *     [RDATE] => Array (
 *       [0] = Array (
 *         [datetime] => 1997-09-21 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *       [1] = Array (
 *         [datetime] => 1997-10-05 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *     )
 *   )
 *
 * @todo
 *   figure out how to handle this if subgroups are nested,
 *   like a VALARM nested inside a VEVENT.
 *
 * @param string $filename
 *   Location (local or remote) of a valid iCalendar file.
 *
 * @return array
 *   An array with all the elements from the ical.
 */
function date_ical_import($filename) {
  // Fetch the iCal data. If file is a URL, use backdrop_http_request. fopen
  // isn't always configured to allow network connections.
  if (substr($filename, 0, 4) == 'http') {
    // Fetch the ical data from the specified network location.
    $icaldatafetch = backdrop_http_request($filename);
    // Check the return result.
    if ($icaldatafetch->error) {
      watchdog('date ical', 'HTTP Request Error importing %filename: @error', array('%filename' => $filename, '@error' => $icaldatafetch->error));
      return FALSE;
    }
    // Break the return result into one array entry per lines.
    $icaldatafolded = explode("\n", $icaldatafetch->data);
  }
  else {
    $icaldatafolded = @file($filename, FILE_IGNORE_NEW_LINES);
    if ($icaldatafolded === FALSE) {
      watchdog('date ical', 'Failed to open file: %filename', array('%filename' => $filename));
      return FALSE;
    }
  }
  // Verify this is iCal data.
  if (trim($icaldatafolded[0]) != 'BEGIN:VCALENDAR') {
    watchdog('date ical', 'Invalid calendar file: %filename', array('%filename' => $filename));
    return FALSE;
  }
  return date_ical_parse($icaldatafolded);
}

/**
 * Returns an array of iCalendar information from an iCalendar file.
 *
 * As date_ical_import() but different param.
 *
 * @param array $icaldatafolded
 *   An array of lines from an ical feed.
 *
 * @return array
 *   An array with all the elements from the ical.
 */
function date_ical_parse($icaldatafolded = array()) {
  $items = array();

  // Verify this is iCal data.
  if (trim($icaldatafolded[0]) != 'BEGIN:VCALENDAR') {
    watchdog('date ical', 'Invalid calendar file.');
    return FALSE;
  }

  // "Unfold" wrapped lines.
  $icaldata = array();
  foreach ($icaldatafolded as $line) {
    $out = array();
    // See if this looks like the beginning of a new property or value. If not,
    // it is a continuation of the previous line. The regex is to ensure that
    // wrapped QUOTED-PRINTABLE data is kept intact.
    if (!preg_match('/([A-Z]+)[:;](.*)/', $line, $out)) {
      // Trim up to 1 leading space from wrapped line per iCalendar standard.
      $line = array_pop($icaldata) . (ltrim(substr($line, 0, 1)) . substr($line, 1));
    }
    $icaldata[] = $line;
  }
  unset($icaldatafolded);

  // Parse the iCal information.
  $parents = array();
  $subgroups = array();
  $vcal = '';
  foreach ($icaldata as $line) {
    $line = trim($line);
    $vcal .= $line . "\n";
    // Deal with begin/end tags separately.
    if (preg_match('/(BEGIN|END):V(\S+)/', $line, $matches)) {
      $closure = $matches[1];
      $type = 'V' . $matches[2];
      if ($closure == 'BEGIN') {
        array_push($parents, $type);
        array_push($subgroups, array());
      }
      elseif ($closure == 'END') {
        end($subgroups);
        $subgroup = &$subgroups[key($subgroups)];
        switch ($type) {
          case 'VCALENDAR':
            if (prev($subgroups) == FALSE) {
              $items[] = array_pop($subgroups);
            }
            else {
              $parent[array_pop($parents)][] = array_pop($subgroups);
            }
            break;

          // Add the timezones in with their index their TZID.
          case 'VTIMEZONE':
            $subgroup = end($subgroups);
            $id = $subgroup['TZID'];
            unset($subgroup['TZID']);

            // Append this subgroup onto the one above it.
            prev($subgroups);
            $parent = &$subgroups[key($subgroups)];

            $parent[$type][$id] = $subgroup;

            array_pop($subgroups);
            array_pop($parents);
            break;

          // Do some fun stuff with durations and all_day events and then append
          // to parent.
          case 'VEVENT':
          case 'VALARM':
          case 'VTODO':
          case 'VJOURNAL':
          case 'VVENUE':
          case 'VFREEBUSY':
          default:
            // Can't be sure whether DTSTART is before or after DURATION, so
            // parse DURATION at the end.
            if (isset($subgroup['DURATION'])) {
              date_ical_parse_duration($subgroup, 'DURATION');
            }
            // Add a top-level indication for the 'All day' condition. Leave it
            // in the individual date components, too, so it is always available
            // even when you are working with only a portion of the VEVENT
            // array, like in Feed API parsers.
            $subgroup['all_day'] = FALSE;

            // iCal spec states 'The "DTEND" property for a "VEVENT" calendar
            // component specifies the non-inclusive end of the event'. Adjust
            // multi-day events to remove the extra day because the Date code
            // assumes the end date is inclusive.
            if (!empty($subgroup['DTEND']) && (!empty($subgroup['DTEND']['all_day']))) {
              // Make the end date one day earlier.
              $date = new DateObject($subgroup['DTEND']['datetime'] . ' 00:00:00', $subgroup['DTEND']['tz']);
              date_modify($date, '-1 day');
              $subgroup['DTEND']['datetime'] = date_format($date, 'Y-m-d');
            }
            // If a start datetime is defined AND there is no definition for
            // the end datetime THEN make the end datetime equal the start
            // datetime and if it is an all day event define the entire event
            // as a single all day event.
            if (!empty($subgroup['DTSTART']) &&
               (empty($subgroup['DTEND']) && empty($subgroup['RRULE']) && empty($subgroup['RRULE']['COUNT']))) {
              $subgroup['DTEND'] = $subgroup['DTSTART'];
            }
            // Add this element to the parent as an array under the component
            // name.
            if (!empty($subgroup['DTSTART']['all_day'])) {
              $subgroup['all_day'] = TRUE;
            }
            // Add this element to the parent as an array under the.
            prev($subgroups);
            $parent = &$subgroups[key($subgroups)];

            $parent[$type][] = $subgroup;

            array_pop($subgroups);
            array_pop($parents);
            break;
        }
      }
    }
    // Handle all other possibilities.
    else {
      // Grab current subgroup.
      end($subgroups);
      $subgroup = &$subgroups[key($subgroups)];

      // Split up the line into nice pieces for PROPERTYNAME,
      // PROPERTYATTRIBUTES, and PROPERTYVALUE.
      preg_match('/([^;:]+)(?:;([^:]*))?:(.+)/', $line, $matches);
      $name = !empty($matches[1]) ? strtoupper(trim($matches[1])) : '';
      $field = !empty($matches[2]) ? $matches[2] : '';
      $data = !empty($matches[3]) ? $matches[3] : '';
      $parse_result = '';

      switch ($name) {
        // Keep blank lines out of the results.
        case '':
          break;

        // Lots of properties have date values that must be parsed out.
        case 'CREATED':
        case 'LAST-MODIFIED':
        case 'DTSTART':
        case 'DTEND':
        case 'DTSTAMP':
        case 'FREEBUSY':
        case 'DUE':
        case 'COMPLETED':
          $parse_result = date_ical_parse_date($field, $data);
          break;

        case 'EXDATE':
        case 'RDATE':
          $parse_result = date_ical_parse_exceptions($field, $data);
          break;

        case 'TRIGGER':
          // A TRIGGER can either be a date or in the form -PT1H.
          if (!empty($field)) {
            $parse_result = date_ical_parse_date($field, $data);
          }
          else {
            $parse_result = array('DATA' => $data);
          }
          break;

        case 'DURATION':
          // Can't be sure whether DTSTART is before or after DURATION in
          // the VEVENT, so store the data and parse it at the end.
          $parse_result = array('DATA' => $data);
          break;

        case 'RRULE':
        case 'EXRULE':
          $parse_result = date_ical_parse_rrule($field, $data);
          break;

        case 'STATUS':
        case 'SUMMARY':
        case 'DESCRIPTION':
          $parse_result = date_ical_parse_text($field, $data);
          break;

        case 'LOCATION':
          $parse_result = date_ical_parse_location($field, $data);
          break;

        // For all other properties, just store the property and the value.
        // This can be expanded on in the future if other properties should
        // be given special treatment.
        default:
          $parse_result = $data;
          break;
      }

      // Store the result of our parsing.
      $subgroup[$name] = $parse_result;
    }
  }
  return $items;
}

/**
 * Parses a ical date element.
 *
 * Possible formats to parse include:
 *   PROPERTY:YYYYMMDD[T][HH][MM][SS][Z]
 *   PROPERTY;VALUE=DATE:YYYYMMDD[T][HH][MM][SS][Z]
 *   PROPERTY;VALUE=DATE-TIME:YYYYMMDD[T][HH][MM][SS][Z]
 *   PROPERTY;TZID=XXXXXXXX;VALUE=DATE:YYYYMMDD[T][HH][MM][SS]
 *   PROPERTY;TZID=XXXXXXXX:YYYYMMDD[T][HH][MM][SS]
 *
 *   The property and the colon before the date are removed in the import
 *   process above and we are left with $field and $data.
 *
 * @param string $field
 *   The text before the colon and the date, i.e.
 *   ';VALUE=DATE:', ';VALUE=DATE-TIME:', ';TZID='
 * @param string $data
 *   The date itself, after the colon, in the format YYYYMMDD[T][HH][MM][SS][Z]
 *   'Z', if supplied, means the date is in UTC.
 *
 * @return array
 *   $items array, consisting of:
 *      'datetime'   => date in YYYY-MM-DD HH:MM format, not timezone adjusted
 *      'all_day'    => whether this is an all-day event with no time
 *      'tz'         => the timezone of the date, could be blank if the ical
 *                      has no timezone; the ical specs say no timezone
 *                      conversion should be done if no timezone info is
 *                      supplied
 * @todo
 *   Another option for dates is the format PROPERTY;VALUE=PERIOD:XXXX. The
 *   period may include a duration, or a date and a duration, or two dates, so
 *   would have to be split into parts and run through date_ical_parse_date()
 *   and date_ical_parse_duration(). This is not commonly used, so ignored for
 *   now. It will take more work to figure how to support that.
 */
function date_ical_parse_date($field, $data) {

  $items = array('datetime' => '', 'all_day' => '', 'tz' => '');
  if (empty($data)) {
    return $items;
  }
  // Make this a little more whitespace independent.
  $data = trim($data);

  // Turn the properties into a nice indexed array of
  // array(PROPERTYNAME => PROPERTYVALUE);
  $field_parts = preg_split('/[;:]/', $field);
  $properties = array();
  foreach ($field_parts as $part) {
    if (strpos($part, '=') !== FALSE) {
      $tmp = explode('=', $part);
      $properties[$tmp[0]] = $tmp[1];
    }
  }

  // Make this a little more whitespace independent.
  $data = trim($data);

  // Record if a time has been found.
  $has_time = FALSE;

  // If a format is specified, parse it according to that format.
  if (isset($properties['VALUE'])) {
    switch ($properties['VALUE']) {
      case 'DATE':
        preg_match(DATE_REGEX_ICAL_DATE, $data, $regs);
        // Date.
        $datetime = date_pad($regs[1]) . '-' . date_pad($regs[2]) . '-' . date_pad($regs[3]);
        break;

      case 'DATE-TIME':
        preg_match(DATE_REGEX_ICAL_DATETIME, $data, $regs);
        // Date.
        $datetime = date_pad($regs[1]) . '-' . date_pad($regs[2]) . '-' . date_pad($regs[3]);
        // Time.
        $datetime .= ' ' . date_pad($regs[4]) . ':' . date_pad($regs[5]) . ':' . date_pad($regs[6]);
        $has_time = TRUE;
        break;
    }
  }
  // If no format is specified, attempt a loose match.
  else {
    preg_match(DATE_REGEX_LOOSE, $data, $regs);
    if (!empty($regs) && count($regs) > 2) {
      // Date.
      $datetime = date_pad($regs[1]) . '-' . date_pad($regs[2]) . '-' . date_pad($regs[3]);
      if (isset($regs[4])) {
        $has_time = TRUE;
        // Time.
        $datetime .= ' ' . (!empty($regs[5]) ? date_pad($regs[5]) : '00') .
         ':' . (!empty($regs[6]) ? date_pad($regs[6]) : '00') .
         ':' . (!empty($regs[7]) ? date_pad($regs[7]) : '00');
      }
    }
  }

  // Use timezone if explicitly declared.
  if (isset($properties['TZID'])) {
    $tz = $properties['TZID'];
    // Fix alternatives like US-Eastern which should be US/Eastern.
    $tz = str_replace('-', '/', $tz);
    // Unset invalid timezone names.
    module_load_include('inc', 'date_api', 'date_api.admin');
    $tz = _date_timezone_replacement($tz);
    if (!date_timezone_is_valid($tz)) {
      $tz = '';
    }
  }
  // If declared as UTC with terminating 'Z', use that timezone.
  elseif (strpos($data, 'Z') !== FALSE) {
    $tz = 'UTC';
  }
  // Otherwise this date is floating.
  else {
    $tz = '';
  }

  $items['datetime'] = $datetime;
  $items['all_day'] = $has_time ? FALSE : TRUE;
  $items['tz'] = $tz;
  return $items;
}

/**
 * Parse an ical repeat rule.
 *
 * @return array
 *   Array in the form of PROPERTY => array(VALUES)
 *   PROPERTIES include FREQ, INTERVAL, COUNT, BYDAY, BYMONTH, BYYEAR, UNTIL
 */
function date_ical_parse_rrule($field, $data) {
  $data = preg_replace("/RRULE.*:/", '', $data);
  $items = array('DATA' => $data);
  $rrule = explode(';', $data);
  foreach ($rrule as $key => $value) {
    $param = explode('=', $value);
    // Must be some kind of invalid data.
    if (count($param) != 2) {
      continue;
    }
    if ($param[0] == 'UNTIL') {
      $values = date_ical_parse_date('', $param[1]);
    }
    else {
      $values = explode(',', $param[1]);
    }
    // Treat items differently if they have multiple or single values.
    if (in_array($param[0], array('FREQ', 'INTERVAL', 'COUNT', 'WKST'))) {
      $items[$param[0]] = $param[1];
    }
    else {
      $items[$param[0]] = $values;
    }
  }
  return $items;
}

/**
 * Parse exception dates (can be multiple values).
 *
 * @return array
 *   an array of date value arrays.
 */
function date_ical_parse_exceptions($field, $data) {
  $data = str_replace($field . ':', '', $data);
  $items = array('DATA' => $data);
  $ex_dates = explode(',', $data);
  foreach ($ex_dates as $ex_date) {
    $items[] = date_ical_parse_date('', $ex_date);
  }
  return $items;
}

/**
 * Parses the duration of the event.
 *
 * Example:
 *  DURATION:PT1H30M
 *  DURATION:P1Y2M
 *
 * @param array $subgroup
 *   Array of other values in the vevent so we can check for DTSTART.
 */
function date_ical_parse_duration(&$subgroup, $field = 'DURATION') {
  $items = $subgroup[$field];
  $data  = $items['DATA'];
  preg_match('/^P(\d{1,4}[Y])?(\d{1,2}[M])?(\d{1,2}[W])?(\d{1,2}[D])?([T]{0,1})?(\d{1,2}[H])?(\d{1,2}[M])?(\d{1,2}[S])?/', $data, $duration);
  $items['year'] = isset($duration[1]) ? str_replace('Y', '', $duration[1]) : '';
  $items['month'] = isset($duration[2]) ? str_replace('M', '', $duration[2]) : '';
  $items['week'] = isset($duration[3]) ? str_replace('W', '', $duration[3]) : '';
  $items['day'] = isset($duration[4]) ? str_replace('D', '', $duration[4]) : '';
  $items['hour'] = isset($duration[6]) ? str_replace('H', '', $duration[6]) : '';
  $items['minute'] = isset($duration[7]) ? str_replace('M', '', $duration[7]) : '';
  $items['second'] = isset($duration[8]) ? str_replace('S', '', $duration[8]) : '';
  $start_date = array_key_exists('DTSTART', $subgroup) ? $subgroup['DTSTART']['datetime'] : date_format(date_now(), DATE_FORMAT_ISO);
  $timezone = array_key_exists('DTSTART', $subgroup) ? $subgroup['DTSTART']['tz'] : config_get('system.date','default_timezone');
  if (empty($timezone)) {
    $timezone = 'UTC';
  }
  $date = new DateObject($start_date, $timezone);
  $date2 = clone($date);
  foreach ($items as $item => $count) {
    if ($count > 0) {
      date_modify($date2, '+' . $count . ' ' . $item);
    }
  }
  $format = isset($subgroup['DTSTART']['type']) && $subgroup['DTSTART']['type'] == 'DATE' ? 'Y-m-d' : 'Y-m-d H:i:s';
  $subgroup['DTEND'] = array(
    'datetime' => date_format($date2, DATE_FORMAT_DATETIME),
    'all_day' => isset($subgroup['DTSTART']['all_day']) ? $subgroup['DTSTART']['all_day'] : 0,
    'tz' => $timezone,
  );
  $duration = date_format($date2, 'U') - date_format($date, 'U');
  $subgroup['DURATION'] = array('DATA' => $data, 'DURATION' => $duration);
}

/**
 * Parse and clean up ical text elements.
 */
function date_ical_parse_text($field, $data) {
  if (strstr($field, 'QUOTED-PRINTABLE')) {
    $data = quoted_printable_decode($data);
  }
  // Strip line breaks within element.
  $data = str_replace(array("\r\n ", "\n ", "\r "), '', $data);
  // Put in line breaks where encoded.
  $data = str_replace(array("\\n", "\\N"), "\n", $data);
  // Remove other escaping.
  $data = stripslashes($data);
  return $data;
}

/**
 * Parse location elements.
 *
 * Catch situations like the upcoming.org feed that uses
 * LOCATION;VENUE-UID="http://upcoming.yahoo.com/venue/104/":111 First Street...
 * or more normal LOCATION;UID=123:111 First Street...
 * Upcoming feed would have been improperly broken on the ':' in http://
 * so we paste the $field and $data back together first.
 *
 * Use non-greedy check for ':' in case there are more of them in the address.
 */
function date_ical_parse_location($field, $data) {
  if (preg_match('/UID=[?"](.+)[?"][*?:](.+)/', $field . ':' . $data, $matches)) {
    $location = array();
    $location['UID'] = $matches[1];
    $location['DESCRIPTION'] = stripslashes($matches[2]);
    return $location;
  }
  else {
    // Remove other escaping.
    $location = stripslashes($data);
    return $location;
  }
}

/**
 * Return a date object for the ical date, adjusted to its local timezone.
 *
 * @param array $ical_date
 *   An array of ical date information created in the ical import.
 * @param string $to_tz
 *   The timezone to convert the date's value to.
 *
 * @return object
 *   A timezone-adjusted date object.
 */
function date_ical_date($ical_date, $to_tz = FALSE) {

  // If the ical date has no timezone, must assume it is stateless
  // so treat it as a local date.
  if (empty($ical_date['datetime'])) {
    return NULL;
  }
  elseif (empty($ical_date['tz'])) {
    $from_tz = date_default_timezone();
  }
  else {
    $from_tz = $ical_date['tz'];
  }
  if (strlen($ical_date['datetime']) < 11) {
    $ical_date['datetime'] .= ' 00:00:00';
  }
  $date = new DateObject($ical_date['datetime'], new DateTimeZone($from_tz));

  if ($to_tz && $ical_date['tz'] != '' && $to_tz != $ical_date['tz']) {
    date_timezone_set($date, timezone_open($to_tz));
  }
  return $date;
}

/**
 * Escape #text elements for safe iCal use.
 *
 * @param string $text
 *   Text to escape
 *
 * @return string
 *   Escaped text
 */
function date_ical_escape_text($text) {
  $text = backdrop_html_to_text($text);
  $text = trim($text);
  // TODO Per #38130 the iCal specs don't want : and " escaped
  // but there was some reason for adding this in. Need to watch
  // this and see if anything breaks.
  // $text = str_replace('"', '\"', $text);
  // $text = str_replace(":", "\:", $text);
  $text = preg_replace("/\\\b/", "\\\\", $text);
  $text = str_replace(",", "\,", $text);
  $text = str_replace(";", "\;", $text);
  $text = str_replace("\n", "\\n ", $text);
  return trim($text);
}

/**
 * Build an iCal RULE from $form_values.
 *
 * @param array $form_values
 *   An array constructed like the one created by date_ical_parse_rrule().
 *     [RRULE] => Array (
 *       [FREQ] => Array (
 *         [0] => MONTHLY
 *       )
 *       [BYDAY] => Array (
 *         [0] => 1SU
 *         [1] => -1SU
 *       )
 *       [UNTIL] => Array (
 *         [datetime] => 1997-21-31 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *     )
 *     [EXDATE] => Array (
 *       [0] = Array (
 *         [datetime] => 1997-09-21 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *       [1] = Array (
 *         [datetime] => 1997-10-05 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *     )
 *     [RDATE] => Array (
 *       [0] = Array (
 *         [datetime] => 1997-09-21 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *       [1] = Array (
 *         [datetime] => 1997-10-05 09:00:00
 *         [all_day] => 0
 *         [tz] => US/Eastern
 *       )
 *     )
 */
function date_api_ical_build_rrule($form_values) {
  $rrule = '';
  if (empty($form_values) || !is_array($form_values)) {
    return $rrule;
  }

  // Grab the RRULE data and put them into iCal RRULE format.
  $rrule .= 'RRULE:FREQ=' . (!array_key_exists('FREQ', $form_values) ? 'DAILY' : $form_values['FREQ']);
  $rrule .= ';INTERVAL=' . (!array_key_exists('INTERVAL', $form_values) ? 1 : $form_values['INTERVAL']);

  // Unset the empty 'All' values.
  if (array_key_exists('BYDAY', $form_values) && is_array($form_values['BYDAY'])) {
    unset($form_values['BYDAY']['']);
  }
  if (array_key_exists('BYMONTH', $form_values) && is_array($form_values['BYMONTH'])) {
    unset($form_values['BYMONTH']['']);
  }
  if (array_key_exists('BYMONTHDAY', $form_values) && is_array($form_values['BYMONTHDAY'])) {
    unset($form_values['BYMONTHDAY']['']);
  }

  if (array_key_exists('BYDAY', $form_values) && is_array($form_values['BYDAY']) && $byday = implode(",", $form_values['BYDAY'])) {
    $rrule .= ';BYDAY=' . $byday;
  }
  if (array_key_exists('BYMONTH', $form_values) && is_array($form_values['BYMONTH']) && $bymonth = implode(",", $form_values['BYMONTH'])) {
    $rrule .= ';BYMONTH=' . $bymonth;
  }
  if (array_key_exists('BYMONTHDAY', $form_values) && is_array($form_values['BYMONTHDAY']) && $bymonthday = implode(",", $form_values['BYMONTHDAY'])) {
    $rrule .= ';BYMONTHDAY=' . $bymonthday;
  }
  // The UNTIL date is supposed to always be expressed in UTC.
  // The input date values may already have been converted to a date object on a
  // previous pass, so check for that.
  if (array_key_exists('UNTIL', $form_values) && array_key_exists('datetime', $form_values['UNTIL']) && !empty($form_values['UNTIL']['datetime'])) {
    // We only collect a date for UNTIL, but we need it to be inclusive, so
    // force it to a full datetime element at the last second of the day.
    if (!is_object($form_values['UNTIL']['datetime'])) {
      // If this is a date without time, give it time.
      if (strlen($form_values['UNTIL']['datetime']) < 11) {
        $granularity_options = backdrop_map_assoc(array(
          'year',
          'month',
          'day',
          'hour',
          'minute',
          'second',
        ));

        $form_values['UNTIL']['datetime'] .= ' 23:59:59';
        $form_values['UNTIL']['granularity'] = serialize($granularity_options);
        $form_values['UNTIL']['all_day'] = FALSE;
      }
      $until = date_ical_date($form_values['UNTIL'], 'UTC');
    }
    else {
      $until = $form_values['UNTIL']['datetime'];
    }
    $rrule .= ';UNTIL=' . date_format($until, DATE_FORMAT_ICAL) . 'Z';
  }
  // Our form doesn't allow a value for COUNT, but it may be needed by
  // modules using the API, so add it to the rule.
  if (array_key_exists('COUNT', $form_values)) {
    $rrule .= ';COUNT=' . $form_values['COUNT'];
  }

  // iCal rules presume the week starts on Monday unless otherwise specified,
  // so we'll specify it.
  if (array_key_exists('WKST', $form_values)) {
    $rrule .= ';WKST=' . $form_values['WKST'];
  }
  else {
    $rrule .= ';WKST=' . date_repeat_dow2day(config_get('system.date','first_day'));
  }

  // Exceptions dates go last, on their own line.
  // The input date values may already have been converted to a date
  // object on a previous pass, so check for that.
  if (isset($form_values['EXDATE']) && is_array($form_values['EXDATE'])) {
    $ex_dates = array();
    foreach ($form_values['EXDATE'] as $value) {
      if (!empty($value['datetime'])) {
        $date = !is_object($value['datetime']) ? date_ical_date($value, 'UTC') : $value['datetime'];
        $ex_date = !empty($date) ? date_format($date, DATE_FORMAT_ICAL) . 'Z' : '';
        if (!empty($ex_date)) {
          $ex_dates[] = $ex_date;
        }
      }
    }
    if (!empty($ex_dates)) {
      sort($ex_dates);
      $rrule .= chr(13) . chr(10) . 'EXDATE:' . implode(',', $ex_dates);
    }
  }
  elseif (!empty($form_values['EXDATE'])) {
    $rrule .= chr(13) . chr(10) . 'EXDATE:' . $form_values['EXDATE'];
  }

  // Exceptions dates go last, on their own line.
  if (isset($form_values['RDATE']) && is_array($form_values['RDATE'])) {
    $ex_dates = array();
    foreach ($form_values['RDATE'] as $value) {
      $date = !is_object($value['datetime']) ? date_ical_date($value, 'UTC') : $value['datetime'];
      $ex_date = !empty($date) ? date_format($date, DATE_FORMAT_ICAL) . 'Z' : '';
      if (!empty($ex_date)) {
        $ex_dates[] = $ex_date;
      }
    }
    if (!empty($ex_dates)) {
      sort($ex_dates);
      $rrule .= chr(13) . chr(10) . 'RDATE:' . implode(',', $ex_dates);
    }
  }
  elseif (!empty($form_values['RDATE'])) {
    $rrule .= chr(13) . chr(10) . 'RDATE:' . $form_values['RDATE'];
  }

  return $rrule;
}
