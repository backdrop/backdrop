/**
 * @file layout.admin.js
 *
 * Behaviors for editing a layout.
 */

(function ($) {

"use strict";

/**
 * Behavior for showing a list of layouts.
 *
 * Detect flexbox support for displaying our list of layouts with vertical
 * height matching for each row of layout template icons.
 */
Backdrop.behaviors.layoutList = {
  attach: function(context) {
    var $element = $(context).find('.layout-options');
    if ($element.length) {
      if (Backdrop.featureDetect.flexbox()) {
        $element.addClass('flexbox');
      }
      else {
        $element.addClass('no-flexbox');
      }
    }
  }
};

/**
 * Behavior for creating/configuring layout settings.
 */
Backdrop.behaviors.layoutConfigure = {
  attach: function(context) {
    var $form = $('.layout-settings-form').once('layout-settings');
    if ($form.length && Backdrop.ajax) {
      var ajax = Backdrop.ajax['edit-path-update'];
      var updateContexts = function() {
        // Cancel existing AJAX requests and start a new one.
        for (var n = 0; n < ajax.currentRequests.lenth; n++) {
          ajax.currentRequests[n].abort();
          ajax.cleanUp(ajax.currentRequests[n]);
        }
        $('input[data-layout-path-update]').triggerHandler('mousedown');
      };
      // Update contexts after a slight typing delay.
      var timer = 0;
      $('input[name="path"]').on('keyup', function(e) {
        clearTimeout(timer);
        timer = setTimeout(updateContexts, 200);
      });
    }

    // Convert AJAX buttons to links.
    var $linkButtons = $(context).find('.layout-link-button').once('link-button');
    if ($linkButtons.length) {
      $linkButtons.each(function() {
        var $self = $(this).addClass('js-hide');
        $('<a class="layout-button-link" href="#"></a>')
          .insertBefore(this)
          // Copy over the title of the button as the link text.
          .text(this.value)
          // Copy over classes.
          .addClass(this.className)
          .removeClass('layout-link-button form-submit ajax-processed link-button-processed js-hide')
          .on('click', function(event) {
            $self.triggerHandler('mousedown');
            event.preventDefault();
          });
      });
    }
  }
};

/**
 * Behavior for editing layouts.
 */
Backdrop.behaviors.layoutDisplayEditor = {
  attach: function(context) {
    var synth = window.speechSynthesis;
    // Apply drag and drop to regions.
    var $regions = $('.layout-editor-region-content').once('layout-sortable');
    if ($regions.length) {
      $regions.sortable({
        connectWith: '.layout-editor-region-content',
        tolerance: 'pointer',
        update: Backdrop.behaviors.layoutDisplayEditor.updateLayout,
        items: '.layout-editor-block',
        placeholder: 'layout-editor-placeholder layout-editor-block',
        forcePlaceholderSize: true
      });

      // Get the list of droppables a.k.a regions where we can drop blocks.
      var droppables = $.map($('.layout-editor-region'), function (item) {return $(item).attr('id')});

      // Find the next region.
      var findNextDroppable = function (current, arr) {
        var index = arr.indexOf(current);
        if(index >= 0 && index < arr.length - 1) {
          return arr[index + 1]
        }
        return false;
      }

      // Find th previous region.
      var findPreviousDroppable = function (current, arr) {
        var index = arr.indexOf(current);
        if(index > 0 && index <= arr.length - 1) {
          return arr[index - 1]
        }
        return false;
      }

      var speak = function (destination) {
        var utterThis = new SpeechSynthesisUtterance('moved to: ' + destination);
        synth.speak(utterThis);
      }

      // Check if an element is in the viewport.
      $.fn.isInViewport = function () {
        let elementTop = $(this).offset().top;
        let elementBottom = elementTop + $(this).outerHeight();

        let viewportTop = $(window).scrollTop();
        let viewportBottom = viewportTop + $(window).height();

        return elementBottom > viewportTop && elementTop < viewportBottom;
      }

      // Scroll to a droppable if it's not visible.
      var scrollIfNotVisible = function (nextDroppableId, droppables) {
        if (findNextDroppable(nextDroppableId, droppables) && $('#' + findNextDroppable(nextDroppableId, droppables)).isInViewport() === false) {
          console.log('scrolling');
          var $nextDroppable = $('#' + findNextDroppable(nextDroppableId, droppables));
          var _offset = $nextDroppable.offset();
          $('html, body').animate({
            scrollTop: _offset.top,
            scrollLeft: _offset.left
          });
        }
      }

      $('.layout-editor-block').bind('keydown', function(event) {
        // Press j to move down.
        if(event.which == 74) {
          var currentDroppableId = $(this).closest('.layout-editor-region').attr('id');
          var nextDroppableId = findNextDroppable(currentDroppableId, droppables);

          if (!nextDroppableId) {
            return;
          }

          scrollIfNotVisible(nextDroppableId, droppables);

          var droppable = $('#' + nextDroppableId + ' .layout-editor-region-content');
          var draggable = $(this);
          var droppableOffset = droppable.offset();
          var draggableOffset = draggable.offset();
          var dx = droppableOffset.left - draggableOffset.left;
          var dy = droppableOffset.top - draggableOffset.top;
          //var nextDestinationWidth = $('#' + nextDroppableId + ' .layout-editor-region-content')

          draggable.width(droppable.width());


          draggable.simulate("drag", {
            dx: dx,
            dy: dy
          });
          draggable.focus();
          var currentRegionLabel = $('#' + findNextDroppable(currentDroppableId, droppables) + ' h2.label').text();
          speak(currentRegionLabel);
        }

        // @Todo : move up
        if(event.which == 75) {
          var currentDroppableId = $(this).closest('.layout-editor-region').attr('id');
          var nextDroppable = findPreviousDroppable(currentDroppableId, droppables);

          if (!nextDroppable) {
            return;
          }

          scrollIfNotVisible(nextDroppableId, droppables);

          var droppable = $('#' + nextDroppable + ' .layout-editor-region-content');
          var draggable = $(this);
          var droppableOffset = droppable.offset();
          var draggableOffset = draggable.offset();
          var dx = droppableOffset.left - draggableOffset.left;
          var dy = droppableOffset.top - draggableOffset.top;
          //var nextDestinationWidth = $('#' + nextDroppable + ' .layout-editor-block')

          draggable.width(droppable.width());

          draggable.simulate("drag", {
            dx: dx,
            dy: dy
          });
          draggable.focus();
          var currentRegionLabel = $('#' + findPreviousDroppable(currentDroppableId, droppables) + ' h2.label').text();
          speak(currentRegionLabel);
        }

        if(event.which == 38) {
          $(this).insertBefore($(this).prev());
          $(this).focus();
        }
        if(event.which == 40) {
          $(this).insertAfter($(this).next());
          $(this).focus();
        }
        if (event.which == 84 || event.which == 33) {
          $(this).parent().prepend($(this));
          $(this).focus();
        }
        if (event.which == 66 || event.which == 34) {
          $(this).parent().append($(this));
          $(this).focus();
        }
        // @Todo: this duplicates Backdrop.behaviors.layoutDisplayEditor.updateLayout().
        var region = $(this).parent();
        var regionName = region.closest('.layout-editor-region').data('regionName');
        var blockList = [];
        region.find('.layout-editor-block').each(function(index) {
          blockList.push($(this).data('blockId'));
        });
        $('input[name="content[positions][' + regionName + ']"]').val(blockList.join(','));
      });

      // Open a dialog if editing a particular block.
      var blockUuid = window.location.hash.replace(/#configure-block:/, '');
      if (blockUuid) {
        window.setTimeout(function() {
          $('[data-block-id="' + blockUuid + '"]').find('li.configure > a').triggerHandler('click');
          // Clear out the hash. Use history if available, preventing another
          // entry (which would require two back button clicks). Fallback to
          // directly updating the URL in the location bar.
          if (window.history && window.history.replaceState) {
            window.history.replaceState({}, '', '#');
          }
          else {
            window.location.hash = '';
          }
        }, 100);
      }
    }

    var $flexible_regions = $('.layout-flexible-editor').once('layout-sortable');
    if ($flexible_regions.length) {
      $flexible_regions.sortable({
        connectWith: '.layout-flexible-content',
        tolerance: 'pointer',
        update: Backdrop.behaviors.layoutDisplayEditor.updateFlexibleLayout,
        items: '.flexible-row',
        placeholder: 'layout-editor-placeholder layout-editor-block',
        forcePlaceholderSize: true
      });
    }

    // Detect the addition of new blocks.
    if ($(context).hasClass('layout-editor-block')) {
      var regionName = $(context).closest('.layout-editor-region').data('regionName');
      var positions = $('input[name="content[positions][' + regionName + ']"]').get(0);
      var blockId = $(context).data('blockId');
      if (positions.value.indexOf(blockId) === -1) {
        positions.value += ',' + $(context).data('blockId');
      }
    }

    // Disable the machine name field on text blocks if reusable is checked.
    if ($('input[name="reusable"]').prop('checked')) {
      $('span.field-suffix').show();
    } else {
      $('span.field-suffix').hide();
    }
    $('input[name="reusable"]').change(function() {
      $('span.field-suffix').toggle();
    });
  },

  /**
   * jQuery UI sortable update callback.
   */
  updateLayout: function(event, ui) {
    var regionName = $(this).closest('.layout-editor-region').data('regionName');
    var blockList = [];
    $(this).find('.layout-editor-block').each(function() {
      blockList.push($(this).data('blockId'));
    });
    $('input[name="content[positions][' + regionName + ']"]').val(blockList.join(','));
  },
  /**
   * jQuery UI sortable update callback.
   */
  updateFlexibleLayout: function(event, ui) {
    var blockList = [];
    $(this).find('.flexible-row').each(function() {
      blockList.push($(this).data('rowId'));
    });
    $('input[name="row_positions"]').val(blockList.join(','));
  }
};

/**
 * Filters the 'Add block' list by a text input search string.
 */
Backdrop.behaviors.blockListFilterByText = {
  attach: function (context, settings) {
    var $input = $('input#layout-block-list-search').once('layout-block-list-search');
    var $form = $('.layout-block-list');
    var $rows, zebraClass;
    var zebraCounter = 0;

    // Filter the list of layouts by provided search string.
    function filterBlockList() {
      var query = $input.val().toLowerCase();

      function showBlockItem(index, row) {
        var $row = $(row);
        var $sources = $row.find('.block-item, .description');
        var textMatch = $sources.text().toLowerCase().indexOf(query) !== -1;
        var $match = $row.closest('div.layout-block-add-row');
        $match.toggle(textMatch);
        if (textMatch) {
          stripeRow($match);
        }
      }

      // Reset the zebra striping for consistent even/odd classes.
      zebraCounter = 0;
      $rows.each(showBlockItem);

      if ($('div.layout-block-add-row:visible').length === 0) {
        if ($('.filter-empty').length === 0) {
          $('.layout-block-list').append('<p class="filter-empty">' + Backdrop.t('No blocks match your search.') + '</p>');
        }
      }
      else {
        $('.filter-empty').remove();
      }
    }

    function stripeRow($match) {
      zebraClass = (zebraCounter % 2) ? 'odd' : 'even';
      $match.removeClass('even odd');
      $match.addClass(zebraClass);
      zebraCounter++;
    }

    if ($form.length && $input.length) {
      $rows = $form.find('div.layout-block-add-row');
      $rows.each(function () {
        stripeRow($(this));
      });

      // @todo Use autofocus attribute when possible.
      $input.focus().on('keyup', filterBlockList);
    }
  }
}

})(jQuery);
