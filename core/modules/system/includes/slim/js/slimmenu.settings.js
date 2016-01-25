/**
 * @file
 * Simple responsification of menus.
 */
(function ($) {
  // Iterate through selectors, check window sizes, add some classes.
  Backdrop.behaviors.slimmenu = {
    attach: function (context, settings) {
      setting = settings.menu_style || {};
      menu = setting[0].menuName;
      block = $('.block-system-' + menu + ' .block-content').find('ul').first().addClass('slimmenu');
      $('.slimmenu').slimmenu(setting[0]);      
    }
  };

}(jQuery));
