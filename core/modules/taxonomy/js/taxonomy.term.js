(function ($) {

Backdrop.behaviors.Term = {
  // Provide the vertical tab summaries.
  attach: function (context) {
    var $context = $(context);

    // Relation settings.
    $context.find('#edit-relations').backdropSetSummary(function(context) {
      var vals = [];

      var info = Backdrop.t('No relations.');
      if (parents = $(context).find('select[name="parent[]"] option:selected').text()) {
        var info = 'Parent: ' + Backdrop.checkPlain(parents);
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
