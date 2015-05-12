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
    var $rowsAndFieldsets, $rows, $fieldsets;

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

      // Todo add magic search:
        // 'ddd' for disabled and 'eee' for enabled
        // '-' + '<search-term>' to negate search

      function showModuleRow(index, row) {
        if ($('#edit-target').is(':checked')) {
          var searchTarget = '.module-tags';
        }
        else {
          var searchTarget = '.table-filter-text-source';
        }

        var $row = $(row);
        var $sources = $row.find('.table-filter-text-source');
        var rowMatch = $sources.text().toLowerCase().indexOf(query) !== -1;
        var $fieldsetTitle = $row.closest('fieldset').find('legend:first');

        // Finding the fieldset title and filtering it can be expensive and
        // repetitive to do for every row, so save the filtered title as data on
        // the DOM element.
        var filterTitle;
        if (!$fieldsetTitle.data('filterTitle')) {
          // Don't include hidden DOM elements such as the show/hide label.
          filterTitle = $fieldsetTitle.clone().find('.element-invisible').remove().end().text().toLowerCase();
          $fieldsetTitle.data('filterTitle', filterTitle);
        }
        else {
          filterTitle = $fieldsetTitle.data('filterTitle');
        }
        var fieldsetTitleMatch = filterTitle.indexOf(query) !== -1;

        // If the row contains the string or the fieldset does, show the row.
        $row.closest('tr').toggle(rowMatch || fieldsetTitleMatch);
      }

      // Filter only if the length of the query is at least 2 characters.
      if (query.length >= 2) {
        $(tagRows).each(function( index ) {
          showModuleRow(query, this, '.table-filter-text-source');
        });

        $('#edit-target').on('change', function() {
          $rows.each(showModuleRow);
        });

        // We first show() all <fieldset>s to be able to use ':visible'.
        $fieldsets.show().each(hidePackageFieldset);

        if ($fieldsets.filter(':visible').length === 0) {
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
      var coreHidden = $('#edit-core-switch-core').prop("checked") === false;
      var contribHidden = $('#edit-core-switch-contrib').prop("checked") === false;
      console.log($sources);
      if (selecTag != 'all') {
        var textMatch = $sources.text().toLowerCase().indexOf(selecTag) !== -1;
      }
      else {
        var textMatch = true;
      }
      var coreMatch = $sources.text().toLowerCase().indexOf("core") !== -1;
      if (coreHidden && coreMatch) {
        $row.closest('tr').hide();
      }
      else if(contribHidden && !coreMatch) {
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
      $fieldsets = $form.find('fieldset');

      // @todo Use autofocus attribute when possible.
      $input.focus().on('keyup', filterModuleList);
      $input.triggerHandler('keyup');
      $('#edit-tags').on('change', filterModuleListByTag);
      $('#edit-core-switch-core').on('change', filterCore);
      $('#edit-core-switch-contrib').on('change', filterCore);
    }

    corefltr = $form.find('#edit-core-switch-core');
    contribfltr = $form.find('#edit-core-switch-contrib');
    corefltr.change(function() {
      if (corefltr.is(':not(:checked)')) {
          contribfltr.prop('disabled', true);
      }
      else {
        contribfltr.prop('disabled', false);
      }
    });
    contribfltr.change(function() {
      if (contribfltr.is(':not(:checked)')) {
          corefltr.prop('disabled', true);
      }
      else {
        corefltr.prop('disabled', false);
      }
    });
  }
};

})(jQuery);
