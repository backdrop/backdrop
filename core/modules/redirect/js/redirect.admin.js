/**
 * @file
 * Attaches behaviors for the Redirect module admin pages.
 */
(function ($) {

"use strict";

Backdrop.behaviors.redirectAdmin = {
  attach: function (context) {
    $(context).find('.redirect-list-tableselect input:checkbox').on('change', function() {
      var anyChecked = $('table.redirect-list-tableselect input:checkbox:checked').length;
      if (anyChecked) {
        $('fieldset.redirect-list-operations').show();
      }
      else {
        $('fieldset.redirect-list-operations').hide();
      }
    });
    $('fieldset.redirect-list-operations').hide();
  }
};

})(jQuery);
