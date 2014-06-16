/**
 * @file
 * Attaches comment behaviors to the node form.
 */

(function ($) {

Backdrop.behaviors.commentFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.comment-node-settings-form', context).backdropSetSummary(function (context) {
      return Backdrop.checkPlain($('.form-item-comment input:checked', context).next('label').text());
    });

    // Provide the summary for the node type form.
    $('fieldset.comment-node-type-settings-form', context).backdropSetSummary(function(context) {
      var vals = [];

      // Default comment setting.
      vals.push($(".form-item-comment select option:selected", context).text());

      // Threading.
      var threading = $(".form-item-comment-default-mode input:checked", context).next('label').text();
      if (threading) {
        vals.push(threading);
      }

      // Comments per page.
      var number = $(".form-item-comment-default-per-page select option:selected", context).val();
      vals.push(Backdrop.t('@number comments per page', {'@number': number}));

      return Backdrop.checkPlain(vals.join(', '));
    });
  }
};

})(jQuery);
