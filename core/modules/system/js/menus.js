(function ($) {

"use strict";

  /**
 * Attaches menu styles to menu blocks. Currently only the "dropdown" style.
 */
Backdrop.behaviors.menuStyles = {
  attach: function(context, settings) {
    var $menus = $(context).find('[data-menu-style]').once('menu-style');
    $menus.each(function() {
      var element = this;
      var $menu = $(element);
      var style = $menu.data('menuStyle');
      var menuSettings = $menu.data('menuSettings');
      if (Backdrop.menuStyles[style]) {
        Backdrop.menuStyles[style].attach(element, menuSettings);
      }
    });
  },
  detach: function(context, settings) {
    var $menus = $(context).find('[data-menu-style]').once('menu-style');
    $menus.each(function() {
      var element = this;
      var $menu = $(element);
      var style = $menu.data('menuStyle');
      var menuSettings = $menu.data('menuSettings');
      if (Backdrop.menuStyles[style] && Backdrop.menuStyles[style].detach) {
        Backdrop.menuStyles[style].detach(element, menuSettings);
      }
    });
  }
};

Backdrop.menuStyles = {};
Backdrop.menuStyles.dropdown = {
  attach: function(element, settings) {
    // Set defaults for the settings.
    settings = $.extend(settings, {
      subIndicatorsPos: 'append',
      subIndicatorsText: ''
    });
    $(element).addClass('sm').smartmenus(settings);

    // Add gentle menu slide behavior.
    $('.menu-toggle-state').once('smartmenus-behavior', function() {
      var $mainMenuState = $(this);
      // animate mobile menu
      $mainMenuState.change(function(e) {
        var $menu = $(element);
        if (this.checked) {
          $menu.hide().slideDown(250, function() { $menu.css('display', ''); });
        }
        else {
          $menu.show().slideUp(250, function() { $menu.css('display', ''); });
        }
      });
      // hide mobile menu beforeunload
      $(window).bind('beforeunload unload', function() {
        if ($mainMenuState[0].checked) {
          $mainMenuState[0].click();
        }
      });
    });
  }
};

})(jQuery);
