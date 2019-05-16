(function ($) {

Backdrop.behaviors.Term = {
  // Provide the vertical tab summaries.
  attach: function (context) {
    var $context = $(context);

    // Relation settings.
    $context.find('#edit-relations').backdropSetSummary(function(context) {
      var vals = [];
      var info = Backdrop.t('No relations.');

      // Get a list of all selected options and concatenate with a comma.
      var $parentOptions = $context.find('select[name="parent[]"] option:selected');
      var parents = $.map(
        $parentOptions,
        function(element) {
          // Remove leading hyphens on indented terms.
          return $(element).text().replace(/^(\-)+/, '');
        })
        .join(', ');

      if (parents) {
        info = Backdrop.formatPlural($parentOptions.length, 'Parent: @parents', 'Parents: @parents', { '@parents': parents });
      }

      vals.push(info);
      return vals.join(', ');
    });

    // Multilingual settings.
    $context.find('#edit-multilingual').backdropSetSummary(function(context) {
      var vals = [];
      if ($context.find('input[name="language"]:checked').length) {
        vals.push(Backdrop.t('Enabled'));
      }
      else {
        vals.push(Backdrop.t('Not enabled'));
      }
      return Backdrop.checkPlain(vals.join(', '));
    });
  }
};

})(jQuery);
