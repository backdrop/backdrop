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

    var tagRows = [];
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

    // Filter the list of modules by provided search string.
    function filterModuleListBySearch() {
      var query = $input.val().toLowerCase();

      // Filter only if the length of the query is at least 2 characters.
      if (query.length >= 2) {
        $(tagRows).each(function( index ) {
          showModuleRow(query, this, '.table-filter-text-source');
        });

        // Toggle no results string
        if ($rows.filter(':visible').length === 0) {
          if ($('.filter-empty').length === 0) {
            $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
          }
        }
        else {
          $('.filter-empty').remove();
        }
      }
      else {
        $.each(tagRows, function() {
          $(this).show();
        });
      }
    }

    // Filter the list of modules by tags.
    // Creates an array ('tagRows')of modules matching the selected tags, as a 
    // subset of the $rows object, which is used by filterModuleListBySearch() 
    // instead of $rows to do further filtering. 
    function filterModuleListByTag() {
      var $selecTag = $('#edit-tags').val().toLowerCase();
      // If 'All tags' selected, show all tags and add all to tagRows.
      if ($selecTag == 'all') {
        $rows.each(function( index ) {
          $(this).closest('tr').show();
          tagRows.push(this);
        });
      }
      else {
        tagRows = [];
        $rows.each(function( index ) {
          // Filter by tag and add matches to tagRows.
          row = showModuleRow($selecTag, this, '.module-tags');
          tagRows.push(row);
        });
      }
      
      // Toggle no results string
      if ($rows.filter(':visible').length === 0) {
        if ($('.filter-empty').length === 0) {
          $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
        }
      }
      else {
        $('.filter-empty').remove();
      }
    }
    
    // Filters each module row based either on tag or search string and returns
    // the row if it matches.
    function showModuleRow(selecTag, row, searchTarget) {
      var $row = $(row);
      var $sources = $row.find(searchTarget);
      var coreHidden = $('#edit-core-switch').prop("checked");
      var textMatch = $sources.text().toLowerCase().indexOf(selecTag) !== -1;
      var coreMatch = $sources.text().toLowerCase().indexOf("core") !== -1;
      if (coreHidden && coreMatch) {
        $row.closest('tr').hide();
      }
      else {
        $row.closest('tr').toggle(textMatch);
      }
      if(textMatch) {
        return row;
      }
    }
    
    // Toggles rows when the 'Hide core' checkbox is toggled.
    function filterCore() {
      filterModuleListByTag();
      if ($input.val().toLowerCase().length >= 2) {
        filterModuleListBySearch();
      }
    }

    if ($form.length) {
      $rowsAndFieldsets = $form.find('tr, fieldset');
      $rows = $form.find('tbody tr');
      $fieldset = $form.find('#edit-modules');
      // Filter all rows by tag.
      filterModuleListByTag();

      $input.focus().on('keyup', filterModuleListBySearch);
      $('#edit-tags').on('change', filterModuleListByTag);
      $('#edit-core-switch').on('change', filterCore);
    }
  }
};

})(jQuery);
