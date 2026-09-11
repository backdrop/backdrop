/**
 * @file
 * Backwards compatibility layer for jQuery plugins used by Backdrop.
 *
 * Some utility functions have been removed in v4. We add them back, either as
 * they worked in v3, or as simple wrappers around native JavaScript code.
 * @see https://github.com/jquery/jquery/blob/3.x-stable/src/deprecated.js
 */
(function ($) {
  "use strict";

  $.isArray = Array.isArray;
  $.parseJSON = JSON.parse;
  $.isFunction = function (obj) {
    return typeof obj === 'function';
  };
  $.isNumeric = function (obj) {
    let type = typeof obj;
    return (type === 'number' || type === 'string') && !isNaN(obj - parseFloat(obj));
  };
  $.trim = function (obj) {
    if (typeof obj === 'string') {
      return obj.trim();
    }
    return '';
  };
})(jQuery);
