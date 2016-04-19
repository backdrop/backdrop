/**
 * @file
 * Simple responsification of menus.
 */
(function ($) {
  Backdrop.behaviors.slimmenu = {
    attach: function (context, settings) {
      settings.menu_style = settings.menu_style || {};
      $.each(settings.menu_style, function(ind, iteration) {
        if (iteration.menu_style_name != 'slimmenu') {
          return true;
        }
        if (!iteration.selector.length) {
          return;
        }
        // Set 1/0 to true/false respectively.
        $.each(iteration, function(key, value) {
          if (value == 0) {
            iteration[key] = false;
          }
          if (value == 1) {
            iteration[key] = true;
          }
        });
        block = $('.' + iteration.selector + ' .block-content').find('ul').first().addClass('slimmenu');
        $('.slimmenu').once('responsive-menus-mean-menu', function() {
          $(this).slimmenu(iteration);      
        });
      });

    }
  };
}(jQuery));