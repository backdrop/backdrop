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
        console.log('there I fixed it');
        return true;
      }
      return $(event.target).closest('.cke_dialog').length || this._super(event);
    }
  });

})(jQuery);
