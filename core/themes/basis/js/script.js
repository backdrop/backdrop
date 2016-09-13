(function ($) {

"use strict";

// Add the flexbox class on all pages as soon as possible.
Backdrop.featureDetect.flexbox();

Backdrop.behaviors.toggles = {
  attach: function(context, settings) {
    var $toggles = $(context).find('[data-toggle]').once('toggle');

    $toggles.click(function(){
      var $this = $(this);
      var $target = $('[data-toggleable="' + $this.attr('data-toggle') + '"]');
      $target.toggleClass('js-toggled');
    });
  }
};

/**
 * Override tableDragHandle().
 */
Backdrop.theme.prototype.tableDragHandle = function() {
  return '<a href="#" title="' + Backdrop.t('Drag to re-order') + '" class="tabledrag-handle"><div class="handle"><div class="handle-inner">&nbsp;</div></div></a>';
};

})(jQuery);
