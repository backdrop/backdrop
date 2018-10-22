
(function ($) {

Backdrop.behaviors.nodePreview = {
  attach: function (context) {
    // Insert the preview admin banner into the page before the layout.
    $('.node-preview-container').once('node-preview').each(function() {
      $topParent = $('body').find('div').first();
      $topParent.addClass('preview-js');
      $(this).insertBefore($topParent);
    });
    // Prevent clicking on links during preview.
    $topParent.on('click.preview', 'a', function (e) {
      e.preventDefault();
    });
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
