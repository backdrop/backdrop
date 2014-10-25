(function ($) {

Backdrop.behaviors.contentTypes = {
  // Provide the vertical tab summaries.
  attach: function (context) {
    var $context = $(context);
    // Submission form settings.
    $context.find('fieldset#edit-submission').backdropSetSummary(function() {
      var vals = [];
      vals.push(Backdrop.checkPlain($context.find('input[name="title_label"]').val()) || Backdrop.t('Requires a title'));
      vals.push(Backdrop.t('Preview !status', { '!status': $context.find('input[name="node_preview"]:checked').siblings('label').text() }));
      return vals.join(', ');
    });

    // Publishing settings.
    $context.find('#edit-workflow').backdropSetSummary(function() {
      var vals = [];
      if (parseInt($context.find('input[name="status_default"]:checked').val())) {
        vals.push(Backdrop.t('Published'));
      }
      else {
        vals.push(Backdrop.t('Unpublished'));
      }
      if ($context.find('input[name="sticky_default"]:checked').length) {
        vals.push(Backdrop.t('Sticky'));
      }
      if ($context.find('input[name="promote_default"]:checked').length) {
        vals.push(Backdrop.t('Promoted'));
      }
      return vals.join(', ');
    });


    // Revision settings.
    $context.find('#edit-revision').backdropSetSummary(function() {
      var vals = [];
      if ($context.find('input[name="revision_enabled"]:checked').length) {
        vals.push(Backdrop.t('Revisions enabled'));
      }
      else {
        vals.push(Backdrop.t('Revisions disabled'));
      }
      return vals.join(', ');
    });

    // Display settings.
    $context.find('#edit-display').backdropSetSummary(function(context) {
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
