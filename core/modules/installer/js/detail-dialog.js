/**
 * @file
 *
 * Open all external links in a dialog.
 * Inspired by https://css-tricks.com/snippets/jquery/open-external-links-in-new-window/
 */
(function ($) {
  $(document).ready(function() {
    // Open all links in dialogs in a new window.
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

    // On mobile add an indicator of number of projects installed and scroll to
    // installation queue on click.
    var windowsize = $(window).width();
    var header = $("#installer-browser-main th").html();
    var items = $('.installer-browser-install-queue-item').length;
    checkQueue();

    function checkQueue() {
      items = $('.installer-browser-install-queue-item').length;
      if (windowsize < 768) {
        updateTH();
      }
    }

    function updateTH() {
      $("#installer-browser-main th").html(header+': <span class="projects-selected">'+items+' selected. <a id="status-count-link" href="ff">review and install</a></span>');
    }

    $(document).ajaxComplete(function() {
      checkQueue();
    });

    $(window).resize(function() {
      windowsize = $(window).width();
      if (windowsize < 768) {
        updateTH();
      }
      else {
        $("#installer-browser-main th").html(header);
      }
    });
    
    $('body').on('click', '#status-count-link', function(event) {
      event.preventDefault();
      event.stopPropagation();
      $('html, body').animate({
          scrollTop: $("#installer-browser-sidebar-right").offset().top
      }, 400);
    });
  });
})(jQuery);
