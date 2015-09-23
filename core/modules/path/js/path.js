/**
 * @file
 * Attaches behaviors for the Path module.
 */
(function ($) {

"use strict";

Backdrop.behaviors.pathFieldsetSummaries = {
  attach: function (context) {
    $(context).find('fieldset.path-form').backdropSetSummary(function (element) {
      var $element = $(element);
      var path = $element.find('[name="path[alias]"]').val();
      var automatic = $element.find('[name="path[auto]"]').prop('checked');

      if (automatic) {
        return Backdrop.t('Automatic alias');
      }
      if (path) {
        $('#edit-path-alias').keyup(function (event) {
          var my_path = $element.find('[name="path[alias]"]').val();
          console.log(path);
          if (my_path == 'node') {
            $('input#edit-path-alias').css('color', '#f00');
            $('input#edit-submit').prop('disabled', true);
            $('input#edit-submit').css('background-color', '#fafafa');
          }
          else {
            $('input#edit-submit').prop('disabled', false);
            $('input#edit-path-alias').css('color', '#000');
            $('input#edit-submit').css('background-color', '#1766ae');
          }
        });

        return Backdrop.t('Alias: @alias', { '@alias': path });
      }
      else {
        return Backdrop.t('No alias');
      }
    });
  }
};

})(jQuery);
