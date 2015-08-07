/**
 * @file
 * Attaches administration-specific behavior for the Filter module.
 */

(function ($) {

Backdrop.behaviors.filterStatus = {
  attach: function (context, settings) {
    // tableDrag is required and we should be on the filters admin page.
    // if (typeof Backdrop.tableDrag == 'undefined' || typeof Backdrop.tableDrag.filters == 'undefined') {
    if (typeof Backdrop.tableDrag == 'undefined') {
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

    // Add a handler so when a row is dropped, update fields dropped into new regions.
    tableDrag.onDrop = function () {
      dragObject = this;
      // Use "region-message" row instead of "region" row because
      // "region-{region_name}-message" is less prone to regexp match errors.
      dropIndex = dragObject.rowObject.element.rowIndex;
      if(dragObject.changed == true) {
        disableIndex = $('#filterorder tr.disable-label').index();
        enabling = dropIndex < disableIndex;
        dropRow = dragObject.rowObject.element;
        $(dropRow).find('input:checkbox:first').prop('checked', enabling);
      }
    };

    // Add the behavior to each region select list.
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
        

        // Modify empty regions with added or removed fields.
        checkEmptyRegions(table, row);
        // Remove focus from selectbox.
        $(this).get(0).blur();
    });

    var checkEmptyRegions = function (table, rowObject) {
        // This region has become empty.
        if ($('.disable-label').next('tr').length == 0) {
          $('table#filterorder tr:last').after('<tr class="region-empty"><td colspan=3>Empty</td></tr>');
        }
        else {
          $('.disable-label').nextAll('.region-empty').remove();
        }
        if ($('.disable-label').prev('tr').length == 0) {
          $('table#filterorder tr:first').before('<tr class="region-empty"><td colspan=3>Empty</td></tr>');
        }
        else {
          $('.disable-label').prevAll('.region-empty').remove();
        }
    };
    
    // Add modal for configure link
    $('.filter-configure').hide();

    $(".configure-link").click(function (event) {
      event.preventDefault();
      $(this).closest('tr').find('.filter-configure').dialog({
        draggable: false,
        width: "300px",
        modal: true,
        title: "",
        buttons: {
          "Update settings": function () {
            $(this).dialog("close");
            $(this).dialog( "destroy");
          }
        }
      });
    });
  }
};

})(jQuery);
