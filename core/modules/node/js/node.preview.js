
(function ($) {

Backdrop.behaviors.nodePreview = {
  attach: function (context) {
    // Insert the preview admin banner into the page before the layout.
    $('.node-preview-container').each(function() {
      $(this).insertBefore($('.page').once('preview-js').find('.layout'));
    });
    // Prevent clicking on links during preview.
    var $preview = $(context).find('.layout').once('layout-preview');
    if ($preview.length) {
      $preview.on('click.preview', 'a', function (e) {
        e.preventDefault();
      });
    }
  },
  detach: function (context, settings, trigger) {
    if (trigger === 'unload') {
      var $preview = $(context).find('.layout').removeOnce('layout-preview');
      if ($preview.length) {
        $preview.off('click.preview');
      }
    }
  }
};

})(jQuery);
