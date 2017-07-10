(function ($) {

Drupal.behaviors.fileFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.file-form-destination', context).drupalSetSummary(function (context) {
      var scheme = $('.form-item-scheme input:checked', context).parent().text();
      return Drupal.t('Destination: @scheme', { '@scheme': scheme });
    });
    $('fieldset.file-form-user', context).drupalSetSummary(function (context) {
      var name = $('.form-item-name input', context).val() || Drupal.settings.anonymous;
      return Drupal.t('Associated with @name', { '@name': name });
    });
  }
};

})(jQuery);
