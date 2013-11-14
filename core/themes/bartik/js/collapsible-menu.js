/**
 * Behaviors for collapsible menu.
 */
(function($) {

  /**
   * Adds toggle link.
   * Toggles menu on small resolutions.
   * Restores menu on window width increasing.
   */
  Drupal.behaviors.responsiveBartikCollapsibleMenu = {
    attach: function (context, settings) {

      // We can keep menu collapsed up to width maxWidth.
      var maxWidth = 445;

      // Do nothing if menu is empty.
      if ($('#main-menu-links a').length == 0) {
        return;
      }

      // Append toggle link to the main menu.
      $('nav#main-menu').append('<a href="#" id="menu-toggle">' + Drupal.t('Menu') + '</a>');

      // Collapse/expand menu by click on link.
      $('a#menu-toggle').click(function() {
        $('#main-menu-links').slideToggle('fast');
        return false;
      });

      // Restore visibility settings of menu on increasing of windows width over 445px.
      // Media query works with width up to 460px. But I guess we should take into account some padding.
      $(window).resize(function(){
        var w = $(window).width();
        // Remove all styles if window size more than maxWidth and menu is hidden.
        if(w > maxWidth && $('#main-menu-links').is(':hidden')) {
          $('#main-menu-links').removeAttr('style');
        }
      });
    }
  }
})(jQuery);
