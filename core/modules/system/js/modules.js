(function ($) {

/**
 * Filters the module list table by a text input search string.
 *
 * Additionally accounts for multiple tables being wrapped in "package" fieldset
 * elements.
 */
Backdrop.behaviors.moduleFilterByText = {
  attach: function(context, settings) {
    var $input = $('input.table-filter-text').once('table-filter-text');
    var $form = $('#system-modules');
    var $rowsAndFieldsets, $rows, $fieldset;

    // Hide the module requirements.
    $form.find('.requirements').hide();

    // Toggle the requirements info.
    $('a.requirements-toggle').click(function(e) {
      var $requirements = $(this).closest('td').find('.requirements').toggle();
      if ($requirements.is(':visible')) {
        $(this).text(Backdrop.t('less'));
      }
      else {
        $(this).text(Backdrop.t('more'));
      }
      e.preventDefault();
      e.stopPropagation();
    });

    // Fliter the list of modules by provided search string.
    function filterModuleList() {
      $('#edit-tags').val('All');
      var query = $input.val().toLowerCase();

      function showModuleRow(index, row) {
        var searchTarget = '.table-filter-text-source';
        var $row = $(row);
        var $sources = $row.find(searchTarget);
        var textMatch = $sources.text().toLowerCase().indexOf(query) !== -1;
        $row.closest('tr').toggle(textMatch);
      }

      // Filter only if the length of the query is at least 2 characters.
      if (query.length >= 2) {
        $rows.each(showModuleRow);

        if ($fieldset.filter(':visible').length === 0) {
          if ($('.filter-empty').length === 0) {
            $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
          }
        }
      }
    }

    // Fliter the list of modules by provided search string.
    function filterModuleListByTag() {
      $input.val('');

      function showModuleRowByTag(index, row) {
        var searchTarget = '.module-tags';
        var $row = $(row);
        var $sources = $row.find(searchTarget);
        var textMatch = $sources.text().toLowerCase().indexOf($selecTag) !== -1;
        $row.closest('tr').toggle(textMatch);
      }

      var $selecTag = $('#edit-tags').val().toLowerCase();
      if ($selecTag == 'all') {
        $rows.each(function( index ) {
          $(this).closest('tr').show();
        console.log($selecTag);
        });
      }
      else {
        $rows.each(showModuleRowByTag);
      }

        if ($fieldset.filter(':visible').length === 0) {
          if ($('.filter-empty').length === 0) {
            $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
          }
        }
    }

    if ($form.length) {
      $rowsAndFieldsets = $form.find('tr, fieldset');
      $rows = $form.find('tbody tr');
      $fieldset = $form.find('#edit-modules');

      // @todo Use autofocus attribute when possible.
      $input.focus().on('keyup', filterModuleList);
      $('#edit-tags').on('change', filterModuleListByTag);
    }
  }
};

})(jQuery);
