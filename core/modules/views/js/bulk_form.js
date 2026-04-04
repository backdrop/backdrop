(function ($) {

"use strict";

Backdrop.behaviors.viewsBulkForm = {
  attach: function(context) {
    $('.views-form', context).each(function() {
      Backdrop.viewsBulkForm.initTableBehaviors(this);
      Backdrop.viewsBulkForm.initGenericBehaviors(this);
    });
  }
};

Backdrop.viewsBulkForm = Backdrop.viewsBulkForm || {};
Backdrop.viewsBulkForm.initTableBehaviors = function(form) {
  // If the table is not grouped, "Select all on this page / all pages"
  // markup gets inserted below the table header.
  var $selectAllElement = $('.views-select-all-pages--wrapper', form);
  if ($selectAllElement.length) {
    $('.views-table > tbody', form).prepend('<tr class="views-select-all-pages--row even">></tr>');
    var colspan = $('table th', form).length;

    // Add the select all pages markup as the first row spanning all columns.
    $('.views-select-all-pages--row', form).html('<td colspan="' + colspan + '"></td>');
    $('.views-select-all-pages--row td', form).prepend($selectAllElement);

    $('.views-select-all-pages--all-pages-button', form).on('click', function() {
      Backdrop.viewsBulkForm.tableSelectAllPages(form);
      return false;
    });
    $('.views-select-all-pages--this-page-button', form).on('click', function() {
      Backdrop.viewsBulkForm.tableSelectThisPage(form);
      return false;
    });
  }

  // This is the "select all" checkbox in (each) table header.
  $('th.select-all input:checkbox', form).on('change', function() {
    var table = $(this).closest('table:not(.sticky-header)')[0];

    // Toggle the visibility of the "select all" row (if any).
    if (this.checked) {
      $('.views-select-all-pages--row', table).show();
    }
    else {
      $('.views-select-all-pages--row', table).hide();
      // Disable "select all across pages".
      Backdrop.viewsBulkForm.tableSelectThisPage(form);
    }
  });
};

/**
 * Prepares the select all across pages functionality.
 */
Backdrop.viewsBulkForm.tableSelectAllPages = function(form) {
  $('.views-select-all-pages--this-page', form).hide();
  $('.views-select-all-pages--all-pages', form).show();
  // Modify the value of the hidden form flag field.
  $('input[name="select_all_pages"]', form).val('1');
};

/**
 * Prepares the select all on this page functionality.
 */
Backdrop.viewsBulkForm.tableSelectThisPage = function(form) {
  $('.views-select-all-pages--all-pages', form).hide();
  $('.views-select-all-pages--this-page', form).show();
  // Modify the value of the hidden form field.
  $('input[name="select_all_pages"]', form).val('0');
};

Backdrop.viewsBulkForm.initGenericBehaviors = function(form) {
  // Show the "select all" fieldset for non-tables.
  $('.views-select-all-pages--wrapper', form).show();

  // Listener for the non-table page-wise "select all" checkbox.
  $('.views-select-all-pages--this-page-checkbox', form).on('change', function() {
    // Check or uncheck all checkbox within this page.
    $('input:checkbox[name^="bulk_form"]', form).prop('checked', this.checked);

    // Uncheck the "select all items in all pages" checkbox.
    $('.views-select-all-pages--all-pages-checkbox', form).prop('checked', false);

    // Toggle the "select all" checkbox in grouped tables (if any).
    $('.bulk-form-table-select-all', form).prop('checked', this.checked);
  });

  // Listener for the non-table "select all in all pages" checkbox.
  $('.views-select-all-pages--all-pages-checkbox', form).on('change', function() {
    $('input:checkbox[name^="bulk_form"]', form).prop('checked', this.checked);

    // Uncheck the "select all" checkbox.
    $('.views-select-all-pages--this-page-checkbox', form).prop('checked', false);

    // Toggle the "select all" checkbox in grouped tables (if any).
    $('.views-select-all-pages--all-pages-checkbox', form).prop('checked', this.checked);

    // Modify the value of the hidden form field.
    $('input[name="select_all_pages"]', form).val(this.checked ? '1' : '0');
  });

  $('input:checkbox[name^="bulk_form"]', form).on('change', function() {
    // If a checkbox was deselected, uncheck any "select all" checkboxes.
    if (!this.checked) {
      // Uncheck the select all checkboxes for non-tables.
      $('.views-select-all-pages--this-page-checkbox', form).prop('checked', false);
      $('.views-select-all-pages--all-pages-checkbox', form).prop('checked', false);

      // Modify the value of the hidden form field.
      $('input[name="select_all_pages"]', form).val('0');

      var table = $(this).closest('table')[0];
      if (table) {
        // If there's a "select all" row, hide it.
        if ($('.views-select-all-pages--row', table).length) {
          $('.views-select-all-pages--row', table).hide();
          // Disable "select all across pages".
          Backdrop.viewsBulkForm.tableSelectThisPage(form);
        }
      }
    }

  });
};

})(jQuery);
