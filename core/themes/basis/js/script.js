(function ($) {
"use strict";

Backdrop.behaviors.toggles = {
  attach: function(context, settings) {
    var $toggles = $('[data-toggle]');

    $toggles.click(function(){
      var $this = $(this);
      var $target = $('[data-toggleable="' + $this.attr('data-toggle') + '"]');
      $target.toggleClass('js-toggled');
    });
  }
}

// Detect if flexbox is available to give helper class for CSS
Backdrop.featureDetect.flexbox();

if (typeof Backdrop.tableDrag !== 'undefined') {
  /**
   * Overriding core's tableDrag prototype to update markup of handle
   * Take an item and add event handlers to make it become draggable.
   */
  Backdrop.tableDrag.prototype.makeDraggable = function (item) {
    var self = this;
    var $item = $(item);

    // Create the handle.
    var handle = $('<a href="#" class="tabledrag-handle"><div class="handle"><div class="handle-inner">&nbsp;</div></div></a>').attr('title', Backdrop.t('Drag to re-order'));
    // Insert the handle after indentations (if any).
    var $indentationLast = $item.find('td:first .indentation:last');
    if ($indentationLast.length) {
      $indentationLast.after(handle);
      // Update the total width of indentation in this entire table.
      self.indentCount = Math.max($item.find('.indentation').length, self.indentCount);
    }
    else {
      $item.find('td:first').prepend(handle);
    }

    // Add hover action for the handle.
    handle.hover(function () {
      self.dragObject == null ? $(this).addClass('tabledrag-handle-hover') : null;
    }, function () {
      self.dragObject == null ? $(this).removeClass('tabledrag-handle-hover') : null;
    });

    // Add event handler to start dragging when a handle is clicked or touched
    handle.on('mousedown touchstart pointerdown', function (event) {
      event.preventDefault();
      if (event.type == "touchstart") {
        event = event.originalEvent.touches[0];
      }
      self.dragStart(event, self, item);
    });

    // Prevent the anchor tag from jumping us to the top of the page.
    handle.on('click', function (e) {
      e.preventDefault();
    });

    // Set blur cleanup when a handle is focused.
    handle.on('focus', function () {
      $(this).addClass('tabledrag-handle-hover');
      self.safeBlur = true;
    });

    // On blur, fire the same function as a touchend/mouseup. This is used to
    // update values after a row has been moved through the keyboard support.
    handle.on('blur', function (event) {
      $(this).removeClass('tabledrag-handle-hover');
      if (self.rowObject && self.safeBlur) {
        self.dropRow(event, self);
      }
    });

    // Add arrow-key support to the handle.
    handle.on('keydown', function (event) {
      // If a rowObject doesn't yet exist and this isn't the tab key.
      if (event.keyCode !== 9 && !self.rowObject) {
        self.rowObject = new self.row(item, 'keyboard', self.indentEnabled, self.maxDepth, true);
      }

      var keyChange = false;
      var groupHeight;
      switch (event.keyCode) {
        case 37: // Left arrow.
        case 63234: // Safari left arrow.
          keyChange = true;
          self.rowObject.indent(-1 * self.rtl);
          break;
        case 38: // Up arrow.
        case 63232: // Safari up arrow.
          var $previousRow = $(self.rowObject.element).prev('tr').eq(0);
          var previousRow = $previousRow.get(0);
          while (previousRow && $previousRow.is(':hidden')) {
            $previousRow = $(previousRow).prev('tr').eq(0);
            previousRow = $previousRow.get(0);
          }
          if (previousRow) {
            self.safeBlur = false; // Do not allow the onBlur cleanup.
            self.rowObject.direction = 'up';
            keyChange = true;

            if ($(item).is('.tabledrag-root')) {
              // Swap with the previous top-level row.
              groupHeight = 0;
              while (previousRow && $previousRow.find('.indentation').length) {
                $previousRow = $(previousRow).prev('tr').eq(0);
                previousRow = $previousRow.get(0);
                groupHeight += $previousRow.is(':hidden') ? 0 : previousRow.offsetHeight;
              }
              if (previousRow) {
                self.rowObject.swap('before', previousRow);
                // No need to check for indentation, 0 is the only valid one.
                window.scrollBy(0, -groupHeight);
              }
            }
            else if (self.table.tBodies[0].rows[0] !== previousRow || $previousRow.is('.draggable')) {
              // Swap with the previous row (unless previous row is the first one
              // and undraggable).
              self.rowObject.swap('before', previousRow);
              self.rowObject.interval = null;
              self.rowObject.indent(0);
              window.scrollBy(0, -parseInt(item.offsetHeight, 10));
            }
            handle.trigger('focus'); // Regain focus after the DOM manipulation.
          }
          break;
        case 39: // Right arrow.
        case 63235: // Safari right arrow.
          keyChange = true;
          self.rowObject.indent(self.rtl);
          break;
        case 40: // Down arrow.
        case 63233: // Safari down arrow.
          var $nextRow = $(self.rowObject.group).filter(':last').next('tr').eq(0);
          var nextRow = $nextRow.get(0);
          while (nextRow && $nextRow.is(':hidden')) {
            $nextRow = $(nextRow).next('tr').eq(0);
            nextRow = $nextRow.get(0);
          }
          if (nextRow) {
            self.safeBlur = false; // Do not allow the onBlur cleanup.
            self.rowObject.direction = 'down';
            keyChange = true;

            if ($(item).is('.tabledrag-root')) {
              // Swap with the next group (necessarily a top-level one).
              groupHeight = 0;
              var nextGroup = new self.row(nextRow, 'keyboard', self.indentEnabled, self.maxDepth, false);
              if (nextGroup) {
                $(nextGroup.group).each(function () {
                  groupHeight += $(this).is(':hidden') ? 0 : this.offsetHeight;
                });
                var nextGroupRow = $(nextGroup.group).filter(':last').get(0);
                self.rowObject.swap('after', nextGroupRow);
                // No need to check for indentation, 0 is the only valid one.
                window.scrollBy(0, parseInt(groupHeight, 10));
              }
            }
            else {
              // Swap with the next row.
              self.rowObject.swap('after', nextRow);
              self.rowObject.interval = null;
              self.rowObject.indent(0);
              window.scrollBy(0, parseInt(item.offsetHeight, 10));
            }
            handle.trigger('focus'); // Regain focus after the DOM manipulation.
          }
          break;
      }

      if (self.rowObject && self.rowObject.changed === true) {
        $(item).addClass('drag');
        if (self.oldRowElement) {
          $(self.oldRowElement).removeClass('drag-previous');
        }
        self.oldRowElement = item;
        self.restripeTable();
        self.onDrag();
      }

      // Returning false if we have an arrow key to prevent scrolling.
      if (keyChange) {
        return false;
      }
    });

    // Compatibility addition, return false on keypress to prevent unwanted scrolling.
    // IE and Safari will suppress scrolling on keydown, but all other browsers
    // need to return false on keypress. http://www.quirksmode.org/js/keys.html
    handle.on('keypress', function (event) {
      switch (event.keyCode) {
        case 37: // Left arrow.
        case 38: // Up arrow.
        case 39: // Right arrow.
        case 40: // Down arrow.
          return false;
      }
    });
  };
}

})(jQuery);
