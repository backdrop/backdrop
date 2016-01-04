/**
 * @file system.admin.themes.js
 *
 * Behaviors for themes admin
 */

(function ($) {

"use strict";

/**
 * Behavior for theme list
 *
 * Detect flexbox support for displaying our list of disabled themes
 */
Backdrop.behaviors.layoutDisplayEditor = {
  attach: function(context) {
    var $element = $(context).find('.system-themes-list-disabled');
    if ($element.length) {
      if ($element.css('flex-wrap')) {
        $element.addClass('flexbox');
      }
      else {
        $element.addClass('no-flexbox');
      }
    }
  }
};

/**
 * Behavior for showing a list of disabled themes.
 *
 * Detect flexbox support for displaying our list of themes with vertical
 * height matching for each row of layout template icons.
 */
Backdrop.behaviors.disabledThemesListing = {
  attach: function(context) {
    var $element = $(context).find('.system-themes-list-disabled-wrapper');
    if ($element.length) {
      if ($element.css('flex-wrap')) {
        $element.addClass('flexbox');
      }
      else {
        $element.addClass('no-flexbox');
      }
    }
  }
};

})(jQuery);
