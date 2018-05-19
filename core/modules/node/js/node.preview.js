
(function ($) {

Backdrop.behaviors.nodePreview = {
  attach: function (context) {
    $('.node-preview-container').each(function() {
      $(this).insertBefore($('.page').find('.layout'));
    });
  }
};

})(jQuery);
