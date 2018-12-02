(function ($) {

/**
 * Drag and drop table rows with field manipulation.
 *
 * Using the backdrop_add_tabledrag() function, any table with weights or parent
 * relationships may be made into draggable tables. Columns containing a field
 * may optionally be hidden, providing a better user experience.
 *
 * Created tableDrag instances may be modified with custom behaviors by
 * overriding the .onDrag, .onDrop, .row.onSwap, and .row.onIndent methods.
 * See blocks.js for an example of adding additional functionality to tableDrag.
 */
Backdrop.behaviors.tableDrag = {
  attach: function (context, settings) {
    function initTableDrag(table, base) {
      if (table.length) {
        // Create the new tableDrag instance. Save in the Backdrop variable
        // to allow other scripts access to the object.
        Backdrop.tableDrag[base] = new Backdrop.tableDrag(table[0], settings.tableDrag[base]);
       }
     }

     for (var base in settings.tableDrag) {
       if (settings.tableDrag.hasOwnProperty(base)) {
         initTableDrag($(context).find('#' + base).once('tabledrag'), base);
       }
    }
  }
};

/**
 * Constructor for the tableDrag object. Provides table and field manipulation.
 *
 * @param table
 *   DOM object for the table to be made draggable.
 * @param tableSettings
 *   Settings for the table added via backdrop_add_tabledrag().
 */
Backdrop.tableDrag = function (table, tableSettings) {
  var self = this;

  // Required object variables.
  this.table = table;
  this.tableSettings = tableSettings;
  this.dragObject = null; // Used to hold information about a current drag operation.
  this.rowObject = null; // Provides operations for row manipulation.
  this.oldRowElement = null; // Remember the previous element.
  this.oldY = 0; // Used to determine up or down direction from last pointer move.
  this.changed = false; // Whether anything in the entire table has changed.
  this.maxDepth = 0; // Maximum amount of allowed parenting.
  this.rtl = $(this.table).css('direction') == 'rtl' ? -1 : 1; // Direction of the table.

  // Touch support, borrowed from Modernizer.touch.
  this.touchSupport = ('ontouchstart' in window) || window.DocumentTouch && document instanceof DocumentTouch;

  // Configure the scroll settings.
  this.scrollSettings = { amount: 4, interval: 50, trigger: 70 };
  this.scrollInterval = null;
  this.scrollY = 0;
  this.windowHeight = 0;

  // Check this table's settings to see if there are parent relationships in
  // this table. For efficiency, large sections of code can be skipped if we
  // don't need to track horizontal movement and indentations.
  this.indentEnabled = false;
  for (var group in tableSettings) {
    if (tableSettings.hasOwnProperty(group)) {
       for (var n in tableSettings[group]) {
         if (tableSettings[group].hasOwnProperty(n)) {
           if (tableSettings[group][n].relationship === 'parent') {
             this.indentEnabled = true;
           }
           if (tableSettings[group][n].limit > 0) {
             this.maxDepth = tableSettings[group][n].limit;
           }
         }
      }
    }
  }
  if (this.indentEnabled) {
    this.indentCount = 1; // Total width of indents, set in makeDraggable.
    // Find the width of indentations to measure pointer movements against.
    // Because the table doesn't need to start with any indentations, we
    // manually append 2 indentations in the first draggable row, measure
    // the offset, then remove.
    var indent = Backdrop.theme('tableDragIndentation');
    var testRow = $('<tr/>').addClass('draggable').appendTo(table);
    var testCell = $('<td/>').appendTo(testRow).prepend(indent).prepend(indent);
    this.indentAmount = $('.indentation', testCell).get(1).offsetLeft - $('.indentation', testCell).get(0).offsetLeft;
    testRow.remove();
  }

  // Make each applicable row draggable.
  // Match immediate children of the parent element to allow nesting.
  $('> tr.draggable, > tbody > tr.draggable', table).each(function () { self.makeDraggable(this); });

  // Add a link before the table for users to show or hide weight columns.
  $(table).before($('<a href="#" class="tabledrag-toggle-weight"></a>')
    .attr('title', Backdrop.t('Re-order rows by numerical weight instead of dragging.'))
    .click(function () {
      if ($.cookie('Backdrop.tableDrag.showWeight') == 1) {
        self.hideColumns();
      }
      else {
        self.showColumns();
      }
      return false;
    })
    .wrap('<div class="tabledrag-toggle-weight-wrapper"></div>')
    .parent()
  );

  // Initialize the specified columns (for example, weight or parent columns)
  // to show or hide according to user preference. This aids accessibility
  // so that, e.g., screen reader users can choose to enter weight values and
  // manipulate form elements directly, rather than using drag-and-drop..
  self.initColumns();

  // Add event bindings to the document. The self variable is passed along
  // as event handlers do not have direct access to the tableDrag object.
  $(document).on('touchmove', function (event) { return self.dragRow(event.originalEvent.touches[0], self); });
  $(document).on('touchend', function (event) { return self.dropRow(event.originalEvent.touches[0], self); });
  $(document).on('mousemove pointermove', function (event) { return self.dragRow(event, self); });
  $(document).on('mouseup pointerup', function (event) { return self.dropRow(event, self); });
};

/**
 * Initialize columns containing form elements to be hidden by default,
 * according to the settings for this tableDrag instance.
 *
 * Identify and mark each cell with a CSS class so we can toggle show/hide it.
 * Finally, hide columns if user does not have a 'Backdrop.tableDrag.showWeight'
 * cookie.
 */
Backdrop.tableDrag.prototype.initColumns = function () {
  var $table = $(this.table), hidden, cell, columnIndex;
  for (var group in this.tableSettings) {
    if (this.tableSettings.hasOwnProperty(group)) { // Find the first field in this group.
      for (var d in this.tableSettings[group]) {
        if (this.tableSettings[group].hasOwnProperty(d)) {
          var field = $table.find('.' + this.tableSettings[group][d].target + ':first');
          if (field.length && this.tableSettings[group][d].hidden) {
            hidden = this.tableSettings[group][d].hidden;
            cell = field.closest('td');
            break;
          }
        }
      }

     // Mark the column containing this field so it can be hidden.
     if (hidden && cell[0]) {
       // Add 1 to our indexes. The nth-child selector is 1 based, not 0 based.
       // Match immediate children of the parent element to allow nesting.
       columnIndex = cell.parent().find('> td').index(cell.get(0)) + 1;
       $table.find('> thead > tr, > tbody > tr, > tr').each(this.addColspanClass(columnIndex));
      }
    }
  }

  // Now hide cells and reduce colspans unless cookie indicates previous choice.
  // Set a cookie if it is not already present.
  if ($.cookie('Backdrop.tableDrag.showWeight') === null) {
    $.cookie('Backdrop.tableDrag.showWeight', 0, {
      path: Backdrop.settings.basePath,
      // The cookie expires in one year.
      expires: 365
    });
    this.hideColumns();
  }
  // Check cookie value and show/hide weight columns accordingly.
  else {
    if ($.cookie('Backdrop.tableDrag.showWeight') == 1) {
      this.showColumns();
    }
    else {
      this.hideColumns();
    }
  }
};

/**
 * Mark cells that have colspan so we can adjust the colspan
 * instead of hiding them altogether.
 */
Backdrop.tableDrag.prototype.addColspanClass = function(columnIndex) {
  return function () {
    // Get the columnIndex and adjust for any colspans in this row.
    var $row = $(this);
    var index = columnIndex;
    var cells = $row.children();
    var cell;
    cells.each(function (n) {
      if (n < index && this.colSpan && this.colSpan > 1) {
        index -= this.colSpan - 1;
      }
    });
    if (index > 0) {
      cell = cells.filter(':nth-child(' + index + ')');
      if (cell[0].colSpan && cell[0].colSpan > 1) {
        // If this cell has a colspan, mark it so we can reduce the colspan.
        cell.addClass('tabledrag-has-colspan');
      }
      else {
        // Mark this cell so we can hide it.
        cell.addClass('tabledrag-hide');
      }
    }
  };
};

/**
 * Hide the columns containing weight/parent form elements.
 * Undo showColumns().
 */
Backdrop.tableDrag.prototype.hideColumns = function () {
  // Hide weight/parent cells and headers.
  $('.tabledrag-hide', 'table.tabledrag-processed').css('display', 'none');
  // Show TableDrag handles.
  $('.tabledrag-handle', 'table.tabledrag-processed').css('display', '');
  // Reduce the colspan of any effected multi-span columns.
  $('.tabledrag-has-colspan', 'table.tabledrag-processed').each(function () {
    this.colSpan = this.colSpan - 1;
  });
  // Change link text.
  $('.tabledrag-toggle-weight').text(Backdrop.t('Show row weights'));
  // Change cookie.
  $.cookie('Backdrop.tableDrag.showWeight', 0, {
    path: Backdrop.settings.basePath,
    // The cookie expires in one year.
    expires: 365
  });
  // Trigger an event to allow other scripts to react to this display change.
  $('table.tabledrag-processed').trigger('columnschange', 'hide');
};

/**
 * Show the columns containing weight/parent form elements
 * Undo hideColumns().
 */
Backdrop.tableDrag.prototype.showColumns = function () {
  // Show weight/parent cells and headers.
  $('.tabledrag-hide', 'table.tabledrag-processed').css('display', '');
  // Hide TableDrag handles.
  $('.tabledrag-handle', 'table.tabledrag-processed').css('display', 'none');
  // Increase the colspan for any columns where it was previously reduced.
  $('.tabledrag-has-colspan', 'table.tabledrag-processed').each(function () {
    this.colSpan = this.colSpan + 1;
  });
  // Change link text.
  $('.tabledrag-toggle-weight').text(Backdrop.t('Hide row weights'));
  // Change cookie.
  $.cookie('Backdrop.tableDrag.showWeight', 1, {
    path: Backdrop.settings.basePath,
    // The cookie expires in one year.
    expires: 365
  });
  // Trigger an event to allow other scripts to react to this display change.
  $('table.tabledrag-processed').trigger('columnschange', 'show');
};

/**
 * Find the target used within a particular row and group.
 */
Backdrop.tableDrag.prototype.rowSettings = function (group, row) {
  var field = $('.' + group, row);
  var tableSettingsGroup = this.tableSettings[group];
   for (var delta in tableSettingsGroup) {
     if (tableSettingsGroup.hasOwnProperty(delta)) {
       var targetClass = tableSettingsGroup[delta].target;
       if (field.is('.' + targetClass)) {
         // Return a copy of the row settings.
         var rowSettings = {};
         for (var n in tableSettingsGroup[delta]) {
           if (tableSettingsGroup[delta].hasOwnProperty(n)) {
             rowSettings[n] = tableSettingsGroup[delta][n];
           }
         }
         return rowSettings;
      }
    }
  }
};

/**
 * Take an item and add event handlers to make it become draggable.
 */
Backdrop.tableDrag.prototype.makeDraggable = function (item) {
  var self = this;
  var $item = $(item);

  // Create the handle.
  var handle = $(Backdrop.theme('tableDragHandle'));
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

/**
 * Pointer event initiator, creates drag object and information.
 *
 * @param jQuery.Event event
 *   The event object that trigger the drag.
 * @param Backdrop.tableDrag self
 *   The drag handle.
 * @param DOM item
 *   The item that that is being dragged.
 */
Backdrop.tableDrag.prototype.dragStart = function (event, self, item) {
  // Create a new dragObject recording the pointer information.
  self.dragObject = {};
  self.dragObject.initOffset = self.getPointerOffset(item, event);
  self.dragObject.initPointerCoords = self.pointerCoords(event);
  if (self.indentEnabled) {
    self.dragObject.indentPointerPos = self.dragObject.initPointerCoords;
  }

  // If there's a lingering row object from the keyboard, remove its focus.
  if (self.rowObject) {
    $(self.rowObject.element).find('a.tabledrag-handle').trigger('blur');
  }

  // Create a new rowObject for manipulation of this row.
  self.rowObject = new self.row(item, 'pointer', self.indentEnabled, self.maxDepth, true);

  // Save the position of the table.
  self.table.topY = $(self.table).offset().top;
  self.table.bottomY = self.table.topY + self.table.offsetHeight;

  // Add classes to the handle and row.
  $(item).addClass('drag');

  // Set the document to use the move cursor during drag.
  $('body').addClass('drag');
  if (self.oldRowElement) {
    $(self.oldRowElement).removeClass('drag-previous');
  }
};

/**
 * Pointer movement handler, bound to document.
 */
Backdrop.tableDrag.prototype.dragRow = function (event, self) {
  if (self.dragObject) {
    self.currentPointerCoords = self.pointerCoords(event);
    var y = self.currentPointerCoords.y - self.dragObject.initOffset.y;
    var x = self.currentPointerCoords.x - self.dragObject.initOffset.x;

    // Check for row swapping and vertical scrolling.
    if (y !== self.oldY) {
      self.rowObject.direction = y > self.oldY ? 'down' : 'up';
      self.oldY = y; // Update the old value.

      // Check if the window should be scrolled (and how fast).
      var scrollAmount = self.checkScroll(self.currentPointerCoords.y);
      // Stop any current scrolling.
      clearInterval(self.scrollInterval);
      // Continue scrolling if the pointer has moved in the scroll direction.
      if (scrollAmount > 0 && self.rowObject.direction === 'down' || scrollAmount < 0 && self.rowObject.direction === 'up') {
        self.setScroll(scrollAmount);
      }

      // If we have a valid target, perform the swap and restripe the table.
      var currentRow = self.findDropTargetRow(x, y);
      if (currentRow) {
        if (self.rowObject.direction === 'down') {
          self.rowObject.swap('after', currentRow, self);
        }
        else {
          self.rowObject.swap('before', currentRow, self);
        }
        self.restripeTable();
      }
    }

    // Similar to row swapping, handle indentations.
    if (self.indentEnabled) {
      var xDiff = self.currentPointerCoords.x - self.dragObject.indentPointerPos.x;
      // Set the number of indentations the pointer has been moved left or right.
      var indentDiff = Math.round(xDiff / self.indentAmount * self.rtl);
      // Indent the row with our estimated diff, which may be further
      // restricted according to the rows around this row.
      var indentChange = self.rowObject.indent(indentDiff);
      // Update table and pointer indentations.
      self.dragObject.indentPointerPos.x += self.indentAmount * indentChange * self.rtl;
      self.indentCount = Math.max(self.indentCount, self.rowObject.indents);
    }

    return false;
  }
};

/**
 * Pointerup behavior.
 */
Backdrop.tableDrag.prototype.dropRow = function (event, self) {
  var droppedRow, $droppedRow;

  // Drop row functionality.
  if (self.rowObject !== null) {
    droppedRow = self.rowObject.element;
    $droppedRow = $(droppedRow);
    // The row is already in the right place so we just release it.
    if (self.rowObject.changed === true) {
      // Update the fields in the dropped row.
      self.updateFields(droppedRow);

      // If a setting exists for affecting the entire group, update all the
      // fields in the entire dragged group.
      for (var group in self.tableSettings) {
        if (self.tableSettings.hasOwnProperty(group)) {
          var rowSettings = self.rowSettings(group, droppedRow);
          if (rowSettings.relationship === 'group') {
            for (var n in self.rowObject.children) {
              if (self.rowObject.children.hasOwnProperty(n)) {
                self.updateField(self.rowObject.children[n], group);
              }
            }
          }
        }
      }

      self.rowObject.markChanged();
      if (self.changed === false) {
        $(Backdrop.theme('tableDragChangedWarning')).insertBefore(self.table).hide().fadeIn('slow');
        self.changed = true;
      }
    }

    if (self.indentEnabled) {
      self.rowObject.removeIndentClasses();
    }
    if (self.oldRowElement) {
      $(self.oldRowElement).removeClass('drag-previous');
    }
    $droppedRow.removeClass('drag').addClass('drag-previous');
    self.oldRowElement = droppedRow;
    self.onDrop();
    self.rowObject = null;
  }

  // Functionality specific only to pointerup events.
  if (self.dragObject !== null) {
    $('.tabledrag-handle', droppedRow).removeClass('tabledrag-handle-hover');

    self.dragObject = null;
    $('body').removeClass('drag');
    clearInterval(self.scrollInterval);
  }
};


/**
 * Get the coordinates from the event (allowing for browser differences).
 */
Backdrop.tableDrag.prototype.pointerCoords = function (event) {
  // Complete support for pointer events was only introduced to jQuery in
  // version 1.11.1; between versions 1.7 and 1.11.0 pointer events have the
  // clientX and clientY properties undefined. In those cases, the properties
  // must be retrieved from the event.originalEvent object instead.
  var clientX = event.clientX || event.originalEvent.clientX;
  var clientY = event.clientY || event.originalEvent.clientY;

  if (event.pageX || event.pageY) {
    return { x: event.pageX, y: event.pageY };
  }

  return {
    x: clientX + document.body.scrollLeft - document.body.clientLeft,
    y: clientY + document.body.scrollTop  - document.body.clientTop
  };
};

/**
 * Given a target element and a pointer event, get the event offset from that
 * element. To do this we need the element's position and the target position.
 */
Backdrop.tableDrag.prototype.getPointerOffset = function (target, event) {
  var docPos   = $(target).offset();
  var pointerPos = this.pointerCoords(event);
  return { x: pointerPos.x - docPos.left, y: pointerPos.y - docPos.top };
};

/**
 * Find the row the pointer is currently over. This row is then taken and
 * swapped with the one being dragged.
 *
 * @param x
 *   The x coordinate of the pointer on the page (not the screen).
 * @param y
 *   The y coordinate of the pointer on the page (not the screen).
 */
Backdrop.tableDrag.prototype.findDropTargetRow = function (x, y) {
  var rows = $(this.table.tBodies[0].rows).not(':hidden');
  for (var n = 0; n < rows.length; n++) {
    var row = rows[n];
    var rowY = $(row).offset().top;
    var rowHeight;
    // Because Safari does not report offsetHeight on table rows, but does on
    // table cells, grab the firstChild of the row and use that instead.
    // http://jacob.peargrove.com/blog/2006/technical/table-row-offsettop-bug-in-safari.
    if (row.offsetHeight == 0) {
      rowHeight = parseInt(row.firstChild.offsetHeight, 10) / 2;
    }
    // Other browsers.
    else {
      rowHeight = parseInt(row.offsetHeight, 10) / 2;
    }

    // Because we always insert before, we need to offset the height a bit.
    if ((y > (rowY - rowHeight)) && (y < (rowY + rowHeight))) {
      if (this.indentEnabled) {
        // Check that this row is not a child of the row being dragged.
        for (n in this.rowObject.group) {
          if (this.rowObject.group[n] == row) {
            return null;
          }
        }
      }
      else {
        // Do not allow a row to be swapped with itself.
        if (row == this.rowObject.element) {
          return null;
        }
      }

      // Check that swapping with this row is allowed.
      if (!this.rowObject.isValidSwap(row)) {
        return null;
      }

      // We may have found the row the pointer just passed over, but it doesn't
      // take into account hidden rows. Skip backwards until we find a draggable
      // row.
      while ($(row).is(':hidden') && $(row).prev('tr').is(':hidden')) {
        row = $(row).prev('tr').get(0);
      }
      return row;
    }
  }
  return null;
};

/**
 * After the row is dropped, update the table fields according to the settings
 * set for this table.
 *
 * @param changedRow
 *   DOM object for the row that was just dropped.
 */
Backdrop.tableDrag.prototype.updateFields = function (changedRow) {
  for (var group in this.tableSettings) {
    if (this.tableSettings.hasOwnProperty(group)) {
      // Each group may have a different setting for relationship, so we find
      // the source rows for each separately.
      this.updateField(changedRow, group);
    }
  }
};

/**
 * After the row is dropped, update a single table field according to specific
 * settings.
 *
 * @param changedRow
 *   DOM object for the row that was just dropped.
 * @param group
 *   The settings group on which field updates will occur.
 */
Backdrop.tableDrag.prototype.updateField = function (changedRow, group) {
  var rowSettings = this.rowSettings(group, changedRow);
  var sourceRow, nextRow, previousRow, useSibling;

  // Set the row as its own target.
  if (rowSettings.relationship == 'self' || rowSettings.relationship == 'group') {
    sourceRow = changedRow;
  }
  // Siblings are easy, check previous and next rows.
  else if (rowSettings.relationship == 'sibling') {
    previousRow = $(changedRow).prev('tr').get(0);
    nextRow = $(changedRow).next('tr').get(0);
    sourceRow = changedRow;
    if ($(previousRow).is('.draggable') && $('.' + group, previousRow).length) {
      if (this.indentEnabled) {
        if ($('.indentations', previousRow).length == $('.indentations', changedRow)) {
          sourceRow = previousRow;
        }
      }
      else {
        sourceRow = previousRow;
      }
    }
    else if ($(nextRow).is('.draggable') && $('.' + group, nextRow).length) {
      if (this.indentEnabled) {
        if ($('.indentations', nextRow).length == $('.indentations', changedRow)) {
          sourceRow = nextRow;
        }
      }
      else {
        sourceRow = nextRow;
      }
    }
  }
  // Parents, look up the tree until we find a field not in this group.
  // Go up as many parents as indentations in the changed row.
  else if (rowSettings.relationship == 'parent') {
    previousRow = $(changedRow).prev('tr');
    while (previousRow.length && $('.indentation', previousRow).length >= this.rowObject.indents) {
      previousRow = previousRow.prev('tr');
    }
    // If we found a row.
    if (previousRow.length) {
      sourceRow = previousRow[0];
    }
    // Otherwise we went all the way to the left of the table without finding
    // a parent, meaning this item has been placed at the root level.
    else {
      // Use the first row in the table as source, because it's guaranteed to
      // be at the root level. Find the first item, then compare this row
      // against it as a sibling.
      sourceRow = $(this.table).find('tr.draggable:first').get(0);
      if (sourceRow == this.rowObject.element) {
        sourceRow = $(this.rowObject.group[this.rowObject.group.length - 1]).next('tr.draggable').get(0);
      }
      useSibling = true;
    }
  }

  // Because we may have moved the row from one category to another,
  // take a look at our sibling and borrow its sources and targets.
  this.copyDragClasses(sourceRow, changedRow, group);
  rowSettings = this.rowSettings(group, changedRow);

  // In the case that we're looking for a parent, but the row is at the top
  // of the tree, copy our sibling's values.
  if (useSibling) {
    rowSettings.relationship = 'sibling';
    rowSettings.source = rowSettings.target;
  }

  var targetClass = '.' + rowSettings.target;
  var targetElement = $(targetClass, changedRow).get(0);

  // Check if a target element exists in this row.
  if (targetElement) {
    var sourceClass = '.' + rowSettings.source;
    var sourceElement = $(sourceClass, sourceRow).get(0);
    switch (rowSettings.action) {
      case 'depth':
        // Get the depth of the target row.
        targetElement.value = $('.indentation', $(sourceElement).closest('tr')).length;
        break;
      case 'match':
        // Update the value.
        targetElement.value = sourceElement.value;
        break;
      case 'order':
        var siblings = this.rowObject.findSiblings(rowSettings);
        if ($(targetElement).is('select')) {
          // Get a list of acceptable values.
          var values = [];
          $('option', targetElement).each(function () {
            values.push(this.value);
          });
          var maxVal = values[values.length - 1];
          // Populate the values in the siblings.
          $(targetClass, siblings).each(function () {
            // If there are more items than possible values, assign the maximum value to the row.
            if (values.length > 0) {
              this.value = values.shift();
            }
            else {
              this.value = maxVal;
            }
          });
        }
        else {
          // Assume a numeric input field.
          var weight = parseInt($(targetClass, siblings[0]).val(), 10) || 0;
          $(targetClass, siblings).each(function () {
            this.value = weight;
            weight++;
          });
        }
        break;
    }
  }
};

/**
 * Copy all special tableDrag classes from one row's form elements to a
 * different one, removing any special classes that the destination row
 * may have had.
 */
Backdrop.tableDrag.prototype.copyDragClasses = function (sourceRow, targetRow, group) {
  var sourceElement = $('.' + group, sourceRow);
  var targetElement = $('.' + group, targetRow);
  if (sourceElement.length && targetElement.length) {
    targetElement[0].className = sourceElement[0].className;
  }
};

Backdrop.tableDrag.prototype.checkScroll = function (cursorY) {
  var de  = document.documentElement;
  var b  = document.body;

  var windowHeight = this.windowHeight = window.innerHeight || (de.clientHeight && de.clientWidth != 0 ? de.clientHeight : b.offsetHeight);
  var scrollY = this.scrollY = (document.all ? (!de.scrollTop ? b.scrollTop : de.scrollTop) : (window.pageYOffset ? window.pageYOffset : window.scrollY));
  var trigger = this.scrollSettings.trigger;
  var delta = 0;

  // Return a scroll speed relative to the edge of the screen.
  if (cursorY - scrollY > windowHeight - trigger) {
    delta = trigger / (windowHeight + scrollY - cursorY);
    delta = (delta > 0 && delta < trigger) ? delta : trigger;
    return delta * this.scrollSettings.amount;
  }
  else if (cursorY - scrollY < trigger) {
    delta = trigger / (cursorY - scrollY);
    delta = (delta > 0 && delta < trigger) ? delta : trigger;
    return -delta * this.scrollSettings.amount;
  }
};

Backdrop.tableDrag.prototype.setScroll = function (scrollAmount) {
  var self = this;

  this.scrollInterval = setInterval(function () {
    // Update the scroll values stored in the object.
    self.checkScroll(self.currentPointerCoords.y);
    var aboveTable = self.scrollY > self.table.topY;
    var belowTable = self.scrollY + self.windowHeight < self.table.bottomY;
    if (scrollAmount > 0 && belowTable || scrollAmount < 0 && aboveTable) {
      window.scrollBy(0, scrollAmount);
    }
  }, this.scrollSettings.interval);
};

Backdrop.tableDrag.prototype.restripeTable = function () {
  // :even and :odd are reversed because jQuery counts from 0 and
  // we count from 1, so we're out of sync.
  // Match immediate children of the parent element to allow nesting.
  $('> tbody > tr.draggable:visible, > tr.draggable:visible', this.table)
    .removeClass('odd even')
    .filter(':odd').addClass('even').end()
    .filter(':even').addClass('odd');
};

/**
 * Stub function. Allows a custom handler when a row begins dragging.
 */
Backdrop.tableDrag.prototype.onDrag = function () {
  return null;
};

/**
 * Stub function. Allows a custom handler when a row is dropped.
 */
Backdrop.tableDrag.prototype.onDrop = function () {
  return null;
};

/**
 * Constructor to make a new object to manipulate a table row.
 *
 * @param tableRow
 *   The DOM element for the table row we will be manipulating.
 * @param method
 *   The method in which this row is being moved. Either 'keyboard' or 'pointer'.
 * @param indentEnabled
 *   Whether the containing table uses indentations. Used for optimizations.
 * @param maxDepth
 *   The maximum amount of indentations this row may contain.
 * @param addClasses
 *   Whether we want to add classes to this row to indicate child relationships.
 */
Backdrop.tableDrag.prototype.row = function (tableRow, method, indentEnabled, maxDepth, addClasses) {
  var $tableRow = $(tableRow)
  this.element = tableRow;
  this.method = method;
  this.group = [tableRow];
  this.groupDepth = $tableRow.find('.indentation').length;
  this.changed = tableRow;
  this.table = $tableRow.closest('table')[0];
  this.indentEnabled = indentEnabled;
  this.maxDepth = maxDepth;
  this.direction = ''; // Direction the row is being moved.

  if (this.indentEnabled) {
    this.indents = this.groupDepth;
    this.children = this.findChildren(addClasses);
    this.group = $.merge(this.group, this.children);
    // Find the depth of this entire group.
    for (var n = 0; n < this.group.length; n++) {
      this.groupDepth = Math.max($('.indentation', this.group[n]).length, this.groupDepth);
    }
  }
};

/**
 * Find all children of rowObject by indentation.
 *
 * @param addClasses
 *   Whether we want to add classes to this row to indicate child relationships.
 */
Backdrop.tableDrag.prototype.row.prototype.findChildren = function (addClasses) {
  var parentIndentation = this.indents;
  var $currentRow = $(this.element, this.table).next('tr.draggable');
  var rows = [];
  var child = 0;
  var rowIndentation;
  while ($currentRow.length) {
    rowIndentation = $currentRow.find('.indentation').length;
    // A greater indentation indicates this is a child.
    if (rowIndentation > parentIndentation) {
      child++;
      rows.push($currentRow[0]);
      if (addClasses) {
        $currentRow.find('.indentation').each(function (indentNum) {
          if (child == 1 && (indentNum == parentIndentation)) {
            $(this).addClass('tree-child-first');
          }
          if (indentNum == parentIndentation) {
            $(this).addClass('tree-child');
          }
          else if (indentNum > parentIndentation) {
            $(this).addClass('tree-child-horizontal');
          }
        });
      }
    }
    else {
      break;
    }
    $currentRow = $currentRow.next('tr.draggable');
  }
  if (addClasses && rows.length) {
    $('.indentation:nth-child(' + (parentIndentation + 1) + ')', rows[rows.length - 1]).addClass('tree-child-last');
  }
  return rows;
};

/**
 * Ensure that two rows are allowed to be swapped.
 *
 * @param row
 *   DOM object for the row being considered for swapping.
 */
Backdrop.tableDrag.prototype.row.prototype.isValidSwap = function (row) {
  if (this.indentEnabled) {
    var prevRow, nextRow;
    if (this.direction == 'down') {
      prevRow = row;
      nextRow = $(row).next('tr').get(0);
    }
    else {
      prevRow = $(row).prev('tr').get(0);
      nextRow = row;
    }
    this.interval = this.validIndentInterval(prevRow, nextRow);

    // We have an invalid swap if the valid indentations interval is empty.
    if (this.interval.min > this.interval.max) {
      return false;
    }
  }

  // Do not let an un-draggable first row have anything put before it.
  if (this.table.tBodies[0].rows[0] == row && $(row).is(':not(.draggable)')) {
    return false;
  }

  return true;
};

/**
 * Perform the swap between two rows.
 *
 * @param position
 *   Whether the swap will occur 'before' or 'after' the given row.
 * @param row
 *   DOM element what will be swapped with the row group.
 */
Backdrop.tableDrag.prototype.row.prototype.swap = function (position, row) {
  Backdrop.detachBehaviors(this.group, Backdrop.settings, 'move');
  $(row)[position](this.group);
  Backdrop.attachBehaviors(this.group, Backdrop.settings);
  this.changed = true;
  this.onSwap(row);
};

/**
 * Determine the valid indentations interval for the row at a given position
 * in the table.
 *
 * @param prevRow
 *   DOM object for the row before the tested position
 *   (or null for first position in the table).
 * @param nextRow
 *   DOM object for the row after the tested position
 *   (or null for last position in the table).
 */
Backdrop.tableDrag.prototype.row.prototype.validIndentInterval = function (prevRow, nextRow) {
  var minIndent, maxIndent;

  // Minimum indentation:
  // Do not orphan the next row.
  minIndent = nextRow ? $('.indentation', nextRow).length : 0;

  // Maximum indentation:
  if (!prevRow || $(prevRow).is(':not(.draggable)') || $(this.element).is('.tabledrag-root')) {
    // Do not indent:
    // - the first row in the table,
    // - rows dragged below a non-draggable row,
    // - 'root' rows.
    maxIndent = 0;
  }
  else {
    // Do not go deeper than as a child of the previous row.
    maxIndent = $('.indentation', prevRow).length + ($(prevRow).is('.tabledrag-leaf') ? 0 : 1);
    // Limit by the maximum allowed depth for the table.
    if (this.maxDepth) {
      maxIndent = Math.min(maxIndent, this.maxDepth - (this.groupDepth - this.indents));
    }
  }

  return { 'min': minIndent, 'max': maxIndent };
};

/**
 * Indent a row within the legal bounds of the table.
 *
 * @param indentDiff
 *   The number of additional indentations proposed for the row (can be
 *   positive or negative). This number will be adjusted to nearest valid
 *   indentation level for the row.
 */
Backdrop.tableDrag.prototype.row.prototype.indent = function (indentDiff) {
  // Determine the valid indentations interval if not available yet.
  if (!this.interval) {
    var prevRow = $(this.element).prev('tr').get(0);
    var nextRow = $(this.group).filter(':last').next('tr').get(0);
    this.interval = this.validIndentInterval(prevRow, nextRow);
  }

  // Adjust to the nearest valid indentation.
  var indent = this.indents + indentDiff;
  indent = Math.max(indent, this.interval.min);
  indent = Math.min(indent, this.interval.max);
  indentDiff = indent - this.indents;

  for (var n = 1; n <= Math.abs(indentDiff); n++) {
    // Add or remove indentations.
    if (indentDiff < 0) {
      $('.indentation:first', this.group).remove();
      this.indents--;
    }
    else {
      $('td:first', this.group).prepend(Backdrop.theme('tableDragIndentation'));
      this.indents++;
    }
  }
  if (indentDiff) {
    // Update indentation for this row.
    this.changed = true;
    this.groupDepth += indentDiff;
    this.onIndent();
  }

  return indentDiff;
};

/**
 * Find all siblings for a row, either according to its subgroup or indentation.
 * Note that the passed-in row is included in the list of siblings.
 *
 * @param settings
 *   The field settings we're using to identify what constitutes a sibling.
 */
Backdrop.tableDrag.prototype.row.prototype.findSiblings = function (rowSettings) {
  var siblings = [];
  var directions = ['prev', 'next'];
  var rowIndentation = this.indents;
  var checkRowIndentation;
  for (var d = 0; d < directions.length; d++) {
    var checkRow = $(this.element)[directions[d]]();
    while (checkRow.length) {
      // Check that the sibling contains a similar target field.
      if ($('.' + rowSettings.target, checkRow)) {
        // Either add immediately if this is a flat table, or check to ensure
        // that this row has the same level of indentation.
        if (this.indentEnabled) {
          checkRowIndentation = checkRow.find('.indentation').length;
        }

        if (!(this.indentEnabled) || (checkRowIndentation == rowIndentation)) {
          siblings.push(checkRow[0]);
        }
        else if (checkRowIndentation < rowIndentation) {
          // No need to keep looking for siblings when we get to a parent.
          break;
        }
      }
      else {
        break;
      }
      checkRow = $(checkRow)[directions[d]]();
    }
    // Since siblings are added in reverse order for previous, reverse the
    // completed list of previous siblings. Add the current row and continue.
    if (directions[d] == 'prev') {
      siblings.reverse();
      siblings.push(this.element);
    }
  }
  return siblings;
};

/**
 * Remove indentation helper classes from the current row group.
 */
Backdrop.tableDrag.prototype.row.prototype.removeIndentClasses = function () {
  for (var n in this.children) {
    if (this.children.hasOwnProperty(n)) {
       $(this.children[n]).find('.indentation')
         .removeClass('tree-child')
         .removeClass('tree-child-first')
         .removeClass('tree-child-last')
         .removeClass('tree-child-horizontal');
     }
  }
};

/**
 * Add an asterisk or other marker to the changed row.
 */
Backdrop.tableDrag.prototype.row.prototype.markChanged = function () {
  var marker = Backdrop.theme('tableDragChangedMarker');
  var cell = $('td:first', this.element);
  if ($('abbr.tabledrag-changed', cell).length == 0) {
    cell.append(marker);
  }
};

/**
 * Stub function. Allows a custom handler when a row is indented.
 */
Backdrop.tableDrag.prototype.row.prototype.onIndent = function () {
  return null;
};

/**
 * Stub function. Allows a custom handler when a row is swapped.
 */
Backdrop.tableDrag.prototype.row.prototype.onSwap = function (swappedRow) {
  return null;
};

Backdrop.theme.prototype.tableDragHandle = function () {
  return '<a href="#" title="' + Backdrop.t('Drag to re-order') + '" class="tabledrag-handle"><div class="handle">&nbsp;</div></a>';
};

Backdrop.theme.prototype.tableDragChangedMarker = function () {
  return '<abbr class="warning tabledrag-changed" title="' + Backdrop.t('Changed') + '">*</abbr>';
};

Backdrop.theme.prototype.tableDragIndentation = function () {
  return '<div class="indentation">&nbsp;</div>';
};

Backdrop.theme.prototype.tableDragChangedWarning = function () {
  return '<div class="tabledrag-changed-warning messages warning">' + Backdrop.theme('tableDragChangedMarker') + ' ' + Backdrop.t('Changes made in this table will not be saved until the form is submitted.') + '</div>';
};

})(jQuery);
