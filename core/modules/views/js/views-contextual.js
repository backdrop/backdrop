/**
 * @file
 * JavaScript related to contextual links.
 */
(function ($) {

  Backdrop.behaviors.viewsContextualLinks = {
    attach: function (context) {
      // If there are views-related contextual links attached to the main page
      // content, find the smallest region that encloses both the links and the
      // view, and display it as a contextual links region.
      $('.views-contextual-links-page', context).closest(':has(".view"):not("body")').addClass('contextual-links-region');
      $('.views-contextual-links-page', context).closest(':has(".view"):not("body")').addClass('pink');
    }
  };

})(jQuery);
