(function ($) {

Backdrop.behaviors.contentTypes = {
  attach: function (context) {
    // Provide the vertical tab summaries.
    $('fieldset#edit-submission', context).backdropSetSummary(function(context) {
      var vals = [];
      vals.push(Backdrop.checkPlain($('#edit-title-label', context).val()) || Backdrop.t('Requires a title'));
      return vals.join(', ');
    });
    $('fieldset#edit-workflow', context).backdropSetSummary(function(context) {
      var vals = [];
      $("input[name^='node_options']:checked", context).parent().each(function() {
        vals.push(Backdrop.checkPlain($(this).text()));
      });
      if (!$('#edit-node-options-status', context).is(':checked')) {
        vals.unshift(Backdrop.t('Not published'));
      }
      return vals.join(', ');
    });
    $('fieldset#edit-display', context).backdropSetSummary(function(context) {
      var vals = [];
      $('input:checked', context).next('label').each(function() {
        vals.push(Backdrop.checkPlain($(this).text()));
      });
      if (!$('#edit-node-submitted', context).is(':checked')) {
        vals.unshift(Backdrop.t("Don't display post information"));
      }
      return vals.join(', ');
    });
  }
};

})(jQuery);
