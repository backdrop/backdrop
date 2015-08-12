/**
 * @file
 * Attaches administration-specific behavior for the Filter module.
 */

(function ($) {

Backdrop.behaviors.filterStatus = {
  attach: function (context, settings) {
    // tableDrag is required and we should be on the filters admin page.
    if (typeof Backdrop.tableDrag == 'undefined' || typeof Backdrop.tableDrag.filterorder == 'undefined') {
      return;
    }
    var table = $('table#filterorder');
    var tableDrag = Backdrop.tableDrag.filterorder; // Get the filters tableDrag object.

    // Add a handler for when a row is swapped, update empty regions.
    tableDrag.row.prototype.onSwap = function (swappedRow) {
      checkEmptyRegions(table, this);
    };

    // A custom message for the filters page specifically.
    Backdrop.theme.tableDragChangedWarning = function () {
      return '<div class="messages warning">' + Backdrop.theme('tableDragChangedMarker') + ' ' + Backdrop.t('The changes to these filters will not be saved until the <em>Save filters</em> button is clicked.') + '</div>';
    };

    // Add a handler so when a row is dropped, update the status checkbox.
    tableDrag.onDrop = function () {
      dragObject = this;
      if(dragObject.changed == true) {
        dropRow = dragObject.rowObject.element;
        enabling = $(dropRow).nextAll('.disable-label').length;
        $(dropRow).find('input.filter-status:checkbox').prop('checked', enabling);
      }
    };

    // Add the behavior to move rows to the appropriate section if the status
    // checkbox is checked.
    $('#filterorder input:checkbox', context).change(function (event) {
        // Make our new row and select field.
        var row = $(this).closest('tr');
        tableDrag.rowObject = new tableDrag.row(row);
        // Find the correct region and insert the row as the last in the region.
        var labelRow = $('.disable-label');
        if ($(this).prop('checked')) {
          labelRow.before(row);
        }
        else {
          labelRow.after(row);
       }
        

        // Modify empty regions.
        checkEmptyRegions(table, row);
        // Remove focus from selectbox.
        $(this).get(0).blur();
    });

    // If a region becomes empty, add empty text.
    var checkEmptyRegions = function (table, rowObject) {
      // If the "Disabled filters" section has become empty, add empty text. 
      // Remove the text if the region is not empty.
      var disableRegion = $('.disable-label');
      if (disableRegion.next('tr.draggable').length == 0) {
        if (disableRegion.next('tr.region-empty').length == 0) {
          $('table#filterorder tr:last').after('<tr class="region-empty description"><td colspan=3>No disabled filters.</td></tr>');
        }
      }
      else {
        disableRegion.nextAll('.region-empty').remove();
      }
      // If the "Enabled filters" section has become empty, add empty text. 
      // Remove the text if the region is not empty.
      if (disableRegion.prev('tr.draggable').length == 0) {
        if (disableRegion.prev('tr.region-empty').length == 0) {
          $('table#filterorder').prepend('<tr class="region-empty description"><td colspan=3>No enabled filters.</td></tr>');
        }
      }
      else {
        disableRegion.prevAll('.region-empty').remove();
      }
    };
    
    // Add modal for configure link
    $('.filter-configure').hide();

    $(".configure-link").click(function (event) {
      event.preventDefault();
      row = $(this).closest('tr');
      row.find('.filter-configure').dialog({
        draggable: false,
        width: "500px",
        modal: true,
        close: function( event, ui ) {
          $(this).dialog( "destroy");
        },
        title: "Configure settings",
        buttons: {
          "Update settings": function () {
            $(this).dialog("close");
            row = new tableDrag.row(row);
            row.markChanged();
            if (tableDrag.changed === false) {
              $(Backdrop.theme('tableDragChangedWarning')).insertBefore(table).hide().fadeIn('slow');
              tableDrag.changed = true;
            }
         }
        }
      });
    });
  }
};

})(jQuery);
