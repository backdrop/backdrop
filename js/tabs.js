// $Id$

/**
 * @file tabs.js
 * jQuery UI Tabs (Tabs 3)
 * 
 * This is nothing more than the pure jquery UI tabs implementation.
 */
(function($) {
  $.fn.viewsTabs = function(initial, options) {
    if (initial && initial.constructor == Object) { // shift arguments
      options = initial;
      initial = null;
    }
    options = options || {};

    // first get initial tab from options
    initial = initial && initial.constructor == Number && --initial || 0;

    // Views UI needs the wrapping DIV of the UL.
    this.parent().tabs({ selected: initial });
  };

  // chainable tabs methods
  $.each(['Add', 'Remove', 'Enable', 'Disable', 'Click', 'Load'], function(i, method) {
    $.fn['views' + method + 'Tab'] = function() {
      var args = arguments;
      return this.each(function() {
        var instance = $.ui.tabs.instances[this.UI_TABS_UUID];
        instance[method.toLowerCase()].apply(instance, args);
      });
    };
  });
  $.fn.viewsSelectedTab = function(returnElement) {
    var selected;
    if (returnElement) {

    } else {

    }
    return selected;
  };
})(jQuery);
