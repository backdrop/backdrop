(function ($) {

"use strict";

Backdrop.behaviors.menuToggle = {
  attach: function(context, settings) {
    var $mainMenuState = $('#main-menu-state');
    if ($mainMenuState.length) {
      // animate mobile menu
      $mainMenuState.change(function(e) {
        var $menu = $('.block-system-main-menu .menu');
        if (this.checked) {
          $menu.hide().slideDown(250, function() { $menu.css('display', ''); });
        } else {
          $menu.show().slideUp(250, function() { $menu.css('display', ''); });
        }
      });
      // hide mobile menu beforeunload
      $(window).bind('beforeunload unload', function() {
        if ($mainMenuState[0].checked) {
          $mainMenuState[0].click();
        }
      });
    }
  }
};

})(jQuery);
