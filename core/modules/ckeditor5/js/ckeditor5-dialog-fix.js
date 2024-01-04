/**
 * @file
 * Fix conflict with jquery.ui focusin event handling in dialogs.
 */
(function ($) {

  'use strict';

  $.widget('ui.dialog', $.ui.dialog, {
    _allowInteraction: function _allowInteraction(event) {
      // All editor form elements, like buttons in balloon toolbars get the "ck"
      // class.
      if ($(event.target).hasClass('ck')) {
        return true;
      }
      return this._super(event);
    }
  });

})(jQuery);
