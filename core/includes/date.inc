<?php
/**
 * @file
 * Date API Constants
 */
/**
 * Set up some constants.
 *
 * Includes standard date types, format strings, strict regex strings for ISO
 * and DATETIME formats (seconds are optional).
 *
 * The loose regex will find any variety of ISO date and time, with or
 * without time, with or without dashes and colons separating the elements,
 * and with either a 'T' or a space separating date and time.
 */
define('DATE_ISO', 'date');
define('DATE_UNIX', 'datestamp');
define('DATE_DATETIME', 'datetime');
define('DATE_ARRAY', 'array');
define('DATE_OBJECT', 'object');
define('DATE_ICAL', 'ical');

define('DATE_FORMAT_ISO', "Y-m-d\TH:i:s");
define('DATE_FORMAT_UNIX', "U");
define('DATE_FORMAT_DATETIME', "Y-m-d H:i:s");
define('DATE_FORMAT_ICAL', "Ymd\THis");
define('DATE_FORMAT_ICAL_DATE', "Ymd");
define('DATE_FORMAT_DATE', 'Y-m-d');

define('DATE_REGEX_ISO', '/(\d{4})?(-(\d{2}))?(-(\d{2}))?([T\s](\d{2}))?(:(\d{2}))?(:(\d{2}))?/');
define('DATE_REGEX_DATETIME', '/(\d{4})-(\d{2})-(\d{2})\s(\d{2}):(\d{2}):?(\d{2})?/');
define('DATE_REGEX_LOOSE', '/(\d{4})-?(\d{1,2})-?(\d{1,2})([T\s]?(\d{2}):?(\d{2}):?(\d{2})?(\.\d+)?(Z|[\+\-]\d{2}:?\d{2})?)?/');
define('DATE_REGEX_ICAL_DATE', '/(\d{4})(\d{2})(\d{2})/');
define('DATE_REGEX_ICAL_DATETIME', '/(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})(Z)?/');

// There is actually no maximum and minimum year in PHP 5, but a date with
// a year less than 0 would result in negative ISO and DATETIME dates,
// like -1250-01-01T00:00:00, which probably won't make sense or work
// correctly anywhere.
define('DATE_MIN_YEAR', 1);
define('DATE_MAX_YEAR', 4000);
