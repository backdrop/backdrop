/**
 * @file
 *
 * Open all external links in a dialog.
 * Inspired by https://css-tricks.com/snippets/jquery/open-external-links-in-new-window/
 */
(function ($) {
  $(document).ready(function() {
    $(window).on( "dialog:aftercreate", function( event, ui ) {
      $('a').each(function() {
         var a = new RegExp('/' + window.location.host + '/');
         if(!a.test(this.href)) {
             $(this).click(function(event) {
                 event.preventDefault();
                 event.stopPropagation();
                 window.open(this.href, '_blank');
             });
         }
      });
    });
  });
})(jQuery);
