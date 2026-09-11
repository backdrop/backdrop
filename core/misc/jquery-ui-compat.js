/**
 * @file
 * Sets the compatibility flag for jQuery UI back to true.
 *
 * Core still uses "dialogClass" option, which is deprecated and since jQuery UI
 * version 1.14.0 this flag defaults to false.
 */
(function ($) {
  $.uiBackCompat = true;
})(jQuery);
