INFORMATION FOR DEVELOPERS

Once the Date API is installed, all functions in the API are available to be
used anywhere by any module.

The API uses the PHP 5.2 date functions to create and manipulate dates.

Example, the following will create a date for the local value in one
timezone, adjust it to a different timezone, then return the offset in seconds
in the new timezone for the input date; The offset will be adjusted for both
the timezone difference and daylight savings time, if necessary:

$date = date_create('2007-03-11 02:00:00', timezone_open('America/Chicago'));
$chicago_time = date_format($date, 'Y-m-d H:i');

print 'At '. $chicago_time .' in Chicago, the timezone offset in seconds
  was '. date_offset_get($date);

date_timezone_set($date, timezone_open('Europe/Berlin');
$berlin_time = date_format($date, 'Y-m-d H:i');

print 'It was '. $berlin_time .' in Berlin when it
  was '. $chicago_time .' in Chicago.';
print 'At that time in Berlin, the timezone offset in seconds was
  '. date_offset_get($date);

A helper class is available, new DateObject($string, $timezone, $format), where
$string is a unixtimestamp, an ISO date, or a string like YYYY-MM-DD HH:MM:SS,
$timezone is the name of the timezone this date is in, and $format is the format
of date it is (DATE_FORMAT_UNIX, DATE_FORMAT_ISO, or DATE_FORMAT_DATETIME). It
creates and return a date object set to the right date and timezone.

Simpletest tests for these functions are included in the package.

Available functions include the following (more documentation is provided in
the files):

============================================================================
Preconfigured arrays
============================================================================
Both translated and untranslated values are available. The
date_week_days_ordered() function will shift an array of week day names so it
starts with the site's first day of the week, otherwise the weekday names start
with Sunday as the first value, which is the expected order for many php and sql
functions.

date_month_names();
date_month_names_abbr();
date_month_names_untranslated();
date_week_days();
date_week_days_abbr();
date_week_days_untranslated();
date_week_days_ordered();
date_years();
date_hours();
date_minutes();
date_seconds();
date_timezone_names();
date_ampm();

============================================================================
Miscellaneous date manipulation functions
============================================================================
Pre-defined constants and functions that will handle pre-1970 and post-2038
dates in both PHP 4 and PHP 5, in any OS. Dates can be converted from one
type to another and date parts can be extracted from any date type.

DATE_DATETIME
DATE_ISO
DATE_UNIX
DATE_ARRAY
DATE_OBJECT
DATE_ICAL

date_convert()
date_is_valid();
date_part_is_valid();
date_part_extract();

============================================================================
Date calculation and navigation
============================================================================
date_difference() will find the time difference between any two days, measured
in seconds, minutes, hours, days, months, weeks, or years.

date_days_in_month();
date_days_in_year();
date_weeks_in_year();
date_last_day_of_month();
date_day_of_week();
date_day_of_week_name();
date_difference();

============================================================================
Date regex and format helpers
============================================================================
Pre-defined constants, an array of date format strings and their
equivalent regex strings.

DATE_REGEX_LOOSE is a very loose regex that will pull date parts out
of an ISO date with or without separators, using either 'T' or a space
to separate date and time, and with or without time.

date_format_date() is similar to format_date(), except it takes a
date object instead of a timestamp as the first parameter.

DATE_FORMAT_ISO
DATE_FORMAT_DATETIME
DATE_FORMAT_UNIX
DATE_FORMAT_ICAL

DATE_REGEX_ISO
DATE_REGEX_DATETIME
DATE_REGEX_LOOSE

date_format_date();
date_short_formats();
date_medium_formats();
date_long_formats();
date_format_patterns();

============================================================================
Standardized ical parser and creator
============================================================================
The iCal parser is found in date_api_ical.inc, which is not included by default.
Include that file if you want to use these functions:

Complete rewrite of ical imports to parse vevents, vlocations, valarms,
and all kinds of timezone options and repeat rules for ical imports.
The function now sticks to parsing the ical into an array that can be used
in various ways. It no longer trys to convert timezones while parsing,
instead a date_ical_date_format() function is provided that can be used to
convert from the ical timezone to whatever timezone is desired in the
results. Repeat rules are parsed into an array which other modules can
manipulate however they like to create additional events from the results.

date_ical_export();
date_ical_import();
date_ical_date_format();

============================================================================
Helpers for portable date SQL
============================================================================
The SQL functions are found in date_api_sql.inc, which is not included by
default. Include that file if you want to use these functions:

date_sql();
date_server_zone_adj();
date_sql_concat();
date_sql_pad();

============================================================================
Date forms and validators
============================================================================
Reusable, configurable, self-validating FAPI date elements are found in
date_api_elements.inc, which is not included by default. Include it
if you want to use these elements. To use them, create a form element
and set the '#type' to one of the following:

date_select
 The date_select element will create a collection of form elements, with a
 separate select or textfield for each date part. The whole collection will
 get reformatted back into a date value of the requested type during validation.

date_text
 The date_text element will create a textfield that can contain a whole
 date or any part of a date as text. The user input value will be re-formatted
 back into a date value of the requested type during validation.

date_timezone
 The date_timezone element will create a drop-down selector to pick a
 timezone name.

The custom date elements require a few other pieces of information to work
correctly, like #date_format and #date_type. See the internal documentation
for more information.

============================================================================
Date Popup Module
============================================================================

A new module is included in the package that will enable a popup jQuery
calendar date picker and timepicker in date and time fields.

It is implemented as a custom form element, so set '#type' to 'date_popup'
to use this element. See the internal documentation for more information.

============================================================================
Date Repeat API
============================================================================

An API for repeating dates is available if installed. It can be used by
other modules to create a form element that will allow users to select
repeat rules and store those selections in an iCal RRULE string, and a
calculation function that will parse the RRULE and return an array of dates
that match those rules. The API is implemented in the Date module as a
new date widget if the Date Repeat API is installed.

============================================================================
RDF Integration
============================================================================

To make RDF easier to use, the base date themes (date_display_single and
date_display_range) have been expanded so they pass attributes and
RDF mappings for the field, if any, to the theme. If RDF is installed
and no other mappings are provided, the theme adds RDF information
to mark both the Start and End dates as 'xsd:dateTime' datatypes with the
property of 'dc:date'. This occurs in the theme preprocess layer, in
particular via the functions template_preprocess_date_display_single() and
template_preprocess_date_display_range().

To mark these as events instead, you could install the schemaorg
module, which will load the schema.org vocabulary. The mark the content type
that contains events as an 'Event', using the UI exposed by that
module and set the event start date field with the 'dateStart'
property and tag other fields in the content type with the appropriate
property types. The Date module theme will wrap the start and end
date output with appropriate markup.

If the result is not quite what you need, you should be able to implement your
own theme preprocess functions, e.g. MYTHEME_preprocess_date_display_single()
or MYTHEME_preprocess_date_display_range() and alter the attributes to use the
values you want.
