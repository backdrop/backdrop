/**
 * @file
 * Responsive Admin tabs.
 */

(function ($) {
"use strict";

Backdrop.behaviors.responsivePrimaryTabs = {
  attach: function(context, settings) {
    Backdrop.makeMenuResponsive(context, settings, 'ul.tabs.primary', 'li');
  }

}

})(jQuery);
