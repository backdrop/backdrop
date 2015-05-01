/**
 * @file
 * Attaches behaviors for the Path module.
 */

(function ($) {

Backdrop.behaviors.pathFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.path-form', context).backdropSetSummary(function (context) {
      var path = $('.form-item-path-alias input').val();
      var automatic = $('.form-item-path-auto input').attr('checked');

      if (automatic) {
          return Backdrop.t('Automatic alias');
      }
      if (path) {
          return Backdrop.t('Alias: @alias', { '@alias': path });
      }
      else {
          return Backdrop.t('No alias');
      }
    });
  }
};

})(jQuery);
