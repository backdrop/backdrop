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
    });

    /**
     * Adjusts trigger positions in contextual links to avoid overlaps.
     */
    function adjustContextualLinks() {
      // Get all wrappers anywhere on the page and some info about each.
      var allWrappers = [];
      $('.contextual-links-wrapper', context).each(function() {
        allWrappers.push({
          'wrapper': $(this),
          'regionOffsetBottom': $(this).parent().offset().top + $(this).parent().height(),
          'hshift': 0,
          'vshift': 0
        });
      });

      // Reset margins on all wrappers.
      allWrappers.forEach(function(info) {
        info.wrapper.css('margin', '0');
      });

      // Recalculate margins to avoid collisions.
      var dir = $('html').attr('dir');
      const hsize = 28; // width of trigger wrapper
      const vsize = 19; // height of trigger wrapper
      var n = allWrappers.length;
      for (let i = 0; i < n; i++) {
        var follower = allWrappers[i];
        // Compare follower against all of its predecessors in the list (any of
        // which may have already been adjusted).
        for (let j = 0; j < i; j++) {
          var leader = allWrappers[j];
          // Adjust the position of follower if necessary to avoid collision
          // with leader.
          var leaderOffset = leader.wrapper.offset();
          var followerOffset = follower.wrapper.offset();
          var verticalOverlap = followerOffset.top >= leaderOffset.top && followerOffset.top < leaderOffset.top + vsize;
          if (dir == 'ltr') {
            var horizontalOverlap = followerOffset.left >= leaderOffset.left - hsize && followerOffset.left < leaderOffset.left + hsize;
            if (verticalOverlap && horizontalOverlap) {
              // We have a collision; shift the follower down if there's room,
              // otherwise left.
              if (followerOffset.top + 2 * vsize <= follower.regionOffsetBottom) {
                // Shift down
                follower.vshift += vsize;
                follower.wrapper.css('margin-top', follower.vshift);
              }
              else {
                // Shift left and start a new column.
                follower.vshift = 0;
                follower.hshift += hsize;
                follower.wrapper.css('margin-top', follower.vshift);
                follower.wrapper.css('margin-right', follower.hshift);
              }
            }
          }
          else { // rtl
            var horizontalOverlap = followerOffset.left > leaderOffset.left - hsize && followerOffset.left <= leaderOffset.left + hsize;
            if (verticalOverlap && horizontalOverlap) {
              // We have a collision; shift the follower down if there's room,
              // otherwise right.
              if (followerOffset.top + 2 * vsize <= follower.regionOffsetBottom) {
                // Shift down
                follower.vshift += vsize;
                follower.wrapper.css('margin-top', follower.vshift);
              }
              else {
                // Shift right and start a new column.
                follower.vshift = 0;
                follower.hshift += hsize;
                follower.wrapper.css('margin-top', follower.vshift);
                follower.wrapper.css('margin-left', follower.hshift);
              }
            }
          }
        }
      }
    }
    $(document).ready(adjustContextualLinks);
    Backdrop.optimizedResize.add(adjustContextualLinks, 'adjustContextualLinks');
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
