(function ($) {

Backdrop.behaviors.bf = {
  attach: function(context) {
    $('.views-form', context).each(function() {
      Backdrop.bf.initTableBehaviors(this);
      Backdrop.bf.initGenericBehaviors(this);
    });
  }
}

Backdrop.bf = Backdrop.bf || {};
Backdrop.bf.initTableBehaviors = function(form) {
  // If the table is not grouped, "Select all on this page / all pages"
  // markup gets inserted below the table header.
  var selectAllMarkup = $('.bf-table-select-all-markup', form);
  if (selectAllMarkup.length) {
    $('.views-table > tbody', form).prepend('<tr class="bf-views-table-row-select-all even">></tr>');
    var colspan = $('table th', form).length;
    // Add the select all pages markup as the first row spanning all columns.
    $('.bf-views-table-row-select-all', form).html('<td colspan="' + colspan + '">' + selectAllMarkup.html() + '</td>');

    $('.bf-table-select-all-pages', form).on('click', function() {
      Backdrop.bf.tableSelectAllPages(form);
      return false;
    });
    $('.bf-table-select-this-page', form).on('click', function() {
      Backdrop.bf.tableSelectThisPage(form);
      return false;
    });
  }

  // This is the "select all" checkbox in (each) table header.
  $('th.select-all input:checkbox', form).on('change', function() {
    var table = $(this).closest('table:not(.sticky-header)')[0];

    // Toggle the visibility of the "select all" row (if any).
    if (this.checked) {
      $('.bf-views-table-row-select-all', table).show();
    }
    else {
      $('.bf-views-table-row-select-all', table).hide();
      // Disable "select all across pages".
      Backdrop.bf.tableSelectThisPage(form);
    }
  });
}

/**
 * Prepares the select all across pages functionality.
 */
Backdrop.bf.tableSelectAllPages = function(form) {
  $('.bf-table-this-page', form).hide();
  $('.bf-table-all-pages', form).show();
  // Modify the value of the hidden form flag field.
  $('.bf-select-all-pages-flag', form).val('1');
}
/**
 * Prepares the select all on this page functionality.
 */
Backdrop.bf.tableSelectThisPage = function(form) {
  $('.bf-table-all-pages', form).hide();
  $('.bf-table-this-page', form).show();
  // Modify the value of the hidden form field.
  $('.bf-select-all-pages-flag', form).val('0');
}

Backdrop.bf.initGenericBehaviors = function(form) {
  $('input:checkbox[name^="bulk_form"]', form).on('click', function() {
    if (!this.checked) {
      // Modify the value of the hidden form field.
      $('.bf-select-all-pages-flag', form).val('0')

      var table = $(this).closest('table')[0];
      if (table) {
        // If there's a "select all" row, hide it.
        if ($('.bf-table-select-this-page', table).length) {
          $('.bf-views-table-row-select-all', table).hide();
          // Disable "select all across pages".
          Backdrop.bf.tableSelectThisPage(form);
        }
      }
    }

  });
}

})(jQuery);
