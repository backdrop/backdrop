(function ($) {

Backdrop.behaviors.textarea = {
  attach: function (context, settings) {
    // This script does nothing, but we give people a hint, why.
    console.warn('Deprecated: The backdrop.textarea library is obsolete, resizing happens via CSS instead.');
  }
};

})(jQuery);
