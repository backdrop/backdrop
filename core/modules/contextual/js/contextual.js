/**
 * @file
 * Attaches behaviors for the Contextual module.
 */

(function ($) {

Backdrop.contextualLinks = Backdrop.contextualLinks || {};

/**
 * Attaches outline behavior for regions associated with contextual links.
 */
Backdrop.behaviors.contextualLinks = {
  attach: function (context) {
    $('.contextual-links-wrapper', context).once('contextual-links', function () {
      var $wrapper = $(this);
      var $region = $wrapper.closest('.contextual-links-region');
      var $links = $wrapper.find('ul.contextual-links');
      var $trigger = $('<a class="contextual-links-trigger" href="#" />').text(Backdrop.t('Configure')).click(
        function () {
          $links.stop(true, true).slideToggle(100);
          $wrapper.toggleClass('contextual-links-active');
          return false;
        }
      );
      // Attach hover behavior to trigger and ul.contextual-links.
      $trigger.add($links).hover(
        function () { $region.addClass('contextual-links-region-active'); },
        function () { $region.removeClass('contextual-links-region-active'); }
      );
      // Hide the contextual links when user clicks a link or rolls out of the .contextual-links-region.
      $region.bind('mouseleave click', Backdrop.contextualLinks.mouseleave);
      $region.hover(
        function() { $trigger.addClass('contextual-links-trigger-active'); },
        function() { $trigger.removeClass('contextual-links-trigger-active'); }
      );
      // Prepend the trigger.
      $wrapper.prepend($trigger);

      // Prevent child contextual link triggers from overlapping their parent
      // ones by shifting their position.
      var $child = $wrapper.siblings().find('.contextual-links-wrapper').eq(0);
      if ($child.length > 0) {
        var height = $child.parent().height();
        var parentOffset = $wrapper.offset();
        var childOffset = $child.offset();
        if (childOffset.top < parentOffset.top + 20 && childOffset.left > parentOffset.left  - 25) {
          // There's a collision, so we need to shift the child.
          if (height >= 40) {
              // If there's enought vertical room in the child contextual links
              // region, shift the child links down.
            $child.css('margin-top', '20px');
          }
          else {
            // Otherwise shift the child links to the left.
            $child.css('margin-right', '25px');
          }
        }
      }
    });
  }
};

/**
 * Disables outline for the region contextual links are associated with.
 */
Backdrop.contextualLinks.mouseleave = function () {
  $(this)
    .find('.contextual-links-active').removeClass('contextual-links-active')
    .find('ul.contextual-links').hide();
};

})(jQuery);
