
(function ($) {

Backdrop.behaviors.nodeFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.node-form-revision-information', context).backdropSetSummary(function (context) {
      var revisionCheckbox = $('.form-item-revision input', context);

      // Return 'New revision' if the 'Create new revision' checkbox is checked,
      // or if the checkbox doesn't exist, but the revision log does. For users
      // without the "Administer content" permission the checkbox won't appear,
      // but the revision log will if the content type is set to auto-revision.
      if (revisionCheckbox.is(':checked') || (!revisionCheckbox.length && $('.form-item-log textarea', context).length)) {
        return Backdrop.t('New revision');
      }

      return Backdrop.t('No revision');
    });

    $('fieldset.node-form-author', context).backdropSetSummary(function (context) {
      var name = $('.form-item-name input', context).val() || Backdrop.settings.anonymous,
        date = $('.form-item-date input', context).val();
      return date ?
        Backdrop.t('By @name on @date', { '@name': name, '@date': date }) :
        Backdrop.t('By @name', { '@name': name });
    });

    $('fieldset.node-form-options', context).backdropSetSummary(function (context) {
      var vals = [];

      // Status radio button.
      var $status = $(context).find('input[name="status"]:checked');
      if ($status.val() == 2) {
        var scheduledDate = $('input[name="scheduled[date]"]').val() + ' ' + $('input[name="scheduled[time]"]').val();
        vals.push(Backdrop.t('Scheduled for @date', { '@date': scheduledDate }));
      }
      else {
        var statusLabel = $status.parent().text();
        vals.push(Backdrop.checkPlain($.trim(statusLabel)));
      }

      // Other checkboxes like Promoted and Sticky.
      $(context).find('input:checked').not($status).parent().each(function () {
        vals.push(Backdrop.checkPlain($.trim($(this).text())));
      });

      return vals.join(', ');
    });
  }
};

})(jQuery);
