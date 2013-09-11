
(function ($) {

Backdrop.behaviors.pathFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.path-form', context).backdropSetSummary(function (context) {
      var path = $('.form-item-path-alias input').val();

      return path ?
        Backdrop.t('Alias: @alias', { '@alias': path }) :
        Backdrop.t('No alias');
    });
  }
};

})(jQuery);
