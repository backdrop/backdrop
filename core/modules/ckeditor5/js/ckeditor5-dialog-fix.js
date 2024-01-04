/**
 * @file
 * Fix conflict with jquery.ui focusin event handling in dialogs.
 */
(function ($) {

  'use strict';

  /**
   * Override the _allowInteraction() extension point.
   *
   * @see https://api.jqueryui.com/dialog/#method-_allowInteraction
   * @see https://bugs.jqueryui.com/ticket/9087/
   */
  $.widget('ui.dialog', $.ui.dialog, {
    _allowInteraction: function _allowInteraction(event) {
      // The CKEditor 5 balloon toolbar is outside the modal container, by
      // specifying CKEditor elements as allowed-interaction outside the modal,
      // the balloon buttons can be clicked. All editor form elements, like
      // buttons in balloon toolbars get the "ck" class.
      if ($(event.target).hasClass('ck')) {
        return true;
      }
      return this._super(event);
    }
  });

})(jQuery);
