
/**
 * @file
 * Attaches behaviors for the Path module.
 */

(function ($) {

/*
Geoff commented this out when adding pathauto to path in favour of the below behaviour
from pathauto.

If this was the correct thing to do we should completely remove this.
*/
/*Backdrop.behaviors.pathFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.path-form', context).backdropSetSummary(function (context) {
      var path = $('.form-item-path-alias input').val();

      return path ?
        Backdrop.t('Alias: @alias', { '@alias': path }) :
        Backdrop.t('No alias');
    });
  }
};*/

Backdrop.behaviors.pathFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.path-form', context).backdropSetSummary(function (context) {
      var path = $('.form-item-path-alias input').val();
      var automatic = $('.form-item-path-pathauto input').attr('checked');

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
