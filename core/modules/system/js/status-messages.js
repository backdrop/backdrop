(function ($) {
  'use strict';

  Backdrop.behaviors.statusMessages = {
    attach: function (context, settings) {
      const $messages = $(context).find('[data-message-type]');
      const $messageContainer = $messages.parent().addClass('msgparent');

      if (!$messages.length) {
        return;
      }

      var observer = new IntersectionObserver(onIntersection, {
        root: null,
        threshold: .5
      })

      observer.observe(document.querySelector('.msgparent'))

      function onIntersection(entries, opts){
        if(entries[0].isIntersecting === false) {
            if (!$('#messages-popover').length){
              $('body').append('<div id="messages-popover">'+$messageContainer.html()+'</div>');
              const messagesPopover = document.querySelector('#messages-popover');
              messagesPopover.setAttribute("popover", 'manual');
              messagesPopover.showPopover();
            }
        }
        else {
          if ($('#messages-popover').length){
            $('#messages-popover').remove();
          }
        }
      }
    }
  };
})(jQuery);
