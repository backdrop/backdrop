(function ($) {

Backdrop.behaviors.fileFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.file-form-destination', context).backdropSetSummary(function (context) {
      var scheme = $('.form-item-scheme input:checked', context).parent().text();
      return Backdrop.t('Destination: @scheme', { '@scheme': scheme });
    });
    $('fieldset.file-form-user', context).backdropSetSummary(function (context) {
      var name = $('.form-item-name input', context).val() || Backdrop.settings.anonymous;
      return Backdrop.t('Associated with @name', { '@name': name });
    });
  }
};

})(jQuery);
