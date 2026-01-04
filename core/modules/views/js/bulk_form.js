(function ($) {

Backdrop.behaviors.bulk_form = {
  attach: function(context) {
    $('.views-form', context).each(function() {
      Backdrop.bulk_form.initTableBehaviors(this);
      Backdrop.bulk_form.initGenericBehaviors(this);
    });
  }
}

Backdrop.bulk_form = Backdrop.bulk_form || {};
Backdrop.bulk_form.initTableBehaviors = function(form) {
  // If the table is not grouped, "Select all on this page / all pages"
  // markup gets inserted below the table header.
  var selectAllMarkup = $('.bulk-form-table-select-all-markup', form);
  if (selectAllMarkup.length) {
    $('.views-table > tbody', form).prepend('<tr class="bulk-form-views-table-row-select-all even">></tr>');
    var colspan = $('table th', form).length;
    // Add the select all pages markup as the first row spanning all columns.
    $('.bulk-form-views-table-row-select-all', form).html('<td colspan="' + colspan + '">' + selectAllMarkup.html() + '</td>');

    $('.bulk-form-table-select-all-pages', form).on('click', function() {
      Backdrop.bulk_form.tableSelectAllPages(form);
      return false;
    });
    $('.bulk-form-table-select-this-page', form).on('click', function() {
      Backdrop.bulk_form.tableSelectThisPage(form);
      return false;
    });
  }

  // This is the "select all" checkbox in (each) table header.
  $('th.select-all input:checkbox', form).on('change', function() {
    var table = $(this).closest('table:not(.sticky-header)')[0];

    // Toggle the visibility of the "select all" row (if any).
    if (this.checked) {
      $('.bulk-form-views-table-row-select-all', table).show();
    }
    else {
      $('.bulk-form-views-table-row-select-all', table).hide();
      // Disable "select all across pages".
      Backdrop.bulk_form.tableSelectThisPage(form);
    }
  });
}

/**
 * Prepares the select all across pages functionality.
 */
Backdrop.bulk_form.tableSelectAllPages = function(form) {
  $('.bulk-form-table-this-page', form).hide();
  $('.bulk-form-table-all-pages', form).show();
  // Modify the value of the hidden form flag field.
  $('.bulk-form-select-all-pages-flag', form).val('1');
}
/**
 * Prepares the select all on this page functionality.
 */
Backdrop.bulk_form.tableSelectThisPage = function(form) {
  $('.bulk-form-table-all-pages', form).hide();
  $('.bulk-form-table-this-page', form).show();
  // Modify the value of the hidden form field.
  $('.bulk-form-select-all-pages-flag', form).val('0');
}

Backdrop.bulk_form.initGenericBehaviors = function(form) {
  // Show the "select all" fieldset for non-tables.
  $('.bulk-form-select-all-markup', form).show();

  // Listener for the non-table page-wise "select all" checkbox. 
  $('.bulk-form-select-this-page', form).on('click', function() {
    // Check or uncheck all checkbox within this page.
    $('input:checkbox[name^="bulk_form"]', form).prop('checked', this.checked);
    // Uncheck the "select all items in all pages" checkbox.
    $('.bulk-form-select-all-pages', form).prop('checked', false);

    // Toggle the "select all" checkbox in grouped tables (if any).
    $('.bulk-form-table-select-all', form).prop('checked', this.checked);
  });

  // Listener for the non-table "select all in all pages" checkbox. 
  $('.bulk-form-select-all-pages', form).on('click', function() {
    $('input:checkbox[name^="bulk_form"]', form).prop('checked', this.checked);
    // Uncheck the "select all" checkbox.
    $('.bulk-form-select-this-page', form).prop('checked', false);

    // Toggle the "select all" checkbox in grouped tables (if any).
    $('.bulk-form-table-select-all', form).prop('checked', this.checked);

    // Modify the value of the hidden form field.
    $('.bulk-form-select-all-pages-flag', form).val(this.checked);
  });

  $('input:checkbox[name^="bulk_form"]', form).on('click', function() {
    // If a checkbox was deselected, uncheck any "select all" checkboxes.
    if (!this.checked) {
      // Uncheck the select all checkboxes for non-tables.
      $('.bulk-form-select-this-page', form).prop('checked', false);
      $('.bulk-form-select-all-pages', form).prop('checked', false);
      // Modify the value of the hidden form field.
      $('.bulk-form-select-all-pages-flag', form).val('0')

      var table = $(this).closest('table')[0];
      if (table) {
        // If there's a "select all" row, hide it.
        if ($('.bulk-form-table-select-this-page', table).length) {
          $('.bulk-form-views-table-row-select-all', table).hide();
          // Disable "select all across pages".
          Backdrop.bulk_form.tableSelectThisPage(form);
        }
      }
    }

  });
}

})(jQuery);
