(function ($) {

"use strict";

/**
 * Attach the tableResponsive function to Backdrop.behaviors.
 */
Backdrop.behaviors.tableResponsive = {
  attach: function (context, settings) {
    var $tables = $(context).find('table.responsive-enabled').once('tableresponsive');
    if ($tables.length) {
      for (var i = 0, il = $tables.length; i < il; i++) {
        TableResponsive.tables.push(new TableResponsive($tables[i]));
      }
    }
  }
};

/**
 * The TableResponsive object optimizes table presentation for all screen sizes.
 *
 * A responsive table hides columns at small screen sizes, leaving the most
 * important columns visible to the end user. Users should not be prevented from
 * accessing all columns, however. This class adds a toggle to a table with
 * hidden columns that exposes the columns. Exposing the columns will likely
 * break layouts, but it provides the user with a means to access data, which
 * is a guiding principle of responsive design.
 */
function TableResponsive (table) {
  this.table = table;
  this.$table = $(table);
  this.showText = Backdrop.t('Show all columns');
  this.hideText = Backdrop.t('Hide less important columns');
  // Store a reference to the header elements of the table so that the DOM is
  // traversed only once to find them.
  this.$headers = this.$table.find('th');
  // Add a link before the table for users to show or hide weight columns.
  this.$link = $('<a href="#" class="tableresponsive-toggle"></a>')
    .attr({
      'title': Backdrop.t('Show table cells that were hidden to make the table fit within a small screen.')
    })
    .on('click', $.proxy(this, 'eventhandlerToggleColumns'));

  this.$table.before($('<div class="tableresponsive-toggle-columns"></div>').append(this.$link));

  var _this = this;
  Backdrop.optimizedResize.add(function() {
    $.proxy(_this, 'eventhandlerEvaluateColumnVisibility');
    $(window).trigger('resize.tableresponsive');
  });
}

/**
 * Extend the TableResponsive function with a list of managed tables.
 */
$.extend(TableResponsive, {
  tables: []
});

/**
 * Associates an action link with the table that will show hidden columns.
 *
 * Columns are assumed to be hidden if their header has the class priority-low
 * or priority-medium.
 */
$.extend(TableResponsive.prototype, {
  eventhandlerEvaluateColumnVisibility: function (e) {
    var pegged = parseInt(this.$link.data('pegged'), 10);
    var hiddenLength = this.$headers.filter('.priority-medium:hidden, .priority-low:hidden').length;

    // If the table is not at all visible, do not manipulate the link.
    var tableVisible = this.$table.is(':visible');
    if (!tableVisible) {
      return;
    }

    // If the table has hidden columns, associate an action link with the table
    // to show the columns.
    if (hiddenLength > 0) {
      this.$link.show().text(this.showText);
    }
    // When the toggle is pegged, its presence is maintained because the user
    // has interacted with it. This is necessary to keep the link visible if the
    // user adjusts screen size and changes the visibilty of columns.
    if (!pegged && hiddenLength === 0) {
      this.$link.hide().text(this.hideText);
    }
  },
  // Toggle the visibility of columns classed with either 'priority-low' or
  // 'priority-medium'.
  eventhandlerToggleColumns: function (e) {
    e.preventDefault();
    var self = this;
    var $hiddenHeaders = this.$headers.filter('.priority-medium:hidden, .priority-low:hidden');
    this.$revealedCells = this.$revealedCells || $();
    // Reveal hidden columns.
    if ($hiddenHeaders.length > 0) {
      $hiddenHeaders.each(function (index, element) {
        var $header = $(this);
        var position = $header.prevAll('th').length;
        self.$table.find('tbody tr').each(function () {
          var $cells = $(this).find('td:eq(' + position + ')');
          $cells.show();
          // Keep track of the revealed cells, so they can be hidden later.
          self.$revealedCells = $().add(self.$revealedCells).add($cells);
        });
        $header.show();
        // Keep track of the revealed headers, so they can be hidden later.
        self.$revealedCells = $().add(self.$revealedCells).add($header);
      });
      this.$link.text(this.hideText).data('pegged', 1);
    }
    // Hide revealed columns.
    else {
      this.$revealedCells.hide();
      // Strip the 'display:none' declaration from the style attributes of
      // the table cells that .hide() added.
      this.$revealedCells.css('display', '');
      this.$link.text(this.showText).data('pegged', 0);
      // Refresh the toggle link.
      $(window).trigger('resize.tableresponsive');
    }
  }
});
// Make the TableResponsive object available in the Backdrop namespace.
Backdrop.TableResponsive = TableResponsive;

})(jQuery);
