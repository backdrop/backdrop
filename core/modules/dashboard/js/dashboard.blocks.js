(function($, Backdrop) {
  'use strict';

  Backdrop.behaviors.dashboardCollapsibleBlocks = {
    attach: function(context, settings) {
      if (!window.location.pathname.match(/^\/admin\/dashboard/)) return;

      $('.block-dashboard, .admin-panel', context).once('dashboard-collapsible').each(function(i) {
        var $block = $(this);
        // Only target the main block title, not sub-headings
        var $title = $block.children('.block-title:first, h3:first');
        // For the content, target everything after the title
        var $content = $block.children(':not(.block-title):not(h3:first)');
        
        if (!$title.length || !$content.length) return;
        
        // Use a unique key for each block
        var blockId = 'dashboard-block-' + ($title.text() || i).replace(/\s+/g, '-').toLowerCase();
        
        // Add caret if not present
        if (!$title.find('.toggle-caret').length) {
          var $caret = $('<span class="toggle-caret">&#9654;</span>');
          $title.append($caret);
        }

        // Make title clickable
        $title.attr({
          'tabindex': '0',
          'role': 'button',
          'aria-controls': blockId + '-content'
        });
        $content.attr('id', blockId + '-content');

        // Restore state
        var collapsed = localStorage.getItem(blockId) === 'collapsed';
        if (collapsed) {
          $content.addClass('hide');
          $title.attr('aria-expanded', 'false');
          $title.find('.toggle-caret').css('transform', '');
        } else {
          $content.removeClass('hide');
          $title.attr('aria-expanded', 'true');
          $title.find('.toggle-caret').css('transform', 'rotate(90deg)');
        }

        // Toggle handler
        function toggleBlock() {
          var isCollapsed = !$content.hasClass('hide');
          $content.toggleClass('hide');
          
          if (isCollapsed) {
            $title.attr('aria-expanded', 'false');
            $title.find('.toggle-caret').css('transform', '');
            localStorage.setItem(blockId, 'collapsed');
          } else {
            $title.attr('aria-expanded', 'true');
            $title.find('.toggle-caret').css('transform', 'rotate(90deg)');
            localStorage.setItem(blockId, 'expanded');
          }
        }

        $title.on('click', toggleBlock);
        $title.on('keydown', function(e) {
          if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            toggleBlock();
          }
        });
      });
    }
  };

})(jQuery, Backdrop); 